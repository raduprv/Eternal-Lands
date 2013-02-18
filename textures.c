#include <stdlib.h>
#include <string.h>
#ifndef	NEW_TEXTURES
#include <zlib.h>
#include <SDL_image.h>
#endif	/* NEW_TEXTURES */
#include "textures.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "gl_init.h"
#include "init.h"
#include "load_gl_extensions.h"
#include "map.h"
#ifdef	NEW_TEXTURES
#include "image.h"
#include "image_loading.h"
#include "queue.h"
#include "threads.h"
#include "memory.h"
#include <assert.h>
#else	/* NEW_TEXTURES */
#include "io/elfilewrapper.h"
#include "ddsimage.h"
#endif	/* NEW_TEXTURES */
#include "hash.h"

#define TEXTURE_SIZE_X 512
#define TEXTURE_SIZE_Y 512
#define TEXTURE_RATIO 2

#ifdef	NEW_TEXTURES

#ifdef	ELC
#define ACTOR_TEXTURE_CACHE_MAX 256
#define ACTOR_TEXTURE_THREAD_COUNT 2

actor_texture_cache_t* actor_texture_handles = NULL;
SDL_Thread* actor_texture_threads[ACTOR_TEXTURE_THREAD_COUNT];
Uint32 max_actor_texture_handles = 32;
queue_t* actor_texture_queue = NULL;
Uint32 actor_texture_threads_done = 0;
#endif	/* ELC */

#define TEXTURE_CACHE_MAX 8192

static texture_cache_t* texture_handles = NULL;
static cache_struct* texture_cache = NULL;
static Uint32 texture_handles_used = 0;
#ifdef FASTER_MAP_LOAD
static Uint32 texture_cache_sorted[TEXTURE_CACHE_MAX];
#endif

Uint32 compact_texture(texture_cache_t* texture)
{
	Uint32 size;

	if (texture == 0)
	{
		return 0;
	}

	if (texture->id == 0)
	{
		return 0;
	}

	glDeleteTextures(1, &texture->id);

	size = texture->size;

	texture->id = 0;
	texture->size = 0;

	return size;
}

void bind_texture_id(const GLuint id)
{
	if (last_texture != id)
	{
		last_texture = id;
		glBindTexture(GL_TEXTURE_2D, id);
	}
}

static GLuint build_texture(image_t* image, const Uint32 wrap_mode_repeat,
	const GLenum min_filter, const Uint32 af,
	const texture_format_type format)
{
	void* ptr;
	GLuint id;
	GLenum src_format, type, internal_format;
	Uint32 compressed, compression, width, height, i;

	compressed = 0;
	compression = 0;

	switch (image->format)
	{
		case ift_rgba4:
			src_format = GL_RGBA;
			type = GL_UNSIGNED_SHORT_4_4_4_4;
			internal_format = GL_RGBA4;
			break;
		case ift_rgb8:
			src_format = GL_RGB;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_RGB8;
			break;
		case ift_r5g6b5:
			src_format = GL_RGB;
			type = GL_UNSIGNED_SHORT_5_6_5;
			internal_format = GL_RGB5;
			break;
		case ift_rgba8:
			src_format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_RGBA8;
			break;
		case ift_bgra8:
			src_format = GL_BGRA;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_RGBA8;
			break;
		case ift_bgr8:
			src_format = GL_BGR;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_RGB8;
			break;
		case ift_rgb5_a1:
			src_format = GL_RGBA;
			type = GL_UNSIGNED_SHORT_5_5_5_1;
			internal_format = GL_RGB5_A1;
			break;
		case ift_a8:
			src_format = GL_ALPHA;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_ALPHA8;
			break;
		case ift_l8:
			src_format = GL_LUMINANCE;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_LUMINANCE8;
			break;
		case ift_la8:
			src_format = GL_LUMINANCE_ALPHA;
			type = GL_UNSIGNED_BYTE;
			internal_format = GL_LUMINANCE8_ALPHA8;
			break;
		case ift_dxt1:
			internal_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			compressed = tct_s3tc;
			break;
		case ift_dxt3:
			internal_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			compressed = tct_s3tc;
			break;
		case ift_dxt5:
			internal_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			compressed = tct_s3tc;
			break;
		case ift_ati1:
			internal_format = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
			compressed = tct_latc;
			break;
		case ift_ati2:
			if ((!have_extension(ext_texture_compression_latc))
				&& have_extension(ati_texture_compression_3dc))
			{
				internal_format = GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI;
				compressed = tct_3dc;
			}
			else
			{
				internal_format = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
				compressed = tct_latc;
			}
			break;
		default:
			LOG_ERROR("Unsupported image format (%i)", image->format);
			return 0;
	}

	if ((compressed != 0) && (!have_extension(arb_texture_compression)))
	{
		LOG_ERROR("Can't use compressed source formats because "
			"GL_ARB_texture_compression is not supported.");
		return 0;
	}

	if ((!have_extension(ext_texture_compression_s3tc)) &&
		((compressed & tct_s3tc) == tct_s3tc))
	{
		LOG_ERROR("Can't use s3tc compressed source formats because "
			"GL_EXT_texture_compression_s3tc is not supported.");
		return 0;
	}

	if ((!have_extension(ext_texture_compression_latc)) &&
		((compressed & tct_latc) == tct_latc))
	{
		LOG_ERROR("Can't use s3tc compressed source formats because "
			"GL_EXT_texture_compression_latc is not supported.");
		return 0;
	}

	if ((!have_extension(ati_texture_compression_3dc)) &&
		((compressed & tct_3dc) == tct_3dc))
	{
		LOG_ERROR("Can't use s3tc compressed source formats because "
			"GL_ATI_texture_compression_3dc is not supported.");
		return 0;
	}

	if (compressed == 0)
	{
		switch (format)
		{
			case tft_auto:
				break;
			case tft_rgba4:
				internal_format = GL_RGBA4;
				break;
			case tft_rgb8:
				internal_format = GL_RGB8;
				break;
			case tft_r5g6b5:
				internal_format = GL_RGB5;
				break;
			case tft_rgba8:
				internal_format = GL_RGBA8;
				break;
			case tft_rgb5_a1:
				internal_format = GL_RGB5_A1;
				break;
			case tft_a8:
				internal_format = GL_ALPHA8;
				break;
			case tft_l8:
				internal_format = GL_LUMINANCE8;
				break;
			case tft_la8:
				internal_format = GL_LUMINANCE8_ALPHA8;
				break;
			case tft_dxt1:
				internal_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				compression = tct_s3tc;
				break;
			case tft_dxt3:
				internal_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
				compression = tct_s3tc;
				break;
			case tft_dxt5:
				internal_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				compression = tct_s3tc;
				break;
			case tft_ati1:
				internal_format = GL_COMPRESSED_LUMINANCE_LATC1_EXT;
				compression = tct_latc;
				break;
			case tft_ati2:
				if ((!have_extension(ext_texture_compression_latc))
					&& have_extension(ati_texture_compression_3dc))
				{
					internal_format = GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI;
					compression = tct_3dc;
				}
				else
				{
					internal_format = GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
					compression = tct_latc;
				}
				break;
			default:
				LOG_ERROR("Unsupported texture format (%i)",
					format);
				return 0;
		}
	}

	if ((compression != 0) && (!have_extension(arb_texture_compression)))
	{
		LOG_ERROR("Can't use compressed texture format, because "
			"GL_ARB_texture_compression is not supported.");
		return 0;
	}

	if ((!have_extension(ext_texture_compression_s3tc)) &&
		((compression & tct_s3tc) == tct_s3tc))
	{
		LOG_ERROR("Can't use s3tc compressed texture format, because "
			"GL_EXT_texture_compression_s3tc is not supported.");
		return 0;
	}

	if ((!have_extension(ext_texture_compression_latc)) &&
		((compression & tct_latc) == tct_latc))
	{
		LOG_ERROR("Can't use s3tc compressed texture format, because "
			"GL_EXT_texture_compression_latc is not supported.");
		return 0;
	}

	if ((!have_extension(ati_texture_compression_3dc)) &&
		((compression & tct_3dc) == tct_3dc))
	{
		LOG_ERROR("Can't use s3tc compressed texture format, because "
			"GL_ATI_texture_compression_3dc is not supported.");
		return 0;
	}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);	//failsafe
	bind_texture_id(id);

	CHECK_GL_ERRORS();

	if (wrap_mode_repeat != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
		image->mipmaps - 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

	if (af != 0)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
			anisotropic_filter);
	}

	width = image->width;
	height = image->height;

	CHECK_GL_ERRORS();

	for (i = 0; i < image->mipmaps; i++)
	{
		assert(image->sizes[i] > 0);

		ptr = image->image + image->offsets[i];

		if (compressed != 0)
		{
			ELglCompressedTexImage2D(GL_TEXTURE_2D, i,
				internal_format, width, height, 0,
				image->sizes[i], ptr);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, i, internal_format,
				width, height, 0, src_format, type, ptr);
		}

		CHECK_GL_ERRORS();

		if (width > 1)
		{
			width /= 2;
		}

		if (height > 1)
		{
			height /= 2;
		}
	}

	return id;
}

#ifdef FASTER_MAP_LOAD
typedef struct
{
	Uint32 hash;
	char file_name[128];
} cache_identifier_t;

int cache_cmp_identifier(const void *idfp, const void *idxp)
{
	const cache_identifier_t *idf = idfp;
	Uint32 idx = *((const Uint32*)idxp);
	if (idf->hash < texture_handles[idx].hash)
		return -1;
	if (idf->hash > texture_handles[idx].hash)
		return 1;
	return strcasecmp(idf->file_name, texture_handles[idx].file_name);
}

