/****************************************************************************
 *            ddsimage.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "ddsimage.h"
#include "dds.h"
#include "errors.h"
#include "image.h"
#include "misc.h"
#include "memory.h"
#include <assert.h>

#ifdef	NEW_TEXTURES
static Uint32 decompression_needed(const DdsHeader *header,
	const Uint32 compression, const Uint32 unpack)
{
	if ((header->m_pixel_format.m_flags & DDPF_FOURCC) == DDPF_FOURCC)
	{
		switch (header->m_pixel_format.m_fourcc)
		{
			case DDSFMT_DXT1:
			case DDSFMT_DXT3:
			case DDSFMT_DXT5:
				if ((compression & tct_s3tc) == tct_s3tc)
				{
					if (unpack != 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				return 1;
			case DDSFMT_ATI1:
				if ((compression & tct_latc) == tct_latc)
				{
					if (unpack != 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				return 1;
			case DDSFMT_ATI2:
				if ((compression & tct_latc) == tct_latc)
				{
					if (unpack != 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				if ((compression & tct_3dc) == tct_3dc)
				{
					if (unpack != 0)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				return 1;
		}
	}

	return 0;
}

static image_format_type detect_dds_file_format(const DdsHeader *header,
	Uint8* alpha, Uint32* unpack)
{
	Uint32 red_mask, green_mask, blue_mask, alpha_mask, luminace_mask;

	*unpack = 0;

	if ((header->m_pixel_format.m_flags & DDPF_FOURCC) == DDPF_FOURCC)
	{
		switch (header->m_pixel_format.m_fourcc)
		{
			case DDSFMT_DXT1:
				*alpha = 0;
				return ift_dxt1;
			case DDSFMT_DXT3:
				*alpha = 1;
				return ift_dxt3;
			case DDSFMT_DXT5:
				*alpha = 1;
				return ift_dxt5;
			case DDSFMT_ATI1:
				*alpha = 0;
				return ift_ati1;
			case DDSFMT_ATI2:
				*alpha = 1;
				return ift_ati2;
		}
	}
	else
	{
		red_mask = header->m_pixel_format.m_red_mask;
		green_mask = header->m_pixel_format.m_green_mask;
		blue_mask = header->m_pixel_format.m_blue_mask;
		alpha_mask = header->m_pixel_format.m_alpha_mask;
		luminace_mask = red_mask | green_mask | blue_mask;

		if (alpha_mask != 0)
		{
			*alpha = 1;
		}
		else
		{
			*alpha = 0;
		}

		if (((luminace_mask == red_mask) || (red_mask == 0xFF)) &&
			((luminace_mask == green_mask) || (green_mask == 0xFF)) &&
			((luminace_mask == blue_mask) || (blue_mask == 0xFF)))
		{
			if ((luminace_mask == 0x00) && (alpha_mask == 0xFF))
			{
				return ift_a8;
			}

			if ((luminace_mask == 0xFF) && (alpha_mask == 0x00))
			{
				return ift_l8;
			}

			if ((luminace_mask == 0xFF) && (alpha_mask == 0xFF00))
			{
				return ift_la8;
			}
		}

		if ((red_mask == 0x0F00) && (green_mask == 0x00F0) &&
			(blue_mask == 0x000F) && (alpha_mask == 0xF000))
		{
			return ift_rgba4;
		}

		if ((red_mask == 0xF800) && (green_mask == 0x07E0) &&
			(blue_mask == 0x001F) && (alpha_mask == 0x0000))
		{
			return ift_r5g6b5;
		}

		if ((red_mask == 0x7C00) && (green_mask == 0x03E0) &&
			(blue_mask == 0x001F) && (alpha_mask == 0x8000))
		{
			return ift_rgb5_a1;
		}

		if ((red_mask == 0x000000FF) && (green_mask == 0x0000FF00) &&
			(blue_mask == 0x00FF0000) && (alpha_mask == 0x00000000))
		{
			return ift_rgb8;
		}

		if ((red_mask == 0x000000FF) && (green_mask == 0x0000FF00) &&
			(blue_mask == 0x00FF0000) && (alpha_mask == 0xFF000000))
		{
			return ift_rgba8;
		}

		if ((red_mask == 0x00FF0000) && (green_mask == 0x0000FF00) &&
			(blue_mask == 0x000000FF) && (alpha_mask == 0xFF000000))
		{
			return ift_bgra8;
		}

		if ((red_mask == 0x00FF0000) && (green_mask == 0x0000FF00) &&
			(blue_mask == 0x000000FF) && (alpha_mask == 0x00000000))
		{
			return ift_bgr8;
		}
	}

	*unpack = 1;

	return ift_rgba8;
}
#endif	/* NEW_TEXTURES */

