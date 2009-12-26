/****************************************************************************
 *            ddsimage.c
 *
 * Author: 2009  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "dds.h"
#include "errors.h"
#include "io/elfilewrapper.h"

static Uint32 popcount(const Uint32 x)
{
	Uint32 r;

	r = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);

	return ((r + (r >> 3)) & 030707070707) % 63;
}

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
		LOG_ERROR("File '%s' is invalid. Size of pixe format header is %d bytes, but must"
			" be %3% bytes for valid DDS files.", file_name,
			header->m_pixel_format.m_size, DDS_PIXEL_FORMAT_SIZE);
		return 0;
	}

	if ((header->m_flags & DDSD_MIN_FLAGS) != DDSD_MIN_FLAGS)
	{
		LOG_ERROR("File '%s' is invalid. At least the "
			"DDSD_CAPS, DDSD_PIXELFORMAT, DDSD_WIDTH and DDSD_HEIGHT flags"
			" must be set for a valid DDS file.", file_name);
		return 0;
	}

	if ((header->m_caps.m_caps1 & DDSCAPS_TEXTURE) != DDSCAPS_TEXTURE)
	{
		LOG_ERROR("File '%' is invalid. At least "
			"DDSCAPS_TEXTURE cap must be set for a valid DDS file.",
			file_name);
		return 0;
	}

	if (((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP) == DDSCAPS2_CUBEMAP) &&
		((header->m_caps.m_caps2 & DDSCAPS2_CUBEMAP_ALL_FACES) == 0))
	{
		LOG_ERROR("File '%s' is invalid. At least one cube"
			" map face must be set for a valid cube map DDS file.", file_name);
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
		LOG_ERROR("File '%s' is invalid. Only volmue "
			"textures can have a detph value in a valid DDS file.", file_name);
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
		LOG_ERROR("File '%s' is invalid. DDSCAPS_COMPLEX cap "
			"should be set only if the DDS file is a cube map, a volume and/or"
			" has mipmaps.", file_name);
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) == DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB))
	{
		LOG_ERROR("File '%s' is invalid. A valid DDS file "
			"must set either DDPF_FORCC or DDPF_RGB as pixel format flags.",
			file_name);
		return 0;
	}

	if ((header->m_pixel_format.m_flags & DDPF_LUMINANCE) == DDPF_LUMINANCE)
	{
		LOG_ERROR("File '%s' is invalid. A valid DDS file must "
			"not set DDPF_LUMINANCE as pixel format flags.", file_name);
	}

	if (((header->m_pixel_format.m_flags & DDPF_FOURCC) != DDPF_FOURCC) &&
		((header->m_pixel_format.m_flags & DDPF_RGB) != DDPF_RGB))
	{
		if ((header->m_pixel_format.m_flags & DDPF_LUMINANCE) == DDPF_LUMINANCE)
		{
			header->m_pixel_format.m_green_mask = 0;
			header->m_pixel_format.m_blue_mask = 0;
		}
		else
		{
			LOG_ERROR("File '%s' is invalid. A valid "
				"DDS file must set either DDPF_FORCC or DDPF_RGB as pixe"
				" format flags.", file_name);
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
			LOG_ERROR("File '%s' is invalid. Volmue "
				"textures must have a depth greather than zero.", file_name);
			return 0;
		}
	}

	if ((header->m_pixel_format.m_flags & DDPF_ALPHAPIXELS) != DDPF_ALPHAPIXELS)
	{
		if (header->m_pixel_format.m_alpha_mask != 0)
		{
			LOG_ERROR("File '%s' is invalid. Non alpha pixe"
				" formats must have a zero alpha mask to be a valid DDS "
				"files", file_name);
			header->m_pixel_format.m_alpha_mask = 0;
		}
	}

	bit_count = popcount(header->m_pixel_format.m_red_mask |
		header->m_pixel_format.m_blue_mask |
		header->m_pixel_format.m_green_mask |
		header->m_pixel_format.m_alpha_mask);

	if (((header->m_pixel_format.m_flags & DDPF_RGB) == DDPF_RGB) &&
		(header->m_pixel_format.m_bit_count != bit_count))
	{
		header->m_pixel_format.m_alpha_mask = 0xFFFFFFFF;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_red_mask;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_blue_mask;
		header->m_pixel_format.m_alpha_mask ^= header->m_pixel_format.m_green_mask;
	}

	return 1;
}

static Uint32 init_dds_image(el_file_ptr file, DdsHeader *header, const char* file_name)
{
	Uint8 magic[4];

	el_read(file, sizeof(magic), magic);

	if (!check_dds(magic))
	{
		LOG_ERROR("File '%s' is invalid. Wrong magic number for a valid DDS.", file_name);
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

	return validate_header(header, file_name);
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

static void read_and_uncompress_dxt1_block(el_file_ptr file, Uint8 *values)
{
	DXTColorBlock colors;

	read_colors_block(file, &colors);

	unpack_dxt1(&colors, values);
}

static void read_and_uncompress_dxt3_block(el_file_ptr file, Uint8 *values)
{
	DXTExplicitAlphaBlock alphas;
	DXTColorBlock colors;

	read_explicit_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt3(&alphas, &colors, values);
}

static void read_and_uncompress_dxt5_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;
	DXTColorBlock colors;

	read_interpolated_alphas_block(file, &alphas);
	read_colors_block(file, &colors);

	unpack_dxt5(&alphas, &colors, values);
}

static void read_and_uncompress_ati1_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock alphas;

	read_interpolated_alphas_block(file, &alphas);

	unpack_ati1(&alphas, values);
}

static void read_and_uncompress_ati2_block(el_file_ptr file, Uint8 *values)
{
	DXTInterpolatedAlphaBlock first_block;
	DXTInterpolatedAlphaBlock second_block;

	read_interpolated_alphas_block(file, &first_block);
	read_interpolated_alphas_block(file, &second_block);

	unpack_ati2(&first_block, &second_block, values);
}

static void uncompress_block(el_file_ptr file, Uint32 format, Uint32 x, Uint32 width,
	Uint32 dst_pitch, Uint32 dst_pitch_minus_4, Uint32 *idx, Uint8 *dst)
{
	Uint8 values[64];
	Uint32 i, j, index;

	switch (format)
	{
		case DDSFMT_DXT1:
			read_and_uncompress_dxt1_block(file, values);
			break;
		case DDSFMT_DXT2:
		case DDSFMT_DXT3:
			read_and_uncompress_dxt3_block(file, values);
			break;
		case DDSFMT_DXT4:
		case DDSFMT_DXT5:
			read_and_uncompress_dxt5_block(file, values);
			break;
		case DDSFMT_ATI1:
			read_and_uncompress_ati1_block(file, values);
			break;
		case DDSFMT_ATI2:
			read_and_uncompress_ati2_block(file, values);
			break;
	}

	index = *idx;

	// write 4x4 block to uncompressed version
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			dst[index + 0] = values[(i * 4 + j) * 4 + 0];
			dst[index + 1] = values[(i * 4 + j) * 4 + 1];
			dst[index + 2] = values[(i * 4 + j) * 4 + 2];
			dst[index + 3] = values[(i * 4 + j) * 4 + 3];
			index += 4;
		}
		// advance to next row
		index += dst_pitch_minus_4;
	}
	// next block. Our dest pointer is 4 lines down
	// from where it started
	if ((x + 1) == (width / 4))
	{
		// Jump back to the start of the line
		index -= dst_pitch_minus_4;
	}
	else
	{
		// Jump back up 4 rows and 4 pixels to the
		// right to be at the next block to the right
		index += (4 - dst_pitch) * 4;
	}

	*idx = index;
}

static void* uncompress(el_file_ptr file, DdsHeader *header, const char* file_name)
{
	Uint32 width, height, size, format;
	Uint32 x, y;
	Uint32 dst_pitch, dst_pitch_minus_4, index;
	Uint8 *dst;

	if ((header->m_height % 4) != 0)
	{
		LOG_ERROR("Can`t uncompressed DDS file %s because height is %d and not a "
			"multible of four.", file_name, header->m_height);
		return 0;
	}

	if ((header->m_width % 4) != 0)
	{
		LOG_ERROR("Can`t uncompressed DDS file %s because width is %d and not a "
			"multible of four.", file_name, header->m_width);
		return 0;
	}

	format = header->m_pixel_format.m_fourcc;

	if ((format != DDSFMT_DXT1) && (format != DDSFMT_DXT2) && (format != DDSFMT_DXT3) &&
		(format != DDSFMT_DXT4) && (format != DDSFMT_DXT5) && (format != DDSFMT_ATI1) &&
		(format != DDSFMT_ATI2))
	{
		return 0;
	}

	index = 0;

	width = header->m_width;
	height = header->m_height;

	size = width * height * 4;

	dst = malloc(size * sizeof(GLubyte));

	dst_pitch = width * 4;
	dst_pitch_minus_4 = dst_pitch - 4 * 4;

	// 4x4 blocks in x/y
	for (y = 0; y < (height / 4); y++)
	{
		for (x = 0; x < (width / 4); x++)
		{
			uncompress_block(file, format, x, width, dst_pitch, dst_pitch_minus_4,
				&index, dst);
		}
	}

	return dst;
}

void* load_dds(el_file_ptr file, const char* file_name, int *width, int *height)
{
	DdsHeader header;

	if (file == 0)
	{
		return 0;
	}

	if (init_dds_image(file, &header, file_name) != 0)
	{
		*width = header.m_width;
		*height = header.m_height;

		return uncompress(file, &header, file_name);
	}
	else
	{
		return 0;
	}
}