Uint32 load_texture_cached(const char* file_name, const texture_type type)
{
	cache_identifier_t idf;
	Uint32 i, len;
	Uint32 *idxp, idx;

	len = get_file_name_len(file_name);
	idf.hash = mem_hash(file_name, len);
	safe_strncpy2(idf.file_name, file_name, sizeof(idf.file_name), len);

	idxp = bsearch(&idf, texture_cache_sorted, texture_handles_used,
		sizeof(Uint32), cache_cmp_identifier);
	if (idxp)
		return *idxp;

	if (texture_handles_used < TEXTURE_CACHE_MAX)
	{
		Uint32 slot = texture_handles_used;
		for (i = 0; i < texture_handles_used; i++)
		{
			idx = texture_cache_sorted[i];
			if (idf.hash < texture_handles[idx].hash
				|| (idf.hash == texture_handles[idx].hash
					&& strcasecmp(idf.file_name, texture_handles[idx].file_name) <= 0))
			{
				memmove(texture_cache_sorted+(i+1), texture_cache_sorted+i,
					(texture_handles_used-i)*sizeof(Uint32));
				break;
			}
		}

		texture_cache_sorted[i] = slot;

		safe_strncpy(texture_handles[slot].file_name, idf.file_name,
			sizeof(texture_handles[slot].file_name));
		texture_handles[slot].hash = idf.hash;
		texture_handles[slot].type = type;
		texture_handles[slot].id = 0;
		texture_handles[slot].cache_ptr = cache_add_item(texture_cache,
			texture_handles[slot].file_name,
			&texture_handles[slot], 0);

		texture_handles_used++;

		return slot;
	}
	else
	{
		LOG_ERROR("Error: out of texture space\n");
		return TEXTURE_CACHE_MAX;	// ERROR!
	}
}
#else  // FASTER_MAP_LOAD
Uint32 load_texture_cached(const char* file_name, const texture_type type)
{
	char buffer[128];
	Uint32 i, handle, len, hash;

	handle = texture_handles_used;

	len = get_file_name_len(file_name);
	hash = mem_hash(file_name, len);

	safe_strncpy2(buffer, file_name, sizeof(buffer), len);

	for (i = 0; i < texture_handles_used; i++)
	{
		if (texture_handles[i].file_name[0] != 0)
		{
			if (hash == texture_handles[i].hash)
			{
				if (!strcasecmp(texture_handles[i].file_name,
					buffer))
				{
					// already loaded, use existing texture
					return i;
				}
			}
		}
		else
		{
			// remember the first open slot we have
			if (handle == texture_handles_used)
			{
				handle = i;
			}
		}
	}

	if (handle < TEXTURE_CACHE_MAX)
	{
		//we found a place to store it
		safe_strncpy2(texture_handles[handle].file_name, file_name,
			sizeof(texture_handles[handle].file_name), len);

		texture_handles[handle].hash = hash;
		texture_handles[handle].type = type;
		texture_handles[handle].id = 0;
		texture_handles[handle].cache_ptr = cache_add_item(texture_cache,
			texture_handles[handle].file_name,
			&texture_handles[handle], 0);

		texture_handles_used++;

		return handle;
	}
	else
	{
		LOG_ERROR("Error: out of texture space\n");

		return TEXTURE_CACHE_MAX;	// ERROR!
	}
}
#endif // FASTER_MAP_LOAD

static Uint32 get_supported_compression_formats()
{
	Uint32 result;

	result = 0;

	if (!have_extension(arb_texture_compression))
	{
		return result;
	}

	if (have_extension(ext_texture_compression_s3tc))
	{
		result |= tct_s3tc;
	}

	if (have_extension(ati_texture_compression_3dc))
	{
		result |= tct_3dc;
	}

	if (have_extension(ext_texture_compression_latc))
	{
		result |= tct_latc;
	}

	return result;
}

static Uint32 load_texture(texture_cache_t* texture_handle)
{
	image_t image;
	GLuint id;
	Uint32 strip_mipmaps, base_level, wrap_mode_repeat, af, i, compression;
	GLenum min_filter;
	texture_format_type format;

	memset(&image, 0, sizeof(image_t));

	wrap_mode_repeat = 0;
	strip_mipmaps = 0;
	base_level = 0;
	af = 0;
	min_filter = GL_LINEAR;
	format = tft_auto;

	compression = get_supported_compression_formats();

	switch (texture_handle->type)
	{
		case tt_gui:
			wrap_mode_repeat = 1;
			strip_mipmaps = 1;
			break;
		case tt_image:
			strip_mipmaps = 1;
			if ((compression & tct_s3tc) == tct_s3tc)
			{
				format = tft_dxt1;
			}

			break;
		case tt_font:
			break;
		case tt_mesh:
			wrap_mode_repeat = 1;
			if (poor_man != 0)
			{
				min_filter = GL_LINEAR_MIPMAP_NEAREST;
				base_level = 1;
			}
			else
			{
				min_filter = GL_LINEAR_MIPMAP_LINEAR;
				af = 1;
			}
			break;
		case tt_atlas:
			wrap_mode_repeat = 0;
			break;
	}

	if (load_image_data(texture_handle->file_name, compression, 0,
		strip_mipmaps, base_level, &image) == 0)
	{
		texture_handle->load_err = 1;

		LOG_ERROR("Error loading image '%s'",
			texture_handle->file_name);

		return 0;
	}

	id = build_texture(&image, wrap_mode_repeat, min_filter, af, format);

	assert(id != 0);

	texture_handle->id = id;
	texture_handle->alpha = image.alpha;
	texture_handle->size = 0;

	for (i = 0; i < image.mipmaps; i++)
	{
		texture_handle->size += image.sizes[i];
	}

	free_image(&image);

	return 1;
}

static Uint32 load_texture_handle(const Uint32 handle)
{
	if (handle >= texture_handles_used)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			texture_handles_used);

		return 0;
	}

	if (texture_handles[handle].id != 0)
	{
		return 1;
	}

	if (texture_handles[handle].load_err != 0)
	{
		return 0;
	}

	if (load_texture(&texture_handles[handle]) != 0)
	{
		cache_adj_size(texture_cache,
			texture_handles[handle].size,
			&texture_handles[handle]);

		return 1;
	}

	return 0;
}

static GLuint get_texture_id(const Uint32 handle)
{
	if (handle >= texture_handles_used)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			texture_handles_used);

		return 0;
	}

	if (load_texture_handle(handle) == 0)
	{
		return 0;
	}

	assert(texture_handles[handle].cache_ptr != 0);
	assert(texture_handles[handle].id != 0);

	cache_use(texture_handles[handle].cache_ptr);

	return texture_handles[handle].id;
}

Uint32 get_texture_alpha(const Uint32 handle)
{
	if (handle >= texture_handles_used)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			texture_handles_used);

		return 0;
	}

	if (load_texture_handle(handle) == 0)
	{
		return 0;
	}

	return texture_handles[handle].alpha;
}

void bind_texture(const Uint32 handle)
{
	if (handle >= texture_handles_used)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			texture_handles_used);

		return;
	}

	bind_texture_id(get_texture_id(handle));
}

void bind_texture_unbuffered(const Uint32 handle)
{
	if (handle >= texture_handles_used)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			texture_handles_used);

		return;
	}

	glBindTexture(GL_TEXTURE_2D, get_texture_id(handle));
}

#ifdef	ELC
void reload_actor_texture_resources(actor_texture_cache_t* texture)
{
	if (texture != 0)
	{
		if (texture->new_id != 0)
		{
			glDeleteTextures(1, &texture->new_id);
			texture->new_id = 0;
		}

		if (texture->image.image != 0)
		{
			free_image(&texture->image);
		}

		texture->state = tst_unloaded;
	}
}

void free_actor_texture_resources(actor_texture_cache_t* texture)
{
	if (texture != 0)
	{
		if (texture->id != 0)
		{
			glDeleteTextures(1, &texture->id);
			texture->id = 0;
		}

		if (texture->new_id != 0)
		{
			glDeleteTextures(1, &texture->new_id);
			texture->new_id = 0;
		}

		if (texture->image.image != 0)
		{
			free_image(&texture->image);
		}

		texture->state = tst_unloaded;
	}
}

static Uint32 copy_to_coordinates(const image_t* source, const Uint32 x,
	const Uint32 y, image_t* dest)
{
	Uint32 source_height, source_width, source_offset;
	Uint32 dest_width, dest_offset;
	Uint32 i;
	Uint8 *src, *dst;

	source_width = source->width;
	source_height = source->height;
	dest_width = dest->width;
	src = source->image;
	dst = dest->image;

	for (i = 0; i < source_height; i++)
	{
		source_offset = i * source_width * 4;
		dest_offset = ((y + i) * dest_width + x) * 4;

		memcpy(dst + dest_offset, src + source_offset, source_width * 4);
	}

	return source->alpha;
}

static Uint32 copy_to_coordinates_block(const image_t* source, const Uint32 x,
	const Uint32 y, image_t* dest)
{
	Uint32 source_height, source_width, source_offset;
	Uint32 dest_height, dest_width, dest_offset, dest_start;
	Uint32 i, j, dest_size, source_size;
	Uint8 *src, *dst;

	source_width = source->width;
	source_height = source->height;
	dest_width = dest->width;
	dest_height = dest->height;
	src = source->image;
	dst = dest->image;

	switch (source->format)
	{
		case ift_dxt1:
			source_size = 8;
			break;
		case ift_dxt3:
		case ift_dxt5:
			source_size = 16;
			break;
		default:
			LOG_ERROR("Can use block copy only for DXT1, DXT3 or"
				" DXT5 sources.");
			return 0;
	}

	dest_start = 0;

	switch (dest->format)
	{
		case ift_dxt1:
			dest_size = 8;
			break;
		case ift_dxt3:
		case ift_dxt5:
			dest_size = 16;
			if (source->format == ift_dxt1)
			{
				dest_start = 8;
			}
			break;
		default:
			LOG_ERROR("Can use block copy only for DXT1, DXT3 or"
				" DXT5 sources.");
			return 0;
	}

	if (dest_size < source_size)
	{
		LOG_ERROR("Dest format must be bigger than source format.");
		return 0;
	}

	source_offset = 0;

	if ((x + source_width) > dest_width)
	{
		LOG_ERROR("Offset x %d + source widht %d must be smaller than"
			" dest widht %d", x, source_width, dest_width);
		return 0;
	}

	if ((y + source_height) > dest_height)
	{
		LOG_ERROR("Offset y %d + source height %d must be smaller than"
			" dest height %d", y, source_height, dest_height);
		return 0;
	}

	for (j = 0; j < source_height; j += 4)
	{
		dest_offset = (y + j) * dest_width / 16;
		dest_offset += x / 4;
		dest_offset *= dest_size;
		dest_offset += dest_start;

		for (i = 0; i < source_width; i += 4)
		{
			memcpy(dst + dest_offset, src + source_offset,
				source_size);

			dest_offset += dest_size;
			source_offset += source_size;
		}
	}

	return source->alpha;
}