Uint32 check_dds(const Uint8 *ID)
{
	return (ID[0] == 'D') && (ID[1] == 'D') && (ID[2] == 'S') && (ID[3] == ' ');
}

static Uint32 validate_header(DdsHeader *header, const char* file_name)
{
	Uint32 bit_count;

	if (header->m_size != DDS_HEADER_SIZE)
	{
		LOG_ERROR("File '%s' is invalid. Size of header is"
			" %d bytes, but must be %d bytes for valid DDS files.",
			file_name, header->m_size, DDS_HEADER_SIZE);
		return 0;
	}

	if (header->m_pixel_format.m_size != DDS_PIXEL_FORMAT_SIZE)
	{
		LOG_ERROR("File '%s' is invalid. Size of pixe format header is"
			" %d bytes, but must be %3% bytes for valid DDS files.",
			file_name, header->m_pixel_format.m_size,
			DDS_PIXEL_FORMAT_SIZE);
		return 0;
	}

	if ((header->m_flags & DDSD_MIN_FLAGS) != DDSD_MIN_FLAGS)
	{
		LOG_ERROR("File '%s' is invalid. At least the "
			"DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH and "
			"DDSD_HEIGHT flags must be set for a valid DDS file.",
			file_name);
		return 0;
	}

	if ((header->m_caps.m_caps1 & DDSCAPS_TEXTURE) != DDSCAPS_TEXTURE)
	{
		LOG_ERROR("File '%' is invalid. At least DDSCAPS_TEXTURE cap "
			"must be set for a valid DDS file.", file_name);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == 0))
	{
		LOG_ERROR("File '%s' is invalid. At least one cube"
			" map face must be set for a valid cube map DDS file.",
			file_name);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) == DDSCAPS2_VOLUME))
	{
		LOG_ERROR("File '%s' is invalid. A valid DDS file "
			"can only be a cube map or a volume, not both.", file_name);
		return 0;
	}

	if (((header->m_flags & DDSD_DEPTH) == DDSD_DEPTH) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) != DDSCAPS2_VOLUME))
	{
		LOG_ERROR("File '%s' is invalid. Only volmue images can have"
			" a detph value in a valid DDS file.", file_name);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		LOG_ERROR("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid cube map DDS file.", file_name);
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) == DDSCAPS2_VOLUME) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		LOG_ERROR("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid volume DDS file.", file_name);
	}

	if (((header->m_caps.m_caps1 & DDSCAPS_MIPMAP) == DDSCAPS_MIPMAP) &&
		((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) != DDSCAPS_COMPLEX))
	{
		LOG_ERROR("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set for a valid DDS file with mipmaps.", file_name);
	}

	if (((header->m_caps.m_caps1 & DDSCAPS_COMPLEX) == DDSCAPS_COMPLEX) &&
		((header->m_caps.m_caps1 & DDSCAPS_MIPMAP) != DDSCAPS_MIPMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_VOLUME) != DDSCAPS2_VOLUME)
		&& ((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) != DDSCAPS2_CUBEMAP))
	{
		LOG_ERROR("File '%s' is invalid. DDSCAPS_COMPLEX cap should "
			"be set only if the DDS file is a cube map, a volume "
			"and/or has mipmaps.", file_name);
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) == DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB))
	{
		LOG_ERROR("File '%s' is invalid. A valid DDS file must set "
			"either DDPF_FORCC or DDPF_RGB as pixel format flags.",
			file_name);
		return 0;
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) != DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) != DDPF_RGB))
	{
		if ((header->m_pixel_format.m_flags & DDPF_LUMINANCE) == DDPF_LUMINANCE)
		{
			header->m_pixel_format.m_green_mask = header->m_pixel_format.m_red_mask;
			header->m_pixel_format.m_blue_mask = header->m_pixel_format.m_red_mask;
		}
		else
		{
			if ((header->m_pixel_format.m_flags & DDPF_ALPHA) != DDPF_ALPHA)
			{
				LOG_ERROR("File '%s' is invalid. A valid DDS file must"
					" set either DDPF_FORCC or DDPF_RGB as pixe"
					" format flags.", file_name);
				return 0;
			}
		}
	}

	if ((header->m_caps.m_caps1 & DDSCAPS_MIPMAP) != DDSCAPS_MIPMAP)
	{
		header->m_mipmap_count = 1;
	}
	else
	{
		if (header->m_mipmap_count == 0)
		{
			LOG_ERROR("File '%s' is invalid. Mipmaped images must"
				" have a mipmap count greather than zero.",
				file_name);
			return 0;
		}
	}

	if ((header->m_flags & DDSD_DEPTH) != DDSD_DEPTH)
	{
		header->m_depth = 1;
	}
	else
	{
		if (header->m_depth == 0)
		{
			LOG_ERROR("File '%s' is invalid. Volmue images must "
				"have a depth greather than zero.", file_name);
			return 0;
		}
	}

	bit_count = (popcount(header->m_pixel_format.m_red_mask |
		header->m_pixel_format.m_blue_mask |
		header->m_pixel_format.m_green_mask |
		header->m_pixel_format.m_alpha_mask) + 7) & 0xF8;

	if ((header->m_pixel_format.m_bit_count != bit_count) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB))
	{
		header->m_pixel_format.m_bit_count = bit_count;
	}

	return 1;
}

