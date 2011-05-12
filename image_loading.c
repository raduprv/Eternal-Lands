/****************************************************************************
 *            image_loading.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifdef	NEW_TEXTURES

#include "image_loading.h"
#include "errors.h"
#include "asc.h"
#include <SDL.h>
#include <SDL_image.h>
#include "ddsimage.h"
#include "memory.h"

#define IMAGE_EXTENSIONS_MAX 5
static const char* image_extensions[IMAGE_EXTENSIONS_MAX] =
{
	".dds",
	".jpeg",
	".jpg",
	".png",
	".bmp"
};

Uint32 get_file_name_len(const char* file_name)
{
	char* p0;
	char* p1;
	Uint32 len;

	p0 = strrchr(file_name, '.');
	p1 = strrchr(file_name, '/');
	len = strlen(file_name);

	if (p0 > p1)
	{
		len = p0 - file_name;
	}

	return len;
}

Uint32 check_image_name(const char* file_name, const Uint32 size, char* str)
{
	char buffer[128];
	Uint32 len, i;

	len = get_file_name_len(file_name);

	if (str == 0)
	{
		LOG_ERROR("Buffer is zero!");

		return 0;
	}

	if ((len + 5) >= sizeof(buffer))
	{
		LOG_ERROR("Buffer too small! %d bytes needed, but buffer is "
			"only %d bytes big!", len + 5, size);

		return 0;
	}

	for (i = 0; i < IMAGE_EXTENSIONS_MAX; i++)
	{
		safe_strncpy2(buffer, file_name, sizeof(buffer), len);
		safe_strcat(buffer, image_extensions[i], sizeof(buffer));

		if (el_custom_file_exists(buffer) != 0)
		{
			len = strlen(buffer);

			safe_strncpy2(str, buffer, size, len);

			return 1;
		}
	}

	return 0;
}

Uint32 check_alpha_image_name(const char* file_name, const Uint32 size,
	char* str)
{
	char buffer[128];
	Uint32 len, i;

	if (file_name == 0)
	{
		LOG_ERROR("Zero file name!");

		return 0;
	}

	len = get_file_name_len(file_name);

	if (str == 0)
	{
		LOG_ERROR("Buffer is zero!");

		return 0;
	}

	if ((len + 11) >= sizeof(buffer))
	{
		LOG_ERROR("Buffer too small! %d bytes needed, but buffer is "
			"only %d bytes big!", len + 11, size);

		return 0;
	}

	for (i = 0; i < IMAGE_EXTENSIONS_MAX; i++)
	{
		safe_strncpy2(buffer, file_name, sizeof(buffer), len);
		safe_strcat(buffer, "_alpha", sizeof(buffer));
		safe_strcat(buffer, image_extensions[i], sizeof(buffer));

		if (el_file_exists(buffer) != 0)
		{
			len = strlen(buffer);

			safe_strncpy2(str, buffer, size, len);

			return 1;
		}
	}

	return 0;
}

static Uint32 get_sdl_image_information(el_file_ptr file, image_t* image)
{
	SDL_Surface *image_surface;
	SDL_RWops *buffer;

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");
		return 0;
	}

	buffer = SDL_RWFromMem(el_get_pointer(file), el_get_size(file));

	image_surface = IMG_Load_RW(buffer, 1);

	if (image_surface == 0)
	{
		LOG_ERROR("load_image() error: [%s] [%s]", el_file_name(file),
			IMG_GetError());
		return 0;
	}

	SDL_LockSurface(image_surface);

	memset(image, 0, sizeof(image_t));

	image->width = image_surface->w;
	image->height = image_surface->h;
	image->mipmaps = 1;
	image->format = ift_rgba8;

	if ((image_surface->format->BitsPerPixel == 8) &&
		(image_surface->format->palette != 0))
	{
		image->alpha = 0;
	}
	else
	{
		if (image_surface->format->Amask != 0)
		{
			image->alpha = 1;
		}
		else
		{
			image->alpha = 0;
		}
	}

	SDL_UnlockSurface(image_surface);
	SDL_FreeSurface(image_surface);

	return 1;
}

static Uint32 load_image_SDL(el_file_ptr file, image_t* image)
{
	SDL_Surface *image_surface;
	SDL_RWops *buffer;
	Uint8* data;
	int image_width, image_height, idx;
	int pixel, temp, r, g, b, a;
	int bpp, i, j, index, x_padding;

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");

		return 0;
	}

	if (image == 0)
	{
		LOG_ERROR("Invalid image for file '%s'!", el_file_name(file));

		return 0;
	}

	buffer = SDL_RWFromMem(el_get_pointer(file), el_get_size(file));

	image_surface = IMG_Load_RW(buffer, 1);

	if (image_surface == 0)
	{
		LOG_ERROR("load_image() error: [%s] [%s]", el_file_name(file),
			IMG_GetError());
		return 0;
	}

	// at this point, the Surface contains some type of pixel data.
	// SDL requires us to lock the surface before using the pixel data:
	SDL_LockSurface(image_surface);

	image_width = image_surface->w;
	image_height = image_surface->h;

	image->width = image_width;
	image->height = image_height;
	image->mipmaps = 1;
	image->format = ift_rgba8;
	image->sizes[0] = image_width * image_height * 4;

	if ((image_surface->format->BitsPerPixel == 8) &&
		(image_surface->format->palette != 0))
	{
		image->alpha = 0;
	}
	else
	{
		if (image_surface->format->Amask != 0)
		{
			image->alpha = 1;
		}
		else
		{
			image->alpha = 0;
		}
	}

	x_padding = image_width % 4;

	if (x_padding)
	{
		x_padding = 4 - x_padding;
	}
	if (image_width <= x_padding)
	{
		x_padding = 0;
	}

	image->image = (GLubyte*)malloc_aligned(image_width * image_height * 4,
		16);
	data = image->image;

	idx = 0;
	pixel = 0;
	bpp = image_surface->format->BytesPerPixel;

	for (i = 0; i < image_height; i++)
	{
		for (j = 0; j < image_width; j++)
		{
			if ((image_surface->format->BitsPerPixel == 8) &&
				(image_surface->format->palette != 0))
			{
				index = ((Uint8 *)image_surface->pixels)[idx];
				r = image_surface->format->palette->colors[index].r;
				g = image_surface->format->palette->colors[index].g;
				b = image_surface->format->palette->colors[index].b;
				a = 255;
			}
			else
			{
				memcpy(&pixel, &((Uint8 *)image_surface->pixels)[idx], bpp);
				/* Get Red component */
				temp = pixel & image_surface->format->Rmask;  /* Isolate red component */
				temp = temp >> image_surface->format->Rshift; /* Shift it down to 8-bit */
				temp = temp << image_surface->format->Rloss;  /* Expand to a full 8-bit number */
				r = (Uint8)temp;

				/* Get Green component */
				temp = pixel & image_surface->format->Gmask;  /* Isolate green component */
				temp = temp >> image_surface->format->Gshift; /* Shift it down to 8-bit */
				temp = temp << image_surface->format->Gloss;  /* Expand to a full 8-bit number */
				g = (Uint8)temp;

				/* Get Blue component */
				temp = pixel & image_surface->format->Bmask;  /* Isolate blue component */
				temp = temp >> image_surface->format->Bshift; /* Shift it down to 8-bit */
				temp = temp << image_surface->format->Bloss;  /* Expand to a full 8-bit number */
				b = (Uint8)temp;

				/* Get Alpha component */
				temp = pixel & image_surface->format->Amask;  /* Isolate alpha component */
				temp = temp >> image_surface->format->Ashift; /* Shift it down to 8-bit */
				temp = temp << image_surface->format->Aloss;  /* Expand to a full 8-bit number */
				a = (Uint8)temp;

				if (image_surface->format->Amask == 0)
				{
					a = 255;
				}
			}
			idx += bpp;

			index = i * image_width + j;
			data[index * 4 + 0] = r;
			data[index * 4 + 1] = g;
			data[index * 4 + 2] = b;
			data[index * 4 + 3] = a;
		}
		idx += bpp * x_padding;
	}

	SDL_UnlockSurface(image_surface);
	SDL_FreeSurface(image_surface);

	return 1;
}