static Uint32 copy_to_coordinates_mask2(const image_t* source0,
	const image_t* source1, const image_t* mask,
	const Uint32 x, const Uint32 y, image_t* dest, Uint8* buffer)
{
	Uint32 source_height, source_width, source_offset;
	Uint32 dest_width, dest_offset;
	Uint32 i, size;
	Uint8 *src0, *src1, *msk, *dst;

	source_width = source0->width;
	source_height = source0->height;
	dest_width = dest->width;
	src0 = source0->image;
	src1 = source1->image;
	msk = mask->image;
	dst = dest->image;

	size = source_width * source_height;

	fast_blend(msk, size, src1, src0, buffer);

	for (i = 0; i < source_height; i++)
	{
		source_offset = i * source_width * 4;
		dest_offset = ((y + i) * dest_width + x) * 4;

		memcpy(dst + dest_offset, buffer + source_offset,
			source_width * 4);
	}

	if ((source0->alpha != 0) || (source1->alpha != 0))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

Uint32 load_image_data_file_size(el_file_ptr file, const Uint32 compression,
	const Uint32 unpack, const Uint32 width, const Uint32 height,
	image_t* image)
{
	Uint32 sw, sh, s, size, base_level;

	if (get_image_information(file, image) == 0)
	{
		LOG_ERROR("Can't get image information for '%s'!",
			el_file_name(file));
		return 0;
	}

	s = 1;
	base_level = 0;
	sw = image->width / width;
	sh = image->height / height;

	size = min2u(sw, sh);

	if ((sw != sh) || (popcount(size) != 1))
	{
		LOG_ERROR("Invalid mipmap (%d) size <%d, %d> for file '%s' "
			"<%d, %d>.", size, width, height, el_file_name(file),
			image->width, image->height);
		return 0;
	}

	while (size > s)
	{
		base_level++;
		s += s;
	}

	assert(s == size);

	if (base_level >= image->mipmaps)
	{
		LOG_ERROR("Image file '%s' has %d mipmaps, but base level is"
			" %d", el_file_name(file), image->mipmaps, base_level);
		return 0;
	}

	LOG_DEBUG_VERBOSE("Using base level %d for image '%s'.", base_level,
		el_file_name(file));

	if (load_image_data_file(file, compression, unpack, 1, base_level,
		image) == 0)
	{
		LOG_ERROR("Can't load file '%s'.", el_file_name(file));
		return 0;
	}

	return 1;
}

static Uint32 load_to_coordinates(el_file_ptr file, const Uint32 x,
	const Uint32 y, const Uint32 width, const Uint32 height,
	const Uint32 use_compressed_image, const Uint32 scale, image_t *dst)
{
	image_t image;
	Uint32 tw, th, tx, ty, result;

	memset(&image, 0, sizeof(image));

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");
		return 0;
	}

	tw = width * scale;
	th = height * scale;
	tx = x * scale;
	ty = y * scale;

	if (use_compressed_image == 1)
	{
		if (load_image_data_file_size(file, tct_s3tc, 0, tw, th,
			&image) == 0)
		{
			LOG_ERROR("Can't load file '%s'.", el_file_name(file));
			return 0;
		}

		if ((image.width != tw) || (image.height != th))
		{
			LOG_ERROR("File '%s' has wrong size <%d, %d> instead "
				"of <%d, %d>.", el_file_name(file), 
				image.width, image.height, tw, th);
			free_image(&image);
			return 0;
		}

		if (((tw % 4) != 0) || ((th % 4) != 0) || ((tx % 4) != 0) ||
			((ty % 4) != 0))
		{
			LOG_ERROR("Wrong sizes for compression <%d, %d>, "
				"offset <%d, %d>.", tw, th, tx, ty);
			free_image(&image);
			return 0;
		}

		result = copy_to_coordinates_block(&image, tx, ty, dst);
	}
	else
	{
		if (load_image_data_file_size(file, 0, 1, tw, th, &image) == 0)
		{
			LOG_ERROR("Can't load file '%s'.", el_file_name(file));
			return 0;
		}

		if ((image.width != tw) || (image.height != th))
		{
			LOG_ERROR("File '%s' has wrong size <%d, %d> instead "
				"of <%d, %d>.", el_file_name(file),
				image.width, image.height, tw, th);
			free_image(&image);
			return 0;
		}

		result = copy_to_coordinates(&image, tx, ty, dst);
	}

	free_image(&image);

	return result;
}

static void build_alpha_mask(const Uint8* source, const Uint32 size,
	Uint8* dest)
{
	Uint32 i;

	for (i = 0; i < size; i++)
	{
		dest[i] = source[i * 4 + 3];
	}
}

static Uint32 load_to_coordinates_mask2(el_file_ptr source0, el_file_ptr source1,
	el_file_ptr mask, const Uint32 x, const Uint32 y, const Uint32 width,
	const Uint32 height, const Uint32 use_compressed_image,
	const Uint32 scale, image_t *dest, Uint8* buffer)
{
	image_t src0, src1, msk;
	Uint8* tmp;
	Uint32 tw, th, tx, ty, result;

	memset(&src0, 0, sizeof(src0));
	memset(&src1, 0, sizeof(src1));
	memset(&msk, 0, sizeof(msk));

	tw = width * scale;
	th = height * scale;
	tx = x * scale;
	ty = y * scale;

	if ((source1 == 0) || (mask == 0))
	{
		return load_to_coordinates(source0, x, y, width, height,
			use_compressed_image, scale, dest);
	}

	if (load_image_data_file_size(source0, 0, 1, tw, th, &src0) == 0)
	{
		LOG_ERROR("Can't load file '%s'.", el_file_name(source0));
		free_image(&src0);
		el_close(source1);
		el_close(mask);
		return 0;
	}

	if ((src0.width != tw) || (src0.height != th))
	{
		LOG_ERROR("File '%s' has wrong size <%d, %d> instead of "
			"<%d, %d>.", src0.width, src0.height, tw, th);
		return 0;
	}

	if (load_image_data_file_size(source1, 0, 1, tw, th, &src1) == 0)
	{
		LOG_ERROR("Can't load file '%s'.", el_file_name(source1));
		free_image(&src0);
		free_image(&src1);
		el_close(mask);
		return 0;
	}

	if ((src1.width != tw) || (src1.height != th))
	{
		LOG_ERROR("File '%s' has wrong size <%d, %d> instead of "
			"<%d, %d>.", src1.width, src1.height, tw, th);
		return 0;
	}

	if (load_image_data_file_size(mask, 0, 0, tw, th, &msk) == 0)
	{
		LOG_ERROR("Can't load file '%s'.", el_file_name(mask));
		return 0;
	}

	if ((msk.width != tw) || (msk.height != th))
	{
		LOG_ERROR("File '%s' has wrong size <%d, %d> instead of "
			"<%d, %d>.", msk.width, msk.height, tw, th);

		free_image(&src0);
		free_image(&src1);
		free_image(&msk);

		return 0;
	}

	if (msk.format != ift_a8)
	{
		if (msk.format != ift_rgba8)
		{
			LOG_ERROR("Can't convert image to alpha mask");

			return 0;
		}

		tmp = malloc_aligned(msk.width * msk.height, 16);

		build_alpha_mask(msk.image, msk.width * msk.height, tmp);

		free_image(&msk);
		memset(msk.sizes, 0, sizeof(msk.sizes));
		memset(msk.offsets, 0, sizeof(msk.offsets));

		msk.image = tmp;
		msk.format = ift_a8;
		msk.mipmaps = 1;
		msk.alpha = 1;
		msk.sizes[0] = msk.width * msk.height;
	}

	result = copy_to_coordinates_mask2(&src0, &src1, &msk, tx, ty, dest,
		buffer);

	free_image(&src0);
	free_image(&src1);
	free_image(&msk);

	return result;
}

static Uint32 open_for_coordinates_checks(const char* file_name,
	el_file_ptr* file, const Uint32 width, const Uint32 height,
	Uint16* sizes, image_t* image)
{
	char buffer[128];
	Uint32 i, mask, size, size_x, size_y;

	if (check_image_name(file_name, sizeof(buffer), buffer) == 0)
	{
		LOG_ERROR("File '%s' not found!", file_name);
		return 0;
	}

	*file = el_open_custom(buffer);

	if (*file == 0)
	{
		LOG_ERROR("Can't open file '%s'!", buffer);
		return 0;
	}

	if (get_image_information(*file, image) == 0)
	{
		LOG_ERROR("Can't get image information for '%s'!",
			el_file_name(*file));
		el_close(*file);
		*file = 0;
		return 0;
	}

	size_x = image->width / width;
	size_y = image->height / height;
	size = min2u(size_x, size_y);

	if (((image->width % width) != 0) || ((image->height % height) != 0) ||
		(size_x != size_y) || (popcount(size) != 1))
	{
		LOG_ERROR("File '%s' has wrong size <%d, %d> instead of "
			"being 2 ^ x * <%d, %d>.", el_file_name(*file),
			image->width, image->height, width, height);
		el_close(*file);
		*file = 0;
		return 0;
	}

	mask = 0;

	for (i = 0; i < image->mipmaps; i++)
	{
		size /= 2;
		mask |= 1 << size;
	}

	*sizes &= mask;

	if (image->mipmaps < 2)
	{
		LOG_ERROR("File '%s' has no mipmaps", el_file_name(*file));
	}

	return 1;
}

static Uint32 open_for_coordinates(const char* file_name, el_file_ptr* file,
	const Uint32 width, const Uint32 height, Uint16* sizes,
	image_format_type* format)
{
	image_t image;

	if (open_for_coordinates_checks(file_name, file, width, height, sizes,
		&image) == 0)
	{
		return 0;
	}

	if (*format == image.format)
	{
		return 1;
	}

	switch (*format)
	{
		case ift_dxt1:
			*format = image.format;
			return 1;
		case ift_dxt3:
		case ift_dxt5:
			if (image.format == ift_dxt1)
			{
				return 1;
			}
			*format = ift_rgba8;
			return 1;
		case ift_rgba8:
			return 1;
		default:
			*format = ift_rgba8;
			return 0;
	}
}

static Uint32 open_for_coordinates_mask2(const char* source0,
	const char* source1, const char* mask, el_file_ptr* src0,
	el_file_ptr* src1, el_file_ptr* msk, const Uint32 width,
	const Uint32 height, Uint16* sizes, image_format_type* format)
{
	image_t image;

	if ((source1 == 0) || (mask == 0))
	{
		return open_for_coordinates(source0, src0, width, height,
			sizes, format);
	}

	if ((source1[0] == 0) || (mask[0] == 0))
	{
		return open_for_coordinates(source0, src0, width, height,
			sizes, format);
	}

	if (open_for_coordinates_checks(source0, src0, width, height, sizes,
		&image) == 0)
	{
		LOG_ERROR("Can't open file '%s'.", source0);
		return 0;
	}

	if (open_for_coordinates_checks(source1, src1, width, height, sizes,
		&image) == 0)
	{
		LOG_ERROR("Can't open file '%s'.", source1);
		return 0;
	}

	if (open_for_coordinates_checks(mask, msk, width, height, sizes,
		&image) == 0)
	{
		LOG_ERROR("Can't open file '%s'.", mask);

		el_close(*src1);
		*src1 = 0;

		return 0;
	}

	if ((image.format != ift_a8) && (image.alpha == 0))
	{
		LOG_ERROR("Mask image '%s' has no alpha channel!",
			el_file_name(*msk));

		el_close(*src1);
		*src1 = 0;

		el_close(*msk);
		*msk = 0;

		return 0;
	}

	*format = ift_rgba8;

	return 1;
}