static Uint32 init_dds_image(el_file_ptr file, DdsHeader *header)
{
	Uint8 magic[4];

	el_read(file, sizeof(magic), magic);

	if (!check_dds(magic))
	{
		LOG_ERROR("File '%s' is invalid. Wrong magic number for a "
			"valid DDS.", el_file_name(file));
		return 0;
	}

	el_read(file, sizeof(DdsHeader), header);

	header->m_size = SDL_SwapLE32(header->m_size);
	header->m_flags = SDL_SwapLE32(header->m_flags);
	header->m_height = SDL_SwapLE32(header->m_height);
	header->m_width = SDL_SwapLE32(header->m_width);
	header->m_size_or_pitch = SDL_SwapLE32(header->m_size_or_pitch);
	header->m_depth = SDL_SwapLE32(header->m_depth);
	header->m_mipmap_count = SDL_SwapLE32(header->m_mipmap_count);

	header->m_reserved1[0] = SDL_SwapLE32(header->m_reserved1[0]);
	header->m_reserved1[1] = SDL_SwapLE32(header->m_reserved1[1]);
	header->m_reserved1[2] = SDL_SwapLE32(header->m_reserved1[2]);
	header->m_reserved1[3] = SDL_SwapLE32(header->m_reserved1[3]);
	header->m_reserved1[4] = SDL_SwapLE32(header->m_reserved1[4]);
	header->m_reserved1[5] = SDL_SwapLE32(header->m_reserved1[5]);
	header->m_reserved1[6] = SDL_SwapLE32(header->m_reserved1[6]);
	header->m_reserved1[7] = SDL_SwapLE32(header->m_reserved1[7]);
	header->m_reserved1[8] = SDL_SwapLE32(header->m_reserved1[8]);
	header->m_reserved1[9] = SDL_SwapLE32(header->m_reserved1[9]);
	header->m_reserved1[10] = SDL_SwapLE32(header->m_reserved1[10]);

	header->m_pixel_format.m_size = SDL_SwapLE32(header->m_pixel_format.m_size);
	header->m_pixel_format.m_flags = SDL_SwapLE32(header->m_pixel_format.m_flags);
	header->m_pixel_format.m_fourcc = SDL_SwapLE32(header->m_pixel_format.m_fourcc);
	header->m_pixel_format.m_bit_count = SDL_SwapLE32(header->m_pixel_format.m_bit_count);
	header->m_pixel_format.m_red_mask = SDL_SwapLE32(header->m_pixel_format.m_red_mask);
	header->m_pixel_format.m_green_mask = SDL_SwapLE32(header->m_pixel_format.m_green_mask);
	header->m_pixel_format.m_blue_mask = SDL_SwapLE32(header->m_pixel_format.m_blue_mask);
	header->m_pixel_format.m_alpha_mask = SDL_SwapLE32(header->m_pixel_format.m_alpha_mask);

	header->m_caps.m_caps1 = SDL_SwapLE32(header->m_caps.m_caps1);
	header->m_caps.m_caps2 = SDL_SwapLE32(header->m_caps.m_caps2);
	header->m_caps.m_caps3 = SDL_SwapLE32(header->m_caps.m_caps3);
	header->m_caps.m_caps4 = SDL_SwapLE32(header->m_caps.m_caps4);

	header->m_reserved2 = SDL_SwapLE32(header->m_reserved2);

	return validate_header(header, el_file_name(file));
}