static Uint32 load_image_SDL_alpha(el_file_ptr file, image_t* image)
{
	SDL_Surface *image_surface;
	GLubyte* data;
	int image_width, image_height, idx;
	int pixel, temp, r, g, b, a;
	int bpp, i, j, index, x_padding;

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");

		return 0;
	}

	if (image == 0)
	{
		LOG_ERROR("Invalid image for file '%s'.", el_file_name(file));

		return 0;
	}

	if (image->format != ift_rgba8)
	{
		LOG_ERROR("Alpha map '%s' can't be used for formats other than"
			" RGBA8.", el_file_name(file));

		return 0;
	}

	image_surface = IMG_Load_RW(SDL_RWFromMem(el_get_pointer(file),
		el_get_size(file)), 1);

	if (image_surface == 0)
	{
		LOG_ERROR("load_image() error: [%s] [%s]", el_file_name(file),
			IMG_GetError());
		return 0;
	}

	// at this point, the Surface contains some type of pixel data.
	// SDL requires us to lock the surface before using the pixel data:
	SDL_LockSurface(image_surface);

	image_width = image_surface->w;
	image_height = image_surface->h;

	if (image->width != image_width)
	{
		LOG_ERROR("Alpha map '%s' had wrong width %i, expected width"
			" is %i.", el_file_name(file), image_width,
			image->width);

		return 0;
	}

	if (image->height != image_height)
	{
		LOG_ERROR("Alpha map '%s' had wrong width %i, expected width"
			" is %i.", el_file_name(file), image_height,
			image->height);

		return 0;
	}

	x_padding = image_width % 4;

	if (x_padding)
	{
		x_padding = 4 - x_padding;
	}
	if (image_width <= x_padding)
	{
		x_padding = 0;
	}

	data = image->image;
	image->alpha = 1;

	idx = 0;
	pixel = 0;
	bpp = image_surface->format->BytesPerPixel;

	for (i = 0; i < image_height; i++)
	{
		for (j = 0; j < image_width; j++)
		{
			if ((image_surface->format->BitsPerPixel == 8) &&
				(image_surface->format->palette != 0))
			{
				index = ((Uint8 *)image_surface->pixels)[idx];
				r = image_surface->format->palette->colors[index].r;
				g = image_surface->format->palette->colors[index].g;
				b = image_surface->format->palette->colors[index].b;
				a = (r + g + b) / 3;
			}
			else
			{
				memcpy(&pixel, &((Uint8 *)image_surface->pixels)[idx], bpp);

				/* Get Alpha component */
				temp = pixel & image_surface->format->Amask;  /* Isolate alpha component */
				temp = temp >> image_surface->format->Ashift; /* Shift it down to 8-bit */
				temp = temp << image_surface->format->Aloss;  /* Expand to a full 8-bit number */
				a = (Uint8)temp;
			}

			idx += bpp;

			index = (image_height - i - 1) * image_width + j;

			data[index * 4 + 3] = a;
		}
		idx += bpp * x_padding;
	}

	SDL_UnlockSurface(image_surface);
	SDL_FreeSurface(image_surface);

	return 1;
}