static void load_enhanced_actor_threaded(const enhanced_actor_images_t* files,
	image_t* image, Uint8* buffer)
{
	el_file_ptr pants_tex;
	el_file_ptr pants_mask;
	el_file_ptr boots_tex;
	el_file_ptr boots_mask;
	el_file_ptr torso_tex;
	el_file_ptr arms_tex;
	el_file_ptr torso_mask;
	el_file_ptr arms_mask;
	el_file_ptr hands_tex;
	el_file_ptr head_tex;
	el_file_ptr hands_mask;
	el_file_ptr head_mask;
	el_file_ptr head_base;
	el_file_ptr body_base;
	el_file_ptr arms_base;
	el_file_ptr legs_base;
	el_file_ptr boots_base;
	el_file_ptr hair_tex;
	el_file_ptr weapon_tex;
	el_file_ptr shield_tex;
	el_file_ptr helmet_tex;
	el_file_ptr neck_tex;
	el_file_ptr cape_tex;
	el_file_ptr hands_tex_save;
	image_format_type format;
	Uint32 alpha, use_compressed_image, size;
	Uint32 width, height, scale;
	Sint32 i;
	Uint16 sizes;

	pants_tex = 0;
	pants_mask = 0;
	boots_tex = 0;
	boots_mask = 0;
	torso_tex = 0;
	arms_tex = 0;
	torso_mask = 0;
	arms_mask = 0;
	hands_tex = 0;
	head_tex = 0;
	hands_mask = 0;
	head_mask = 0;
	head_base = 0;
	body_base = 0;
	arms_base = 0;
	legs_base = 0;
	boots_base = 0;
	hair_tex = 0;
	weapon_tex = 0;
	shield_tex = 0;
	helmet_tex = 0;
	neck_tex = 0;
	cape_tex = 0;
	hands_tex_save = 0;
	alpha = 0;

	if (have_extension(ext_texture_compression_s3tc))
	{
		format = ift_dxt1;
	}
	else
	{
		format = ift_rgba8;
	}

	if (poor_man != 0)
	{
		sizes = 0x0003;
		format = ift_rgba8;
	}
	else
	{
		sizes = 0x000F;
	}

	if (files->pants_tex[0])
	{
		open_for_coordinates_mask2(files->pants_tex,
			files->legs_base, files->pants_mask, &pants_tex,
			&legs_base, &pants_mask, 40, 40, &sizes,
			&format);
	}
	if (files->boots_tex[0])
	{
		open_for_coordinates_mask2(files->boots_tex,
			files->boots_base, files->boots_mask, &boots_tex,
			&boots_base, &boots_mask, 39, 40, &sizes,
			&format);
	}
	if (files->torso_tex[0])
	{
		open_for_coordinates_mask2(files->torso_tex,
			files->body_base, files->torso_mask, &torso_tex,
			&body_base, &torso_mask, 49, 54, &sizes,
			&format);
	}
	if (files->arms_tex[0])
	{
		open_for_coordinates_mask2(files->arms_tex,
			files->arms_base, files->arms_mask, &arms_tex,
			&arms_base, &arms_mask, 40, 40, &sizes, 
			&format);
	}
	if (files->hands_tex[0])
	{
		open_for_coordinates_mask2(files->hands_tex,
			files->hands_tex_save, files->hands_mask, &hands_tex,
			&hands_tex_save, &hands_mask, 16, 16, &sizes,
			&format);
	}
	if (files->head_tex[0])
	{
		open_for_coordinates_mask2(files->head_tex,
			files->head_base, files->head_mask, &head_tex,
			&head_base, &head_mask, 32, 32, &sizes, 
			&format);
	}
	if (files->hair_tex[0])
	{
		open_for_coordinates(files->hair_tex, &hair_tex,
			34, 48, &sizes, &format);
	}
	if (files->weapon_tex[0])
	{
		open_for_coordinates(files->weapon_tex, &weapon_tex,
			39, 36, &sizes, &format);
	}
	if (files->shield_tex[0])
	{
		open_for_coordinates(files->shield_tex, &shield_tex,
			39, 36, &sizes, &format);
	}
	if (files->helmet_tex[0])
	{
		open_for_coordinates(files->helmet_tex, &helmet_tex,
			39, 14, &sizes, &format);
	}
	if (files->neck_tex[0])
	{
		open_for_coordinates(files->neck_tex, &neck_tex,
			10, 26, &sizes, &format);
	}
	if (files->cape_tex[0])
	{
		open_for_coordinates(files->cape_tex, &cape_tex,
			62, 38, &sizes, &format);
	}

	scale = 0;

	for (i = 15; i >= 0; i--)
	{
		scale = 1 << i;

		if ((sizes & scale) == scale)
		{
			break;
		}
	}

	if (scale < 4)
	{
		format = ift_rgba8;
	}

	width = 128 * scale;
	height = 128 * scale;

	switch (format)
	{
		case ift_dxt1:
			use_compressed_image = 1;
			size = width * height / 2;
			break;
		case ift_dxt3:
			use_compressed_image = 1;
			size = width * height;
			break;
		case ift_dxt5:
			use_compressed_image = 1;
			size = width * height;
			break;
		case ift_rgba8:
			use_compressed_image = 0;
			size = width * height * 4;
			break;
		default:
			use_compressed_image = 0;
			size = width * height * 4;
			format = ift_rgba8;
			break;
	}

	memset(image, 0, sizeof(image_t));

	image->sizes[0] = size;
	image->width = width;
	image->height = height;
	image->mipmaps = 1;
	image->format = format;
	image->image = malloc_aligned(size, 16);
	memset(image->image, 0xFF, size);

	alpha = 0;

	if (pants_tex != 0)
	{
		alpha += load_to_coordinates_mask2(pants_tex, legs_base,
			pants_mask, 39, 88, 40, 40, use_compressed_image,
			scale, image, buffer);
	}
	if (boots_tex != 0)
	{
		alpha += load_to_coordinates_mask2(boots_tex, boots_base,
			boots_mask, 0, 88, 39, 40, use_compressed_image,
			scale, image, buffer);
	}
	if (torso_tex != 0)
	{
		alpha += load_to_coordinates_mask2(torso_tex, body_base,
			torso_mask, 79, 74, 49, 54, use_compressed_image,
			scale, image, buffer);
	}
	if (arms_tex != 0)
	{
		alpha += load_to_coordinates_mask2(arms_tex, arms_base,
			arms_mask, 0, 48, 40, 40, use_compressed_image,
			scale, image, buffer);
	}
	if (hands_tex != 0)
	{
		alpha += load_to_coordinates_mask2(hands_tex, hands_tex_save,
			hands_mask, 34, 32, 16, 16, use_compressed_image,
			scale, image, buffer);
	}
	if (head_tex != 0)
	{
		alpha += load_to_coordinates_mask2(head_tex, head_base,
			head_mask, 34, 0, 32, 32, use_compressed_image,
			scale, image, buffer);
	}
	if (hair_tex != 0)
	{
		alpha += load_to_coordinates(hair_tex, 0, 0, 34, 48,
			use_compressed_image, scale, image);
	}
	if (weapon_tex != 0)
	{
		alpha += load_to_coordinates(weapon_tex, 89, 38, 39, 36,
			use_compressed_image, scale, image);
	}
	if (shield_tex != 0)
	{
		alpha += load_to_coordinates(shield_tex, 50, 38, 39, 36,
			use_compressed_image, scale, image);
	}
	if (helmet_tex != 0)
	{
		alpha += load_to_coordinates(helmet_tex, 40, 74, 39, 14,
			use_compressed_image, scale, image);
	}
	if (neck_tex != 0)
	{
		alpha += load_to_coordinates(neck_tex, 40, 48, 10, 26,
			use_compressed_image, scale, image);
	}
	if (cape_tex != 0)
	{
		alpha += load_to_coordinates(cape_tex, 66, 0, 62, 38,
			use_compressed_image, scale, image);
	}

	if (alpha > 0)
	{
		alpha = 1;
	}

	image->alpha = alpha;
}

static void copy_enhanced_actor_file_name(char* dest, const char* source)
{
	safe_strncpy2(dest, source, MAX_FILE_PATH, get_file_name_len(source));
}

Uint32 load_enhanced_actor(const enhanced_actor* actor, const char* name)
{
	enhanced_actor_images_t files;
	char str[MAX_ACTOR_NAME];
	Uint32 i, handle, hash, access_time;

	memset(str, 0, sizeof(str));

	if (name != 0)
	{
		safe_strncpy2(str, name, sizeof(str), strlen(name));
	}

	memset(&files, 0, sizeof(files));

	copy_enhanced_actor_file_name(files.pants_tex, actor->pants_tex);
	copy_enhanced_actor_file_name(files.pants_mask, actor->pants_mask);

	copy_enhanced_actor_file_name(files.boots_tex, actor->boots_tex);
	copy_enhanced_actor_file_name(files.boots_mask, actor->boots_mask);

	copy_enhanced_actor_file_name(files.torso_tex, actor->torso_tex);
	copy_enhanced_actor_file_name(files.arms_tex, actor->arms_tex);
	copy_enhanced_actor_file_name(files.torso_mask, actor->torso_mask);
	copy_enhanced_actor_file_name(files.arms_mask, actor->arms_mask);

	copy_enhanced_actor_file_name(files.hands_tex, actor->hands_tex);
	copy_enhanced_actor_file_name(files.head_tex, actor->head_tex);
	copy_enhanced_actor_file_name(files.hands_mask, actor->hands_mask);
	copy_enhanced_actor_file_name(files.head_mask, actor->head_mask);

	copy_enhanced_actor_file_name(files.head_base, actor->head_base);
	copy_enhanced_actor_file_name(files.body_base, actor->body_base);
	copy_enhanced_actor_file_name(files.arms_base, actor->arms_base);
	copy_enhanced_actor_file_name(files.legs_base, actor->legs_base);
	copy_enhanced_actor_file_name(files.boots_base, actor->boots_base);

	copy_enhanced_actor_file_name(files.hair_tex, actor->hair_tex);
	copy_enhanced_actor_file_name(files.weapon_tex, actor->weapon_tex);
	copy_enhanced_actor_file_name(files.shield_tex, actor->shield_tex);
	copy_enhanced_actor_file_name(files.helmet_tex, actor->helmet_tex);
	copy_enhanced_actor_file_name(files.neck_tex, actor->neck_tex);
	copy_enhanced_actor_file_name(files.cape_tex, actor->cape_tex);
	copy_enhanced_actor_file_name(files.hands_tex_save, actor->hands_tex_save);

	hash = mem_hash(&files, sizeof(files));

	assert(actor_texture_handles != 0);
	handle = ACTOR_TEXTURE_CACHE_MAX;

	for (i = 0; i < ACTOR_TEXTURE_CACHE_MAX; i++)
	{
		if ((handle != ACTOR_TEXTURE_CACHE_MAX) &&
			(i >= max_actor_texture_handles))
		{
			// We have one empty slot and won't find a better one,
			// so leave
			break;
		}

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[i].mutex);

		if (actor_texture_handles[i].used != 0)
		{
			CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[i].mutex);

			continue;
		}

		if (hash == actor_texture_handles[i].hash)
		{
			if (memcmp(&actor_texture_handles[i].files,
				&files, sizeof(files)) == 0)
			{
				// already loaded, use existing texture
				actor_texture_handles[i].used = 1;
				actor_texture_handles[i].access_time = cur_time;

				// already loaded, release lock
				CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[i].mutex);

				// release old lock!
				if (handle != ACTOR_TEXTURE_CACHE_MAX)
				{
					CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);
				}

				queue_push_signal(actor_texture_queue, &actor_texture_handles[i]);

				return i;
			}
		}

		// remember the first open slot we have
		if (handle == ACTOR_TEXTURE_CACHE_MAX)
		{
			// Don't unlock mutex! We plan to use this slot!
			handle = i;

			// Use the same slot if we can
			if (memcmp(str, actor_texture_handles[i].name,
				sizeof(str)) == 0)
			{
				// Best fit, we can stopp searching
				break;
			}

			access_time = actor_texture_handles[i].access_time;
		}
		else
		{
			// Use the same slot if we can
			if (memcmp(str, actor_texture_handles[i].name,
				sizeof(str)) == 0)
			{
				// release old lock!
				CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

				handle = i;

				// Best fit, we can stopp searching
				break;
			}

			// Only use an older one if we don't use too many!
			if (access_time > actor_texture_handles[i].access_time)
			{
				// release old lock!
				CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

				// Don't unlock mutex! We plan to use this slot!
				handle = i;
				access_time = actor_texture_handles[i].access_time;
			}
			else
			{
				// release lock
				CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[i].mutex);
			}
		}
	}

	if (handle < ACTOR_TEXTURE_CACHE_MAX)
	{
		free_actor_texture_resources(&actor_texture_handles[handle]);

		memcpy(&actor_texture_handles[handle].files, &files,
			sizeof(files));

		actor_texture_handles[handle].hash = hash;
		safe_strncpy2(actor_texture_handles[handle].name, str,
			sizeof(actor_texture_handles[handle].name),
			strlen(str));
		actor_texture_handles[handle].used = 1;
		actor_texture_handles[handle].access_time = cur_time;

		CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

		queue_push_signal(actor_texture_queue, &actor_texture_handles[handle]);

		return handle;
	}
	else
	{
		LOG_ERROR("Error: out of texture space\n");

		return ACTOR_TEXTURE_CACHE_MAX;	// ERROR!
	}
}