static void read_colors_block(el_file_ptr file, DXTColorBlock *colors)
{
	Uint32 i;

	el_read(file, sizeof(DXTColorBlock), colors);

	for (i = 0; i < 2; i++)
	{
		colors->m_colors[i] = SDL_SwapLE16(colors->m_colors[i]);
	}
}

static void read_interpolated_alphas_block(el_file_ptr file, DXTInterpolatedAlphaBlock *alphas)
{
	el_read(file, sizeof(DXTInterpolatedAlphaBlock), alphas);
}

static void read_explicit_alphas_block(el_file_ptr file, DXTExplicitAlphaBlock *alphas)
{
	Uint32 i;

	el_read(file, sizeof(DXTExplicitAlphaBlock), alphas);

	for (i = 0; i < 4; i++)
	{
		alphas->m_alphas[i] = SDL_SwapLE16(alphas->m_alphas[i]);
	}
}

static void read_and_decompress_dxt1_block(el_file_ptr file, Uint8 *values)
{
	DXTColorBlock colors;

	read_colors_block(file, &colors);

	unpack_dxt1(&colors, values);
}

static void read_and_decompress_dxt3_block(el_file_ptr file, Uint8 *values)
{
	DXTExplicitAlphaBlock alphas;
	DXTColorBlock colors;

	read_explicit_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt3(&alphas, &colors, values);
}

static void read_and_decompress_dxt5_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;
	DXTColorBlock colors;

	read_interpolated_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt5(&alphas, &colors, values);
}

static void read_and_decompress_ati1_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;

	read_interpolated_alphas_block(file, &alphas);

	unpack_ati1(&alphas, values);
}

static void read_and_decompress_ati2_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock first_block;
	DXTInterpolatedAlphaBlock second_block;

	read_interpolated_alphas_block(file, &first_block);
	read_interpolated_alphas_block(file, &second_block);

	unpack_ati2(&first_block, &second_block, values);
}

