#include <stdlib.h>
#include <string.h>
#include "textures.h"
#include "asc.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "gl_init.h"
#include "load_gl_extensions.h"
#include "map.h"
#include "image.h"
#include "image_loading.h"
#include "queue.h"
#include "threads.h"
#include "el_memory.h"
#include <assert.h>
#include "hash.h"

#define TEXTURE_SIZE_X 512
#define TEXTURE_SIZE_Y 512
#define TEXTURE_RATIO 2

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
	GLenum src_format=0, type=0, internal_format;
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
	el_file_ptr eyes_tex;
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
	eyes_tex = 0;
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
	if (files->eyes_tex[0])
	{
		open_for_coordinates(files->eyes_tex, &eyes_tex,
			6, 6, &sizes, &format);
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
	if (eyes_tex != 0)
	{
		alpha += load_to_coordinates(eyes_tex, 50, 32, 6, 6,
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
	Uint32 i, handle, hash, access_time = 0;

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
	copy_enhanced_actor_file_name(files.eyes_tex, actor->eyes_tex);
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

	af = 0;

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
	copy_enhanced_actor_file_name(files.eyes_tex, actor->eyes_tex);
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
			load_enhanced_actor_thread, "TextureThread", &actor_texture_threads_done);
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