Uint32 bind_actor_texture(const Uint32 handle, char* alpha)
{
	Uint32 af, result;
	GLuint id;
	GLenum min_filter;
	texture_format_type format;

	if (handle >= ACTOR_TEXTURE_CACHE_MAX)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			ACTOR_TEXTURE_CACHE_MAX);

		return 0;
	}

	CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

#ifdef	DEBUG
	if (actor_texture_handles[handle].used == 0)
	{
		LOG_ERROR("actor texture used value is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return 0;
	}
#endif	/* DEBUG */

	actor_texture_handles[handle].access_time = cur_time;

	if (alpha != 0)
	{
		*alpha = actor_texture_handles[handle].image.alpha;
	}

	if (actor_texture_handles[handle].id != 0)
	{
		bind_texture_id(actor_texture_handles[handle].id);

		result = 1;
	}
	else
	{
		result = 0;
	}

	if (actor_texture_handles[handle].state == tst_image_loaded)
	{
		if (poor_man != 0)
		{
			min_filter = GL_LINEAR_MIPMAP_NEAREST;
		}
		else
		{
			min_filter = GL_LINEAR_MIPMAP_LINEAR;
			af = 1;
		}

		format = tft_auto;

		if (have_extension(ext_texture_compression_s3tc))
		{
			if (actor_texture_handles[handle].image.alpha == 0)
			{
				format = tft_dxt1;
			}
			else
			{
				format = tft_dxt5;
			}
		}

		id = build_texture(&actor_texture_handles[handle].image,
			0, min_filter, af, format);

		CHECK_GL_ERRORS();

		free_image(&actor_texture_handles[handle].image);

		actor_texture_handles[handle].image.image = 0;

		if (actor_texture_handles[handle].id == 0)
		{
			actor_texture_handles[handle].id = id;
			actor_texture_handles[handle].state = tst_texture_loaded;
		}
		else
		{
			if (actor_texture_handles[handle].new_id != 0)
			{
				LOG_ERROR("New texture id in use at texture"
					" handle: %i.", handle);

				glDeleteTextures(1, &actor_texture_handles[handle].new_id);
			}

			actor_texture_handles[handle].new_id = id;
			actor_texture_handles[handle].state = tst_texture_loading;
		}
	}

	CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

	return result;
}

void free_actor_texture(const Uint32 handle)
{
	if (handle >= ACTOR_TEXTURE_CACHE_MAX)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			ACTOR_TEXTURE_CACHE_MAX);

		return;
	}

	CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

#ifdef	DEBUG
	if (actor_texture_handles[handle].used == 0)
	{
		LOG_ERROR("actor texture used value is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return;
	}
#endif	/* DEBUG */

	actor_texture_handles[handle].used = 0;

	if (handle >= max_actor_texture_handles)
	{
		free_actor_texture_resources(&actor_texture_handles[handle]);
	}

	CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);
}

Uint32 get_actor_texture_ready(const Uint32 handle)
{
	Uint32 result;

	if (handle >= ACTOR_TEXTURE_CACHE_MAX)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			ACTOR_TEXTURE_CACHE_MAX);

		return 0;
	}

	CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

#ifdef	DEBUG
	if (actor_texture_handles[handle].used == 0)
	{
		LOG_ERROR("actor texture used value is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return 0;
	}
#endif	/* DEBUG */

	if (actor_texture_handles[handle].state == tst_texture_loading)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}

	CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

	return result;
}

void use_ready_actor_texture(const Uint32 handle)
{
	if (handle >= ACTOR_TEXTURE_CACHE_MAX)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			ACTOR_TEXTURE_CACHE_MAX);

		return;
	}

	CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

#ifdef	DEBUG
	if (actor_texture_handles[handle].used == 0)
	{
		LOG_ERROR("actor texture used value is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return;
	}

	if (actor_texture_handles[handle].state != tst_texture_loading)
	{
		LOG_ERROR("actor texture not uploaded: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return;
	}
#endif	/* DEBUG */

	actor_texture_handles[handle].state = tst_texture_loaded;

#ifdef	DEBUG
	if (actor_texture_handles[handle].id == 0)
	{
		LOG_ERROR("actor texture is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return;
	}
#endif	/* DEBUG */

	if (actor_texture_handles[handle].id != 0)
	{
		glDeleteTextures(1, &actor_texture_handles[handle].id);
	}

	actor_texture_handles[handle].id =
		actor_texture_handles[handle].new_id;

	actor_texture_handles[handle].new_id = 0;

	CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);
}

void change_enhanced_actor(const Uint32 handle, enhanced_actor* actor)
{
	enhanced_actor_images_t files;
	Uint32 hash;

	if (handle >= ACTOR_TEXTURE_CACHE_MAX)
	{
		LOG_ERROR("handle: %i, max_handle: %i\n", handle,
			ACTOR_TEXTURE_CACHE_MAX);

		return;
	}

	memset(&files, 0, sizeof(files));

	copy_enhanced_actor_file_name(files.pants_tex, actor->pants_tex);
	copy_enhanced_actor_file_name(files.pants_mask, actor->pants_mask);

	copy_enhanced_actor_file_name(files.boots_tex, actor->boots_tex);
	copy_enhanced_actor_file_name(files.boots_mask, actor->boots_mask);

	copy_enhanced_actor_file_name(files.torso_tex, actor->torso_tex);
	copy_enhanced_actor_file_name(files.arms_tex, actor->arms_tex);
	copy_enhanced_actor_file_name(files.torso_mask, actor->torso_mask);
	copy_enhanced_actor_file_name(files.arms_mask, actor->arms_mask);

	copy_enhanced_actor_file_name(files.hands_tex, actor->hands_tex);
	copy_enhanced_actor_file_name(files.head_tex, actor->head_tex);
	copy_enhanced_actor_file_name(files.hands_mask, actor->hands_mask);
	copy_enhanced_actor_file_name(files.head_mask, actor->head_mask);

	copy_enhanced_actor_file_name(files.head_base, actor->head_base);
	copy_enhanced_actor_file_name(files.body_base, actor->body_base);
	copy_enhanced_actor_file_name(files.arms_base, actor->arms_base);
	copy_enhanced_actor_file_name(files.legs_base, actor->legs_base);
	copy_enhanced_actor_file_name(files.boots_base, actor->boots_base);

	copy_enhanced_actor_file_name(files.hair_tex, actor->hair_tex);
	copy_enhanced_actor_file_name(files.weapon_tex, actor->weapon_tex);
	copy_enhanced_actor_file_name(files.shield_tex, actor->shield_tex);
	copy_enhanced_actor_file_name(files.helmet_tex, actor->helmet_tex);
	copy_enhanced_actor_file_name(files.neck_tex, actor->neck_tex);
	copy_enhanced_actor_file_name(files.cape_tex, actor->cape_tex);
	copy_enhanced_actor_file_name(files.hands_tex_save, actor->hands_tex_save);

	hash = mem_hash(&files, sizeof(files));

	CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

#ifdef	DEBUG
	if (actor_texture_handles[handle].used == 0)
	{
		LOG_ERROR("actor texture used value is invalid: %i.", handle);

		CHECK_AND_LOCK_MUTEX(actor_texture_handles[handle].mutex);

		return;
	}
#endif	/* DEBUG */

 	if (hash == actor_texture_handles[handle].hash)
 	{
 		if (!memcmp(&actor_texture_handles[handle].files, &files,
 			sizeof(files)))
 		{
 			actor_texture_handles[handle].access_time = cur_time;
 
 			CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);
 
 			return;
 		}
 	}

	memcpy(&actor_texture_handles[handle].files, &files, sizeof(files));

	actor_texture_handles[handle].hash = hash;
	actor_texture_handles[handle].access_time = cur_time;

	reload_actor_texture_resources(&actor_texture_handles[handle]);

	CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[handle].mutex);

	queue_push_signal(actor_texture_queue, &actor_texture_handles[handle]);
}

int load_enhanced_actor_thread(void* done)
{
	enhanced_actor_images_t files;
	image_t image;
	actor_texture_cache_t* actor;
	Uint8* buffer;
	Uint32 hash;

	init_thread_log("load_actors");

	buffer = malloc_aligned(TEXTURE_SIZE_X * TEXTURE_SIZE_Y * 4, 16);

	while (*((Uint32*)done) == 0)
	{
		actor = queue_pop_blocking(actor_texture_queue);

		if (actor == 0)
		{
			continue;
		}

		CHECK_AND_LOCK_MUTEX(actor->mutex);

		while (actor->state == tst_unloaded)
		{
			memcpy(&files, &actor->files, sizeof(files));
			hash = actor->hash;
			actor->state = tst_image_loading;

			CHECK_AND_UNLOCK_MUTEX(actor->mutex);

			load_enhanced_actor_threaded(&files, &image, buffer);

			CHECK_AND_LOCK_MUTEX(actor->mutex);

			if ((actor->state == tst_image_loading) ||
				(actor->state == tst_unloaded))
			{
				if ((hash == actor->hash) &&
					(!memcmp(&actor->files, &files,	sizeof(files))))
				{
					memcpy(&actor->image, &image, sizeof(image));

					actor->state = tst_image_loaded;
				}
				else
				{
					free_image(&image);

					actor->state = tst_unloaded;
				}
			}
			else
			{
				LOG_ERROR("Wrong actor state %d", actor->state); 
				free_image(&image);
			}
		}

		CHECK_AND_UNLOCK_MUTEX(actor->mutex);
	}

	free_aligned(buffer);

	return 1;
}

void unload_actor_texture_cache()
{
	Uint32 i, used;

	if (actor_texture_handles != 0)
	{
		while (queue_pop(actor_texture_queue) != 0);

		for (i = 0; i < ACTOR_TEXTURE_CACHE_MAX; i++)
		{
			CHECK_AND_LOCK_MUTEX(actor_texture_handles[i].mutex);

			used = actor_texture_handles[i].used;

			free_actor_texture_resources(&actor_texture_handles[i]);

			CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[i].mutex);

			if (used != 0)
			{
				queue_push_signal(actor_texture_queue,
					&actor_texture_handles[i]);
			}
		}
	}
}
#endif	/* ELC */