static void decompress_block(el_file_ptr file, const Uint32 format,
	const Uint32 x, const Uint32 y, const Uint32 width, const Uint32 height,
	const Uint32 offset, Uint8 *dst)
{
	Uint8 values[64];
	Uint32 i, j, index, count_x, count_y;

	switch (format)
	{
		case DDSFMT_DXT1:
			read_and_decompress_dxt1_block(file, values);
			break;
		case DDSFMT_DXT2:
		case DDSFMT_DXT3:
			read_and_decompress_dxt3_block(file, values);
			break;
		case DDSFMT_DXT4:
		case DDSFMT_DXT5:
			read_and_decompress_dxt5_block(file, values);
			break;
		case DDSFMT_ATI1:
			read_and_decompress_ati1_block(file, values);
			break;
		case DDSFMT_ATI2:
			read_and_decompress_ati2_block(file, values);
			break;
	}

	count_x = min2u(width - x, 4);
	count_y = min2u(height - y, 4);

	// write 4x4 block to decompressed version
	for (i = 0; i < count_y; i++)
	{
		for (j = 0; j < count_x; j++)
		{
			index = offset + (y + i) * width + x + j;
			dst[index * 4 + 0] = values[(i * 4 + j) * 4 + 0];
			dst[index * 4 + 1] = values[(i * 4 + j) * 4 + 1];
			dst[index * 4 + 2] = values[(i * 4 + j) * 4 + 2];
			dst[index * 4 + 3] = values[(i * 4 + j) * 4 + 3];
		}
	}
}

static Uint32 get_level_size(const Uint32 format, const Uint32 bpp,
	const Uint32 width, const Uint32 height, const Uint32 level,
	const Uint32 decompress)
{
	Uint32 w, h;

	w = max2u(width >> level, 1);
	h = max2u(height >> level, 1);

	if ((format == DDSFMT_DXT1) || (format == DDSFMT_DXT2) ||
		(format == DDSFMT_DXT3) || (format == DDSFMT_DXT4) ||
		(format == DDSFMT_DXT5) || (format == DDSFMT_ATI2) ||
		(format == DDSFMT_ATI1))
	{
		if (decompress != 0)
		{
			return w * h * 4;
		}
		else
		{
			w = (w + 3) / 4;
			h = (h + 3) / 4;

			if ((format == DDSFMT_DXT1) || (format == DDSFMT_ATI1))
			{
				return w * h * 8;
			}
			else
			{
				return w * h * 16;
			}
		}
	}
	else
	{
		return w * h * bpp;
	}
}

static Uint32 get_dds_level_size(const DdsHeader *header, const Uint32 level,
	const Uint32 decompress, const Uint32 unpack)
{
	Uint32 width, height, format, bpp;

	width = header->m_width;
	height = header->m_height;
	format = header->m_pixel_format.m_fourcc;

	if (unpack != 0)
	{
		bpp = 4;
	}
	else
	{
		bpp = header->m_pixel_format.m_bit_count / 8;
	}

	return get_level_size(format, bpp, width, height, level, decompress);
}

static Uint32 get_dds_size(const DdsHeader *header, const Uint32 decompress,
	const Uint32 strip_mipmaps, const Uint32 base_level)
{
	Uint32 result, mipmap_count, i;

	if (strip_mipmaps != 0)
	{
		return get_dds_level_size(header, base_level, decompress, 0);
	}
	else
	{
		mipmap_count = header->m_mipmap_count;

		result = 0;

		for (i = base_level; i < mipmap_count; i++)
		{
			result += get_dds_level_size(header, i, decompress, 0);
		}

		return result;
	}
}

static Uint32 get_dds_offset(const DdsHeader *header, const Uint32 base_level)
{
	Uint32 result, i;

	result = 0;

	for (i = 0; i < base_level; i++)
	{
		result += get_dds_level_size(header, i, 0, 0);
	}

	return result;
}