Uint32 load_image_data_file(el_file_ptr file, const Uint32 compression,
	const Uint32 unpack, const Uint32 strip_mipmaps,
	const Uint32 base_level, image_t* image)
{
	char buffer[128];
	el_file_ptr alpha_file;
	Uint32 dds, result;

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");

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
		result = load_dds(file, compression, unpack,
			strip_mipmaps, base_level, image);
	}
	else
	{
		result = load_image_SDL(file, image);
	}

	if ((result == 0) || (image->image == 0))
	{
		LOG_ERROR("Can't load file '%s'!", el_file_name(file));

		el_close(file);

		return 0;
	}

	if ((dds != 1) && (check_alpha_image_name(el_file_name(file),
		sizeof(buffer), buffer) != 0))
	{
		alpha_file = el_open_custom(buffer);

		if (alpha_file == 0)
		{
			LOG_ERROR("Can't load file '%s'!", buffer);

			el_close(file);

			return 0;
		}

		load_image_SDL_alpha(alpha_file, image);

		el_close(alpha_file);
	}

	el_close(file);

	return 1;
}

Uint32 load_image_data(const char* file_name, const Uint32 compression,
	const Uint32 unpack, const Uint32 strip_mipmaps,
	const Uint32 base_level, image_t* image)
{
	char buffer[128];
	el_file_ptr file;

	if (check_image_name(file_name, sizeof(buffer), buffer) == 0)
	{
		LOG_ERROR("File '%s' not found!", file_name);
		return 0;
	}

	file = el_open_custom(buffer);

	if (file == 0)
	{
		LOG_ERROR("Can't load file '%s'!", file_name);

		return 0;
	}

	return load_image_data_file(file, compression, unpack, strip_mipmaps,
		base_level, image);
}

Uint32 get_image_information(el_file_ptr file, image_t* image)
{
	Uint32 dds, result;

	if (file == 0)
	{
		LOG_ERROR("Invalid file!");

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
		result = get_dds_information(file, image);

		el_seek(file, 0, SEEK_SET);

		return result;
	}
	else
	{
		return get_sdl_image_information(file, image);
	}
}

void free_image(image_t* image)
{
	if (image == 0)
	{
		LOG_ERROR("Invalid image!");

		return;
	}

	free_aligned(image->image);

	image->image = 0;
}

#endif	/* NEW_TEXTURES */