void init_texture_cache()
{
#ifdef	ELC
	Uint32 i;
#endif	/* ELC */

	texture_cache = cache_init("texture cache", TEXTURE_CACHE_MAX, 0);
	cache_set_compact(texture_cache, compact_texture);
	cache_set_time_limit(texture_cache, 5 * 60 * 1000);

	texture_handles = calloc(TEXTURE_CACHE_MAX, sizeof(texture_cache_t));
#ifdef	ELC
	actor_texture_handles = calloc(ACTOR_TEXTURE_CACHE_MAX, sizeof(actor_texture_cache_t));

	queue_initialise(&actor_texture_queue);

	for (i = 0; i < ACTOR_TEXTURE_CACHE_MAX; i++)
	{
		actor_texture_handles[i].mutex = SDL_CreateMutex();
		actor_texture_handles[i].state = tst_unloaded;
	}

	for (i = 0; i < ACTOR_TEXTURE_THREAD_COUNT; i++)
	{
		actor_texture_threads[i] = SDL_CreateThread(
			load_enhanced_actor_thread, &actor_texture_threads_done);
	}
#endif	/* ELC */
}

void free_texture_cache()
{
	Uint32 i;
#ifdef	ELC
	int result;

	actor_texture_threads_done = 1;

	while (queue_pop(actor_texture_queue) != 0);

	for (i = 0; i < ACTOR_TEXTURE_THREAD_COUNT; i++)
	{
		SDL_CondBroadcast(actor_texture_queue->condition);
		SDL_WaitThread(actor_texture_threads[i], &result);
	}

	queue_destroy(actor_texture_queue);

	for (i = 0; i < ACTOR_TEXTURE_CACHE_MAX; i++)
	{
		free_actor_texture_resources(&actor_texture_handles[i]);

		if (actor_texture_handles[i].mutex != 0)
		{
			SDL_DestroyMutex(actor_texture_handles[i].mutex);
		}
	}

	free(actor_texture_handles);
#endif	/* ELC */

	for (i = 0; i < texture_handles_used; i++)
	{
		if (texture_handles[i].id != 0)
		{
			glDeleteTextures(1, &texture_handles[i].id);
		}
	}

	free(texture_handles);

	cache_delete(texture_cache);
	texture_cache = NULL;
}

void unload_texture_cache()
{
	Uint32 i;

	for (i = 0; i < texture_handles_used; i++)
	{
		if (texture_handles[i].id != 0)
		{
			glDeleteTextures(1, &texture_handles[i].id);

			texture_handles[i].id = 0;

			cache_adj_size(texture_cache, -texture_handles[i].size,
				&texture_handles[i]);
		}
	}

#ifdef	ELC
	unload_actor_texture_cache();
#endif	/* ELC */
}

#ifdef	DEBUG
void dump_texture_cache()
{
	Uint32 i;

	if (actor_texture_handles != 0)
	{
		for (i = 0; i < max_actor_texture_handles; i++)
		{
			CHECK_AND_LOCK_MUTEX(actor_texture_handles[i].mutex);

			printf("%i: id %d, new_id %d, hash 0x%X, "
				"used %d, access_time %d, state %d\n", i,
				actor_texture_handles[i].id,
				actor_texture_handles[i].new_id,
				actor_texture_handles[i].hash,
				actor_texture_handles[i].used,
				actor_texture_handles[i].access_time,
				actor_texture_handles[i].state);

			CHECK_AND_UNLOCK_MUTEX(actor_texture_handles[i].mutex);
		}
	}
}
#endif	/* DEBUG */
#else	// NEW_TEXTURES

#ifdef FASTER_MAP_LOAD
static int texture_cache_sorted[TEXTURE_CACHE_MAX];
static int texture_cache_used = 0;
#endif

#ifdef NEW_CURSOR
//Some textures just can't be compressed (written for custom cursors)
static int compression_enabled = 1;

void enable_compression ()
{
	compression_enabled = 1;
}

void disable_compression ()
{
	compression_enabled = 0;
}
#endif // NEW_CURSOR

__inline__ static void set_texture_filter(texture_filter filter, float anisotropic_filter)
{
	switch (filter)
	{
		case TF_POINT:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case TF_BILINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case TF_TRILINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			break;
		case TF_ANISOTROPIC:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				anisotropic_filter);
			break;
		case TF_ANISOTROPIC_AND_MIPMAPS:
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				anisotropic_filter);
			break;
		default:
			LOG_ERROR("Unsupported texture filter (%d)", filter);
			break;
	}
}

__inline__ static void set_texture_filter_parameter()
{
	if (poor_man)
	{
		set_texture_filter(TF_POINT, 0.0f);
	}
	else
	{
		if (use_mipmaps)
		{
			glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
			if (anisotropic_filter > 1.0f)
			{
				set_texture_filter (TF_ANISOTROPIC_AND_MIPMAPS, anisotropic_filter);
			}
			else
			{
				set_texture_filter(TF_TRILINEAR, 0.0f);
			}
		}		
		else
		{
			if (anisotropic_filter > 1.0f)
			{
				set_texture_filter(TF_ANISOTROPIC, anisotropic_filter);
			}
			else
			{
				set_texture_filter(TF_BILINEAR, 0.0f);
			}
		}
	}
}


static texture_struct *load_texture_SDL(el_file_ptr file, const char * file_name, texture_struct *tex, Uint8 alpha)
{
	SDL_Surface *texture_surface;
	GLubyte* data;
	int texture_width, texture_height, idx;
	int pixel, temp, r, g, b, a;
	int bpp, i, j, index, x_padding;

	if (file == 0)
	{
		return 0;
	}

	texture_surface = IMG_Load_RW(SDL_RWFromMem(el_get_pointer(file), el_get_size(file)), 1);

	if (texture_surface == 0)
	{
		LOG_ERROR("load_texture() error: [%s] [%s]", file_name, IMG_GetError());
		return 0;
	}

	// at this point, theTextureSurface contains some type of pixel data.
	// SDL requires us to lock the surface before using the pixel data:
	SDL_LockSurface(texture_surface);
	tex->has_alpha = 0;
	tex->x_size = texture_surface->w;
	tex->y_size = texture_surface->h;

	texture_width = texture_surface->w;
	texture_height = texture_surface->h;

	x_padding = texture_width % 4;

	if (x_padding)
	{
		x_padding = 4 - x_padding;
	}
	if (texture_width <= x_padding)
	{
		x_padding = 0;
	}

	tex->texture = (GLubyte*)malloc(texture_width * texture_height * 4 * sizeof(GLubyte));
	data = tex->texture;

	idx = 0;
	pixel = 0;
	bpp = texture_surface->format->BytesPerPixel;

	for (i = 0; i < texture_height; i++)
	{
		for (j = 0; j < texture_width; j++)
		{
			if ((texture_surface->format->BitsPerPixel == 8) &&
				(texture_surface->format->palette != 0))
			{
				index = ((Uint8 *)texture_surface->pixels)[idx];
				r = texture_surface->format->palette->colors[index].r;
				g = texture_surface->format->palette->colors[index].g;
				b = texture_surface->format->palette->colors[index].b;
				a = (r + g + b) / 3;
			}
			else
			{
				memcpy(&pixel, &((Uint8 *)texture_surface->pixels)[idx], bpp);
				/* Get Red component */
				temp = pixel & texture_surface->format->Rmask;  /* Isolate red component */
				temp = temp >> texture_surface->format->Rshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Rloss;  /* Expand to a full 8-bit number */
				r = (Uint8)temp;

				/* Get Green component */
				temp = pixel & texture_surface->format->Gmask;  /* Isolate green component */
				temp = temp >> texture_surface->format->Gshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Gloss;  /* Expand to a full 8-bit number */
				g = (Uint8)temp;

				/* Get Blue component */
				temp = pixel & texture_surface->format->Bmask;  /* Isolate blue component */
				temp = temp >> texture_surface->format->Bshift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Bloss;  /* Expand to a full 8-bit number */
				b = (Uint8)temp;

				/* Get Alpha component */
				temp = pixel & texture_surface->format->Amask;  /* Isolate alpha component */
				temp = temp >> texture_surface->format->Ashift; /* Shift it down to 8-bit */
				temp = temp << texture_surface->format->Aloss;  /* Expand to a full 8-bit number */
				a = (Uint8)temp;
			}
			if (alpha)
			{
				a = alpha;
			}
			idx += bpp;

			index = (texture_height - i - 1) * texture_width + j;
			data[index * 4 + 0] = r;
			data[index * 4 + 1] = g;
			data[index * 4 + 2] = b;
			data[index * 4 + 3] = a;
		}
		idx += bpp * x_padding;
	}

	SDL_UnlockSurface(texture_surface);
	SDL_FreeSurface(texture_surface);

	return tex;
}

texture_struct *load_texture(const char * file_name, texture_struct *tex, Uint8 alpha)
{
	el_file_ptr file;
	
	Uint32 dds;

	file = el_open_custom(file_name);

	if (file == 0)
	{
		return 0;
	}

	dds = 0;

	if (el_get_size(file) >= 4)
	{
		if (check_dds(el_get_pointer(file)))
		{
			dds = 1;
		}
	}

	if (dds == 1)
	{
		tex->has_alpha = 0;
		tex->texture = load_dds(file, &tex->x_size, &tex->y_size);
	}
	else
	{
		tex = load_texture_SDL(file, file_name, tex, alpha);
	}

	el_close(file);

	if (tex == 0 || tex->texture == 0)
	{
		return 0;
	}



	return tex;
}

// set a default alpha value except where it's black, make that transparent
void	texture_set_alpha(texture_struct *tex, Uint8 alpha, int cutoff)
{
	int	y;
	Uint8 * texture_mem= tex->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;
			if((texture_mem[texture_offset] + texture_mem[texture_offset+1] + texture_mem[texture_offset+2])/3 > cutoff){
				texture_mem[texture_offset+3]= alpha;
			} else {
				texture_mem[texture_offset+3]= 0;
			}
		}
	}
}

//TODO: add alpha mask at location
// warning, textures need to be the same size
// set the alpha on the texture from an alpha mask, results go into the first texture
void	texture_alpha_mask(texture_struct *tex, texture_struct *mask)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= mask->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;
			texture_mem[texture_offset+3]= btexture_mem[texture_offset+3];
		}
	}
}


//TODO: blend overlay at location
// warning, textures need to be the same size
// alpha blend two textures, results go into the first texture
void	texture_alpha_blend(texture_struct *tex, texture_struct *blend)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= blend->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			int alpha;
			int texture_offset= (y_offset+x)*4;

			alpha= btexture_mem[texture_offset+3];
			if(alpha > 0){
				int	beta= 255-alpha;

				texture_mem[texture_offset]= (beta*texture_mem[texture_offset] + alpha*btexture_mem[texture_offset])/255;
				texture_mem[texture_offset+1]= (beta*texture_mem[texture_offset+1] + alpha*btexture_mem[texture_offset+1])/255;
				texture_mem[texture_offset+2]= (beta*texture_mem[texture_offset+2] + alpha*btexture_mem[texture_offset+2])/255;
				// calc a new alpha just to be complete
				texture_mem[texture_offset+3]= (texture_mem[texture_offset]+texture_mem[texture_offset+1]+texture_mem[texture_offset+2])/3;
			}
		}
	}
}