static void* decompress_dds(el_file_ptr file, DdsHeader *header,
	const Uint32 strip_mipmaps, const Uint32 base_level)
{
	Uint32 width, height, size, format, mipmap_count;
	Uint32 x, y, i, w, h;
	Uint32 index;
	Uint8 *dest;

	if ((header->m_height % 4) != 0)
	{
		LOG_ERROR("Can`t decompressed DDS file %s because height is"
			" %d and not a multiple of four.", el_file_name(file),
			header->m_height);
		return 0;
	}

	if ((header->m_width % 4) != 0)
	{
		LOG_ERROR("Can`t decompressed DDS file %s because width is"
			" %d and not a multiple of four.", el_file_name(file),
			header->m_width);
		return 0;
	}

	format = header->m_pixel_format.m_fourcc;

	if ((format != DDSFMT_DXT1) && (format != DDSFMT_DXT2) &&
		(format != DDSFMT_DXT3) && (format != DDSFMT_DXT4) &&
		(format != DDSFMT_DXT5) && (format != DDSFMT_ATI1) &&
		(format != DDSFMT_ATI2))
	{
		return 0;
	}

	index = 0;

	size = get_dds_size(header, 1, strip_mipmaps, base_level);
	width = max2u(header->m_width >> base_level, 1);
	height = max2u(header->m_height >> base_level, 1);
	mipmap_count = header->m_mipmap_count;

	if (strip_mipmaps != 0)
	{
		if (mipmap_count > (base_level + 1))
		{
			mipmap_count = base_level + 1;
		}
	}

#ifdef	NEW_TEXTURES
	dest = malloc_aligned(size, 16);
#else	/* NEW_TEXTURES */
	dest = malloc(size);
#endif	/* NEW_TEXTURES */

	el_seek(file, get_dds_offset(header, base_level), SEEK_CUR);

	for (i = base_level; i < mipmap_count; i++)
	{
		w = (width + 3) / 4;
		h = (height + 3) / 4;

		assert(index * 4 <= size);

		// 4x4 blocks in x/y
		for (y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++)
			{
				decompress_block(file, format, x * 4, y * 4,
					width, height, index, dest);
			}
		}

		index += width * height;

		if (width > 1)
		{
			width /= 2;
		}

		if (height > 1)
		{
			height /= 2;
		}
	}

	assert(index * 4 == size);

	return dest;
}

static void* unpack_dds(el_file_ptr file, DdsHeader *header,
	const Uint32 strip_mipmaps, const Uint32 base_level)
{
	Uint8* dest;
	Uint32 size, offset, bpp;

	size = get_dds_size(header, 0, strip_mipmaps, base_level);
	offset = get_dds_offset(header, base_level);
	bpp = header->m_pixel_format.m_bit_count / 8;

#ifdef	NEW_TEXTURES
	dest = malloc_aligned(size, 16);
#else	/* NEW_TEXTURES */
	dest = malloc(size);
#endif	/* NEW_TEXTURES */

	fast_unpack(el_get_pointer(file) + sizeof(DdsHeader) + offset + 4, size / bpp,
		header->m_pixel_format.m_red_mask,
		header->m_pixel_format.m_green_mask,
		header->m_pixel_format.m_blue_mask,
		header->m_pixel_format.m_alpha_mask, dest);

	return dest;
}

#ifdef	NEW_TEXTURES
static void* read_dds(el_file_ptr file, DdsHeader *header,
	const Uint32 strip_mipmaps, const Uint32 base_level)
{
	Uint8* dst;
	Uint32 size, offset;

	size = get_dds_size(header, 0, strip_mipmaps, base_level);
	offset = get_dds_offset(header, base_level);

	dst = malloc_aligned(size, 16);

	memcpy(dst, el_get_pointer(file) + sizeof(DdsHeader) + offset + 4, size);

	return dst;
}

static void get_dds_sizes_and_offsets(const DdsHeader *header,
	const Uint32 decompress, const Uint32 unpack,
	const Uint32 strip_mipmaps, const Uint32 base_level,
	image_t* image)
{
	Uint32 offset, size, index, mipmap_count, i;

	memset(image->sizes, 0, sizeof(image->sizes));
	memset(image->offsets, 0, sizeof(image->offsets));

	if (strip_mipmaps != 0)
	{
		size = get_dds_level_size(header, base_level, decompress,
			unpack);

		image->sizes[0] = size;
	}
	else
	{
		mipmap_count = header->m_mipmap_count;

		index = 0;
		offset = 0;

		for (i = base_level; i < mipmap_count; i++)
		{
			size = get_dds_level_size(header, i, decompress,
				unpack);

			image->sizes[index] = size;
			image->offsets[index] = offset;

			index++;
			offset += size;
		}
	}
}