//TODO: overlay at location
// warning, textures need to be the same size
// overlay one texture on another is there is any alpha, results go into the first texture
void	texture_overlay(texture_struct *tex, texture_struct *blend)
{
	int	y;
	Uint8 * texture_mem= tex->texture;
	Uint8 * btexture_mem= blend->texture;

	for(y=0; y<tex->y_size; y++)
	{
		int	x;
		int	y_offset= y*tex->x_size;

		for(x=0; x<tex->x_size; x++)
		{
			unsigned char alpha;
			int texture_offset= (y_offset+x)*4;

			alpha= btexture_mem[texture_offset+3];
			if(alpha > 0){
				texture_mem[texture_offset]= texture_mem[texture_offset];
				texture_mem[texture_offset+1]= texture_mem[texture_offset+1];
				texture_mem[texture_offset+2]= texture_mem[texture_offset+2];
				texture_mem[texture_offset+3]= texture_mem[texture_offset+3];
			}
		}
	}
}

// warning, textures need to be the same size
// do three texture masking to combine three textures, the result goes in the first texture
void	texture_mask2(texture_struct *texR, texture_struct *texG, texture_struct *mask)
{
	int	y;
	Uint8 * textureR;
	Uint8 * textureG;
	Uint8 * textureM;

	if(texG == NULL)	return;	// NOP
	if(mask == NULL)	return;	// NOP
	textureR= texR->texture;
	textureG= texG->texture;
	textureM= mask->texture;

	for(y=0; y<texR->y_size; y++)
	{
		int	x;
		int	y_offset= y*texR->x_size;

		for(x=0; x<texR->x_size; x++)
		{
			Uint8 r,g;
			Uint8 * tex;
			int texture_offset= (y_offset+x)*4;

			r= textureM[texture_offset+0];
			g= textureM[texture_offset+1];
			// decide which one to use
			if(r == 0 && g == 0){
				//tex= NULL;	// no copy needed
				continue;
			} else if(r >= g){
				//tex= NULL;	// no copy needed
				continue;
			} else if(g >= r){
				tex= textureG;
			} else {
				//tex= NULL;	// no copy needed
				continue;
			}

			//if(tex){
				textureR[texture_offset]= tex[texture_offset];
				textureR[texture_offset+1]= tex[texture_offset+1];
				textureR[texture_offset+2]= tex[texture_offset+2];
				textureR[texture_offset+3]= tex[texture_offset+3];
			//}
		}
	}
}

// warning, textures need to be the same size
// do three texture masking to combine three textures, the result goes in the first texture
void	texture_mask3(texture_struct *texR, texture_struct *texG, texture_struct *texB, texture_struct *mask)
{
	int	y;
	Uint8 * textureR;
	Uint8 * textureG;
	Uint8 * textureB;
	Uint8 * textureM;

	if(mask == NULL)	return;	// nothing to do
	// watch for a NULL value meaning use the texture before it
	if(texG == NULL)	texG= texR;
	if(texB == NULL)	texB= texG;
	textureR= texR->texture;
	textureG= texG->texture;
	textureB= texB->texture;
	textureM= mask->texture;

	for(y=0; y<texR->y_size; y++)
	{
		int	x;
		int	y_offset= y*texR->x_size;

		for(x=0; x<texR->x_size; x++)
		{
			Uint8 r,b,g;
			Uint8 * tex;
			int texture_offset= (y_offset+x)*4;

			r= textureM[texture_offset+0];
			g= textureM[texture_offset+1];
			b= textureM[texture_offset+2];
			// decide which one to use
			if(r == 0 && g == 0 && b == 0){
				//tex= NULL;	// no copy needed
				continue;
			} else if(r >= g && r >= b){
				//tex= NULL;	// no copy needed
				continue;
			} else if(g >= r && g >= b){
				tex= textureG;
			} else if(b >= r && b >= g){
				tex= textureB;
			} else {
				//tex= NULL;	// no copy needed
				continue;
			}

			//if(tex){
				textureR[texture_offset]= tex[texture_offset];
				textureR[texture_offset+1]= tex[texture_offset+1];
				textureR[texture_offset+2]= tex[texture_offset+2];
				textureR[texture_offset+3]= tex[texture_offset+3];
			//}
		}
	}
}


// get a texture id out of the texture cache
// if null and we haven't failed to load it before, then reload it 
// (means it was previously freed)

int get_texture_id(int i)
{
	int new_texture_id;
	int alpha;

	// don't look up an out of range texture
	if (i<0 || i>=TEXTURE_CACHE_MAX)
		return 0;
	
	if(!texture_cache[i].texture_id && !texture_cache[i].load_err)
	{
		// we need the alpha to know how to load it
		alpha= texture_cache[i].alpha;
		// our texture was freed, we have to reload it
		if(alpha <= 0) {
			new_texture_id= load_bmp8_color_key(&(texture_cache[i]), alpha);
		} else {
			new_texture_id= load_bmp8_fixed_alpha(&(texture_cache[i]), alpha);
		}
		texture_cache[i].texture_id= new_texture_id;
		if (!new_texture_id)
			texture_cache[i].load_err = 1;
	}
	return texture_cache[i].texture_id;
}

#ifndef	USE_INLINE
void	bind_texture_id(int texture_id)
{
	if(last_texture!=texture_id)
	{
		last_texture=texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
}

int get_and_set_texture_id(int i)
{
	int	texture_id;

#ifdef	DEBUG
	if(i<0||i>TEXTURE_CACHE_MAX) {
		LOG_ERROR("We tried binding a texture ID of %d\n", i);
		return 0;
	}
#endif	//DEBUG

	// do we need to make a hard load or do we already have it?
	if(!texture_cache[i].texture_id && !texture_cache[i].load_err)
	{
		texture_id= get_texture_id(i);
	} else {
		texture_id= texture_cache[i].texture_id;
	}
	bind_texture_id(texture_id);


	return(texture_id);
}
#endif	//USE_INLINE

int load_alphamap(const char * FileName, Uint8 * texture_mem, int orig_x_size, int orig_y_size)
{
	int x_size, y_size;
	texture_struct	texture;
	texture_struct	ttexture;
	texture_struct	*tex;
	char filename[1024];//Create a buffer...
	char * name;

	/* copy (maybe truncating) FileName into a buffer */
	safe_snprintf(filename, sizeof(filename), "%s", FileName);
	/* find last dot */
	name = strrchr(filename, '.');
	if (name == NULL)
	{
		name = filename;
	}
	/* terminate filename before last dot */
	*name = '\0';

	/* safely add '_alpha.bmp' to the string */
	safe_strcat (filename, "_alpha", sizeof (filename));
	name = strrchr(FileName, '.');
	if (name == NULL)
	{
		name = ".bmp";
	}
	safe_strcat (filename, name, sizeof (filename));

	// check for a file
	if (!el_custom_file_exists(filename))
	{
		return 0;	// no file
	}
	// read in the texture
	tex = load_texture(filename, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;

	if(x_size != orig_x_size || y_size != orig_y_size){
		LOG_ERROR("The alphamap for %s was not the same size as the original - we didn't load the alphamap...", FileName);
		free(tex->texture);
		return 0;
	}

	// now add this alpha to the original texture
	texture.x_size= x_size;
	texture.y_size= y_size;
	texture.texture= texture_mem;
	texture_alpha_mask(&texture, tex);

	free(tex->texture);
	return 1;
}

//load a bmp texture, in respect to the color key
GLuint load_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha)
{
	int x_size, y_size;
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;
	GLuint texture;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(tex_cache_entry->file_name, texture_mem, x_size, y_size) && alpha < 0){
#ifdef	NEW_ALPHA
		// no texture alpha found, use the constant
		if(alpha < -1){
			// a specific alpha threshold was mentioned
			texture_set_alpha(tex, 255, -alpha);
		} else {
			texture_set_alpha(tex, 255, 15);
	}
#endif	//NEW_ALPHA
	} else {
		tex->has_alpha++;
	}

    tex_cache_entry->has_alpha = tex->has_alpha;

	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	CHECK_GL_ERRORS();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();

	free(tex->texture);
	return texture;
}

//load a bmp texture, with the specified global alpha
GLuint load_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a)
{
	int x_size, y_size;
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;
	GLuint texture;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, a);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	CHECK_GL_ERRORS();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if(have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();

	free(tex->texture);
	return texture;
}

GLuint load_bmp8_fixed_alpha_with_transparent_color(texture_cache_struct * tex_cache_entry, Uint8 a,Uint8 tr,Uint8 tg,Uint8 tb)
{
	int x_size, y_size,i;
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;
	GLuint texture;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, a);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;
	tex->has_alpha = 1;

	for(i = 0; i<tex->x_size*tex->y_size*4; i+=4)
	{
		char r,g,b;
		r = texture_mem[i];
		g = texture_mem[i+1];
		b = texture_mem[i+2];
		if((r == tr) && (g == tg) && (b == tb))
		{
			texture_mem[i+3] = 0;
		}
	}

	CHECK_GL_ERRORS();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if(have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();

	free(tex->texture);
	return texture;
}

// reload a bmp texture, in respect to the color key
GLuint reload_bmp8_color_key(texture_cache_struct * tex_cache_entry, int alpha, GLuint texture)
{
	int x_size, y_size;
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, 0);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(tex_cache_entry->file_name, texture_mem, x_size, y_size) && alpha < 0){
#ifdef	NEW_ALPHA
		// no texture alpha found, use the constant
		if(alpha < -1){
			// a specific alpha threshold was mentioned
			texture_set_alpha(tex, 255, -alpha);
		} else {
			texture_set_alpha(tex, 255, 15);
	}
#endif	//NEW_ALPHA
	} else {
		tex->has_alpha++;
	}

    tex_cache_entry->has_alpha = tex->has_alpha;

	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	CHECK_GL_ERRORS();
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();

	free(tex->texture);
	return texture;
}

//reload a bmp texture, with the specified global alpha
GLuint reload_bmp8_fixed_alpha(texture_cache_struct * tex_cache_entry, Uint8 a, GLuint texture)
{
	int x_size, y_size;
	Uint8 *texture_mem;
	texture_struct	ttexture;
	texture_struct	*tex;

	tex = load_texture(tex_cache_entry->file_name, &ttexture, a);
	if(!tex){	// oops, failed
		return 0;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	CHECK_GL_ERRORS();
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();

	free(tex->texture);
	return texture;
}

#ifdef FASTER_MAP_LOAD
static int cache_cmp_string(const void* str, const void* iptr)
{
	int i = *((const int*)iptr);
	return strcasecmp(str, texture_cache[i].file_name);
}
#endif

//Tests to see if a texture is already loaded. If it is, return the handle.
//If not, load it, and return the handle
int load_texture_cache(const char* file_name, int alpha)
{
	int slot = load_texture_cache_deferred(file_name, alpha);
	get_and_set_texture_id(slot);
	return slot;
}

#ifdef FASTER_MAP_LOAD
int load_texture_cache_deferred(const char* file_name, int alpha)
{
	int i;
	int *iptr = bsearch(file_name, texture_cache_sorted, texture_cache_used,
		sizeof(int), cache_cmp_string);
	if (iptr)
		return *iptr;

	if (texture_cache_used < TEXTURE_CACHE_MAX)
	{
		int slot = texture_cache_used;
		for (i = 0; i < texture_cache_used; i++)
		{
			int idx = texture_cache_sorted[i];
			if (strcasecmp(file_name, texture_cache[idx].file_name) <= 0)
			{
				memmove(texture_cache_sorted+(i+1),
					texture_cache_sorted+i,
					(texture_cache_used-i)*sizeof(int));
				break;
			}
		}
		texture_cache_sorted[i] = slot;
		my_strncp(texture_cache[slot].file_name, file_name,
			sizeof(texture_cache[slot]));
		texture_cache[slot].texture_id = 0;
		texture_cache[slot].alpha = alpha;
		texture_cache[slot].has_alpha = 0;
		texture_cache_used++;
		return slot;
	}
	else
	{
		LOG_ERROR("Error: out of texture space\n");
		return 0;	// ERROR!
	}
}
#else  // FASTER_MAP_LOAD
int load_texture_cache_deferred(const char* file_name, int alpha)
{
	int i;
	int file_name_length;
	int texture_slot= -1;

	file_name_length=strlen(file_name);

	for(i=0;i<TEXTURE_CACHE_MAX;i++)
	{
		if(texture_cache[i].file_name[0])
		{
			if(!strcasecmp(texture_cache[i].file_name, file_name))
			{
				// already loaded, use existing texture
				return i;
			}
		}
		else
		{
			// remember the first open slot we have
			if(texture_slot < 0)
			{
				texture_slot= i;
			}
		}
	}

	if(texture_slot >= 0 && !texture_cache[texture_slot].file_name[0]) {//we found a place to store it
		safe_snprintf(texture_cache[texture_slot].file_name, sizeof(texture_cache[texture_slot].file_name), "%s", file_name);
		texture_cache[texture_slot].texture_id= 0;
		//if(texture_slot == 0) texture_cache[texture_slot].texture_id= 1;
		texture_cache[texture_slot].alpha= alpha;
        texture_cache[texture_slot].has_alpha = 0;
		return texture_slot;
	} else {	
		LOG_ERROR("Error: out of texture space\n");
		return 0;	// ERROR!
	}
}
#endif // FASTER_MAP_LOAD

#ifndef MAP_EDITOR2
void copy_bmp8_to_coordinates (texture_struct *tex, Uint8 *texture_space, int x_pos, int y_pos)
{
	int x, y, x_size, y_size;
	Uint8 *texture_mem;

	
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;


	for (y= 0; y<y_size; y++)
	{
		int	y_offset= y*x_size;
		int texture_y= (TEXTURE_SIZE_Y-1 - ((y_size-y-1) + y_pos))*TEXTURE_SIZE_X+x_pos;
		//int texture_y= (255 - ((y_size-y) + y_pos))*256+x_pos;

		for (x = 0; x < x_size; x++)
		{
			int texture_offset= (y_offset+x)*4;

			texture_space[(texture_y+x)*4] = texture_mem[texture_offset];
			texture_space[(texture_y+x)*4+1] = texture_mem[texture_offset+1];
			texture_space[(texture_y+x)*4+2] = texture_mem[texture_offset+2];
			texture_space[(texture_y+x)*4+3] = texture_mem[texture_offset+3];
		}
	}
}

texture_struct *load_bmp8_alpha (const char *filename, texture_struct *tex, Uint8 alpha)
{
	int x_size, y_size;
	Uint8 *texture_mem;

	tex = load_texture(filename, tex, alpha);
	if(!tex){	// oops, failed
		return NULL;
	}
	x_size= tex->x_size;
	y_size= tex->y_size;
	texture_mem= tex->texture;

	if(!load_alphamap(filename, texture_mem, x_size, y_size)){
		// no texture alpha found found, use the constant
		texture_set_alpha(tex, alpha, -1);
	} else {
		tex->has_alpha++;
	}

	return(tex);
}

int load_bmp8_to_coordinates (const char *filename, Uint8 *texture_space, int x_pos, int y_pos, Uint8 alpha)
{
	texture_struct	texture;
	texture_struct	*tex;

	tex= load_bmp8_alpha(filename, &texture, alpha);
	if(!tex || !tex->texture){	// oops, failed
		return 0;
	}
	copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);

	// release the temporary memory
	free(tex->texture);
	return(tex->has_alpha);
}

int load_bmp8_to_coordinates_mask2 (const char *filename, const char *basename, const char *maskname, Uint8 *texture_space, int x_pos, int y_pos, Uint8 alpha)
{
	//int x_size, y_size;
	texture_struct	texture;
	texture_struct	basetex;
	texture_struct	masktex;
	texture_struct	*tex;
	texture_struct	*base;
	texture_struct	*mask;

	// first, watch for being able to do simplified processing
	// no masking, or both textures are the same file
	if(!basename || !maskname || !*basename || !*maskname || !strcmp(filename, basename)){
		// yes, either the mask or the base is missing, just do a load
		return load_bmp8_to_coordinates(filename, texture_space, x_pos, y_pos, alpha);
	}
//LOG_ERROR("%s %s %s", filename, basename, maskname);
	tex= load_bmp8_alpha(filename, &texture, alpha);
	if(!tex){	// oops, failed
		return 0;
	}
	//x_size= tex->x_size;
	//y_size= tex->y_size;

	// now load the base in
	base= load_bmp8_alpha(basename, &basetex, alpha);
	if(!base){	// oops, failed
		// just place what we have and free the memory
		copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);
		free(tex->texture);
		return(tex->has_alpha);
	}

	// now load the mask in
	mask= load_bmp8_alpha(maskname, &masktex, alpha);
	if(!mask){	// oops, failed
		// just place what we have and free the memory
		copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);
		free(tex->texture);
		free(base->texture);
		return(tex->has_alpha);
	}

	// lets combine them all together into one
	texture_mask2(tex, base, mask);
	tex->has_alpha+= base->has_alpha;

	// and copy it where we need it
	copy_bmp8_to_coordinates(tex, texture_space, x_pos, y_pos);

	// release the temporary memory
	free(tex->texture);
	free(base->texture);
	free(mask->texture);

	return(tex->has_alpha);
}


#ifdef	ELC
int load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a)
{
	GLuint texture;
	Uint8 * texture_mem;
	int	has_alpha= 0;

	texture_mem=(Uint8*)calloc(1,TEXTURE_SIZE_X*TEXTURE_SIZE_Y*4);
	if(this_actor->pants_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->pants_tex,this_actor->legs_base,this_actor->pants_mask,texture_mem,78*TEXTURE_RATIO,175*TEXTURE_RATIO,a);
	}
	if(this_actor->boots_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->boots_tex,this_actor->boots_base,this_actor->boots_mask,texture_mem,0,175*TEXTURE_RATIO,a);
	}
	if(this_actor->torso_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->torso_tex,this_actor->body_base, this_actor->torso_mask, texture_mem,158*TEXTURE_RATIO,149*TEXTURE_RATIO,a);
	}
	if(this_actor->arms_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->arms_tex,this_actor->arms_base,this_actor->arms_mask,texture_mem,0,96*TEXTURE_RATIO,a);
	}
	if(this_actor->hands_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->hands_tex,this_actor->hands_tex_save,this_actor->hands_mask,texture_mem,67*TEXTURE_RATIO,64*TEXTURE_RATIO,a);
	}
	if(this_actor->head_tex[0]){
		has_alpha+= load_bmp8_to_coordinates_mask2(this_actor->head_tex,this_actor->head_base,this_actor->head_mask,texture_mem,67*TEXTURE_RATIO,0,a);
	}
	if(this_actor->hair_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->hair_tex,texture_mem,0,0,a);
	if(this_actor->weapon_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,178*TEXTURE_RATIO,77*TEXTURE_RATIO,a);
	if(this_actor->shield_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,100*TEXTURE_RATIO,77*TEXTURE_RATIO,a);
	if(this_actor->helmet_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->helmet_tex,texture_mem,80*TEXTURE_RATIO,149*TEXTURE_RATIO,a);
	if(this_actor->neck_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->neck_tex,texture_mem,80*TEXTURE_RATIO,96*TEXTURE_RATIO,a);
	if(this_actor->cape_tex[0])
		has_alpha+= load_bmp8_to_coordinates(this_actor->cape_tex,texture_mem,131*TEXTURE_RATIO,0,a);

	this_actor->has_alpha= has_alpha;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	set_texture_filter_parameter();

#ifndef NEW_CURSOR
	if (have_extension(arb_texture_compression))
#else // NEW_CURSOR
	if (have_extension(arb_texture_compression)&&compression_enabled)
#endif // NEW_CURSOR
	{
		if (have_extension(ext_texture_compression_s3tc))
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,TEXTURE_SIZE_X, TEXTURE_SIZE_Y,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,TEXTURE_SIZE_X, TEXTURE_SIZE_Y,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		}
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,TEXTURE_SIZE_X, TEXTURE_SIZE_Y,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();
	free(texture_mem);
	return texture;
}
#endif	//ELC
#endif  //MAP_EDITOR2

#ifdef MAP_EDITOR2
/////////////////////////////////////////////////////////////////////////////////////
//load a bmp file, convert it to the rgba format, but don't assign it to any texture object
//// TODO: remove this and use the generic texure routines above
char * load_bmp8_color_key_no_texture_img(char * filename, img_struct * img)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	Uint8 *texture_mem;
	gzFile *f = NULL;
	if(!gzfile_exists(filename))	return 0;	// no file at all
	f= my_gzopen(filename, "rb");
  	if (!f)
		return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	gzread(f, file_mem, 50);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
	{
		free(file_mem_start);
		fclose (f);
		gzclose(f);
		return NULL;
	}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
	{
		free(file_mem_start);
		gzclose(f);
		return NULL;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
	{
		free(file_mem_start);
		gzclose(f);
		return NULL;
	}
	file_mem+=16;

	colors_no=*((int *)file_mem);
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
	gzread(f, file_mem, colors_no*4+4);//header only
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));

	for(y=0;y<y_size;y++)
	{
		gzread(f, read_buffer, x_size-x_padding);

		for(x=0;x<x_size;x++)
		{
			current_pallete_entry=*(read_buffer+x);
			b=*(color_pallete+current_pallete_entry*4);
			g=*(color_pallete+current_pallete_entry*4+1);
			r=*(color_pallete+current_pallete_entry*4+2);
			*(texture_mem+(y*x_size+x)*4)=r;
			*(texture_mem+(y*x_size+x)*4+1)=g;
			*(texture_mem+(y*x_size+x)*4+2)=b;
			a=(r+b+g)/3;
			*(texture_mem+(y*x_size+x)*4+3)=a;
		}

	}
	
	if(img)
	{
		img->x=x_size;
		img->y=y_size;
	}
	
	free(file_mem_start);
	free(read_buffer);
	gzclose(f);
	return texture_mem;
}
#endif
#endif	// NEW_TEXTURES