Uint32 load_dds(el_file_ptr file, const Uint32 compression,
	const Uint32 unpack, const Uint32 strip_mipmaps, Uint32 base_level,
	image_t* image)
{
	DdsHeader header;
	Uint32 format_unpack, mipmap_count, start_mipmap;

	if (file == 0)
	{
		return 0;
	}

	if (init_dds_image(file, &header) != 0)
	{
		start_mipmap = min2u(base_level, header.m_mipmap_count - 1);

		if (strip_mipmaps != 0)
		{
			mipmap_count = 1;
		}
		else
		{
			mipmap_count = header.m_mipmap_count - start_mipmap;
		}

		assert(mipmap_count > 0);
		assert(start_mipmap < header.m_mipmap_count);
		assert(mipmap_count <= header.m_mipmap_count);

		image->width = max2u(header.m_width >> start_mipmap, 1);
		image->height = max2u(header.m_height >> start_mipmap, 1);
		image->mipmaps = mipmap_count;
		image->format = detect_dds_file_format(&header, &image->alpha,
			&format_unpack);

		if (decompression_needed(&header, compression, unpack) == 1)
		{
			image->image = decompress_dds(file, &header,
				strip_mipmaps, start_mipmap);
			image->format = ift_rgba8;

			get_dds_sizes_and_offsets(&header, 1, 1,
				strip_mipmaps, start_mipmap, image);
		}
		else
		{
			if ((unpack != 0) || (format_unpack != 0))
			{
				image->image = unpack_dds(file, &header,
					strip_mipmaps, start_mipmap);
				image->format = ift_rgba8;

				get_dds_sizes_and_offsets(&header, 0, 1,
					strip_mipmaps, start_mipmap, image);
			}
			else
			{
				image->image = read_dds(file, &header,
					strip_mipmaps, start_mipmap);

				get_dds_sizes_and_offsets(&header, 0, 0,
					strip_mipmaps, start_mipmap, image);
			}
		}

		assert(image->width > 0);
		assert(image->width > 0);
		assert(image->sizes[0] > 0);
		assert(image->width > 0);
		assert(image->height > 0);
		assert(image->mipmaps > 0);

		if (image->image != 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

Uint32 get_dds_information(el_file_ptr file, image_t* image)
{
	DdsHeader header;
	Uint32 format_unpack;

	if (file == 0)
	{
		return 0;
	}

	if (init_dds_image(file, &header) != 0)
	{
		memset(image, 0, sizeof(image_t));

		image->width = header.m_width;
		image->height = header.m_height;
		image->mipmaps = header.m_mipmap_count;
		image->format = detect_dds_file_format(&header, &image->alpha,
			&format_unpack);

		return 1;
	}
	else
	{
		return 0;
	}
}
#else	/* NEW_TEXTURES */
void* load_dds(el_file_ptr file, int *width, int *height)
{
	DdsHeader header;
	Uint32 format;

	if (file == 0)
	{
		return 0;
	}

	if (init_dds_image(file, &header) != 0)
	{
		*width = header.m_width;
		*height = header.m_height;

		format = header.m_pixel_format.m_fourcc;

		if ((format == DDSFMT_DXT1) || (format == DDSFMT_DXT2) || (format == DDSFMT_DXT3) ||
			(format == DDSFMT_DXT4) || (format == DDSFMT_DXT5) || (format == DDSFMT_ATI1) ||
			(format == DDSFMT_ATI2))
		{
			return decompress_dds(file, &header, 1, 0);
		}
		else
		{
			return unpack_dds(file, &header, 1, 0);
		}
	}
	else
	{
		return 0;
	}
}
#endif	/* NEW_TEXTURES */
