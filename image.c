/****************************************************************************
 *            image.c
 *
 * Author: 2011  Daniel Jungmann <dsj@gmx.net>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "image.h"
#ifdef	USE_SIMD
#include "errors.h"
#include <SDL.h>
#include <xmmintrin.h>

void unpack_rgba8_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0, t1, t2;
	Uint32 i;

	for (i = 0; i < (size / 16); i++)
	{
		t0 = _mm_load_si128((__m128i*)&source[i * 16]);

		t1 = _mm_and_si128(t0, _mm_set1_epi16(0x00FF));
		t2 = _mm_and_si128(t0, _mm_set1_epi16(0xFF00));
		t1 = _mm_shufflelo_epi16(t1, _MM_SHUFFLE(2, 3, 0, 1));
		t1 = _mm_shufflehi_epi16(t1, _MM_SHUFFLE(2, 3, 0, 1));
		t1 = _mm_or_si128(t1, t2);

		_mm_stream_si128((__m128i*)&dest[i * 16], t1);
	}
}

void unpack_r5g6b5_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0, t1, t2;
	Uint32 i;

	for (i = 0; i < (size / 8); i++)
	{
		t0 = _mm_loadl_epi64((__m128i*)&source[i * 8]);

		t0 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_and_si128(t1, _mm_set_epi16(0x0000, 0x001F, 0x07E0, 0xF800, 0x0000, 0x001F, 0x07E0, 0xF800));
		t1 = _mm_mullo_epi16(t1, _mm_set_epi16(0x0000, 0x0800, 0x0008, 0x0001, 0x0001, 0x0800, 0x0008, 0x0001));
		t1 = _mm_mulhi_epu16(t1, _mm_set_epi16(0x0200, 0x0260, 0x5B00, 0x0260, 0x0200, 0x0260, 0x5B00, 0x0260));
		t1 = _mm_mulhi_epu16(t1, _mm_set_epi16(0xFF00, 0x6ED5, 0x0B63, 0x6ED5, 0xFF00, 0x6ED5, 0x0B63, 0x6ED5));
		t2 = _mm_unpackhi_epi16(t0, t0);
		t2 = _mm_and_si128(t2, _mm_set_epi16(0x0000, 0x001F, 0x07E0, 0xF800, 0x0000, 0x001F, 0x07E0, 0xF800));
		t2 = _mm_mullo_epi16(t2, _mm_set_epi16(0x0000, 0x0800, 0x0008, 0x0001, 0x0001, 0x0800, 0x0008, 0x0001));
		t2 = _mm_mulhi_epu16(t2, _mm_set_epi16(0x0200, 0x0260, 0x5B00, 0x0260, 0x0200, 0x0260, 0x5B00, 0x0260));
		t2 = _mm_mulhi_epu16(t2, _mm_set_epi16(0xFF00, 0x6ED5, 0x0B63, 0x6ED5, 0xFF00, 0x6ED5, 0x0B63, 0x6ED5));
		t1 = _mm_packus_epi16(t1, t2);
		t1 = _mm_or_si128(t1, _mm_set1_epi32(0xFF000000));

		_mm_stream_si128((__m128i*)&dest[i * 16], t1);
	}
}

void unpack_rgb5a1_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0, t1, t2;
	Uint32 i;

	for (i = 0; i < (size / 8); i++)
	{
		t0 = _mm_loadl_epi64((__m128i*)&source[i * 8]);

		t0 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_and_si128(t1, _mm_set_epi16(0x8000, 0x001F, 0x03E0, 0x7C00, 0x8000, 0x001F, 0x03E0, 0x7C00));
		t1 = _mm_mullo_epi16(t1, _mm_set_epi16(0x0001, 0x0800, 0x0040, 0x0002, 0x0001, 0x0800, 0x0040, 0x0002));
		t1 = _mm_mulhi_epu16(t1, _mm_set_epi16(0x0200, 0x0260, 0x0260, 0x0260, 0x0200, 0x0260, 0x0260, 0x0260));
		t1 = _mm_mulhi_epu16(t1, _mm_set_epi16(0xFF00, 0x6ED5, 0x6ED5, 0x6ED5, 0xFF00, 0x6ED5, 0x6ED5, 0x6ED5));
		t2 = _mm_unpackhi_epi16(t0, t0);
		t2 = _mm_and_si128(t2, _mm_set_epi16(0x8000, 0x001F, 0x03E0, 0x7C00, 0x8000, 0x001F, 0x03E0, 0x7C00));
		t2 = _mm_mullo_epi16(t2, _mm_set_epi16(0x0001, 0x0800, 0x0040, 0x0002, 0x0001, 0x0800, 0x0040, 0x0002));
		t2 = _mm_mulhi_epu16(t2, _mm_set_epi16(0x0200, 0x0260, 0x0260, 0x0260, 0x0200, 0x0260, 0x0260, 0x0260));
		t2 = _mm_mulhi_epu16(t2, _mm_set_epi16(0xFF00, 0x6ED5, 0x6ED5, 0x6ED5, 0xFF00, 0x6ED5, 0x6ED5, 0x6ED5));
		t1 = _mm_packus_epi16(t1, t2);

		_mm_stream_si128((__m128i*)&dest[i * 16], t1);
	}
}

void unpack_rgba4_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0, t1, t2;
	Uint32 i;

	for (i = 0; i < (size / 8); i++)
	{
		t0 = _mm_loadl_epi64((__m128i*)&source[i * 8]);
		// converts 4 bit values to 8 bit values (multiply with 17)
		t0 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_unpacklo_epi16(t0, t0);
		t1 = _mm_and_si128(t1, _mm_set_epi16(0xF000, 0x000F, 0x00F0, 0x0F00, 0xF000, 0x000F, 0x00F0, 0x0F00));
		t1 = _mm_mullo_epi16(t1, _mm_set_epi16(0x0001, 0x1000, 0x0100, 0x0010, 0x0001, 0x1000, 0x0100, 0x0010));
		t1 = _mm_mulhi_epu16(t1, _mm_set1_epi16(0x0110));
		t2 = _mm_unpackhi_epi16(t0, t0);
		t2 = _mm_and_si128(t2, _mm_set_epi16(0xF000, 0x000F, 0x00F0, 0x0F00, 0xF000, 0x000F, 0x00F0, 0x0F00));
		t2 = _mm_mullo_epi16(t2, _mm_set_epi16(0x0001, 0x1000, 0x0100, 0x0010, 0x0001, 0x1000, 0x0100, 0x0010));
		t2 = _mm_mulhi_epu16(t2, _mm_set1_epi16(0x0110));
		t1 = _mm_packus_epi16(t1, t2);

		_mm_stream_si128((__m128i*)&dest[i * 16], t1);
	}
}

void unpack_a8_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0;
	Uint32 i;

	for (i = 0; i < (size / 4); i++)
	{
		t0 = (__m128i)_mm_load_ss((float*)&source[i * 4]);

		t0 = _mm_unpacklo_epi8(_mm_setzero_si128(), t0);
		t0 = _mm_unpacklo_epi16(_mm_setzero_si128(), t0);

		_mm_stream_si128((__m128i*)&dest[i * 16], t0);
	}
}

void unpack_l8_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0;
	Uint32 i;

	for (i = 0; i < (size / 4); i++)
	{
		t0 = (__m128i)_mm_load_ss((float*)&source[i * 4]);

		t0 = _mm_unpacklo_epi8(t0, t0);
		t0 = _mm_unpacklo_epi16(t0, t0);
		t0 = _mm_or_si128(t0, _mm_set1_epi32(0xFF000000));

		_mm_stream_si128((__m128i*)&dest[i * 16], t0);
	}
}

void unpack_la8_sse2(const Uint8* source, const Uint32 size, Uint8* dest)
{
	__m128i t0, t1, t2;
	Uint32 i;

	for (i = 0; i < (size / 8); i++)
	{
		t0 = _mm_loadl_epi64((__m128i*)&source[i * 8]);

		t1 = _mm_unpacklo_epi8(t0, t0);
		t1 = _mm_and_si128(t1, _mm_set1_epi32(0x0000FFFF));
		t2 = _mm_unpacklo_epi16(_mm_setzero_si128(), t0);
		t1 = _mm_or_si128(t1, t2);

		_mm_stream_si128((__m128i*)&dest[i * 16], t1);
	}
}

void replace_a8_rgba8_sse2(const Uint8* alpha, const Uint32 size, Uint8* source)
{
	__m128i t0;
	Uint32 i;

	for (i = 0; i < (size / 4); i++)
	{
		t0 = (__m128i)_mm_load_ss((float*)&alpha[i * 4]);

		t0 = _mm_unpacklo_epi8(_mm_setzero_si128(), t0);
		t0 = _mm_unpacklo_epi16(_mm_setzero_si128(), t0);

		_mm_maskmoveu_si128(t0, _mm_set1_epi32(0xFF000000),
			(char*)&source[i * 16]);
	}
}

void replace_alpha_rgba8_sse2(const Uint8 alpha, const Uint32 size, Uint8* source)
{
	__m128i t0;
	Uint32 i;

	t0 = _mm_set1_epi8(alpha);

	for (i = 0; i < (size / 4); i++)
	{
		_mm_maskmoveu_si128(t0, _mm_set1_epi32(0xFF000000),
			(char*)&source[i * 16]);
	}
}

void blend_sse2(const Uint8* alpha, const Uint32 size, const Uint8* source0,
	const Uint8* source1, Uint8* dest)
{
	__m128i t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
	__m128i t11;
	Uint32 i;

	LOG_DEBUG_VERBOSE("source0[0]: %d", source0[0]);
	LOG_DEBUG_VERBOSE("&source0[0]: %p", &(source0[0]));
	LOG_DEBUG_VERBOSE("source1[0]: %d", source1[0]);
	LOG_DEBUG_VERBOSE("&source1[0]: %p", &(source1[0]));
	LOG_DEBUG_VERBOSE("alpha[0]: %d", alpha[0]);
	LOG_DEBUG_VERBOSE("&alpha[0]: %p", &(alpha[0]));
	LOG_DEBUG_VERBOSE("dest[0]: %d", dest[0]);
	LOG_DEBUG_VERBOSE("&dest[0]: %p", &(dest[0]));
	LOG_DEBUG_VERBOSE("size: %p", size);

	t11 = _mm_cmpeq_epi32(t11, t11);

	LOG_DEBUG_VERBOSE("cmp");

	t2 = (__m128i)_mm_load_ss((float*)alpha);

	LOG_DEBUG_VERBOSE("load");

	t11 = _mm_or_si128(t11, t2);

	LOG_DEBUG_VERBOSE("or");

	for (i = 0; i < (size / 4); i++)
	{
		t0 = _mm_load_si128((__m128i*)&(source0[i * 16]));
		t1 = _mm_load_si128((__m128i*)&(source1[i * 16]));
		t2 = (__m128i)_mm_load_ss((float*)&(alpha[i * 4]));

		t2 = _mm_unpacklo_epi8(t2, t2);
		t2 = _mm_unpacklo_epi16(t2, t2);

		t3 = _mm_unpacklo_epi8(t0, t0);
		t4 = _mm_unpacklo_epi8(t1, t1);

		t5 = _mm_unpacklo_epi32(t2, t2);
//		t6 = _mm_sub_epi16(_mm_set1_epi8(0xFF), t5);
		t6 = _mm_sub_epi16(t11, t5);

		t7 = _mm_mulhi_epu16(t3, t6);
		t8 = _mm_mulhi_epu16(t4, t5);

		t9 = _mm_adds_epu16(t7, t8);
		t9 = _mm_srli_epi16(t9, 8);

		t3 = _mm_unpackhi_epi8(t0, t0);
		t4 = _mm_unpackhi_epi8(t1, t1);

		t5 = _mm_unpackhi_epi32(t2, t2);
//		t6 = _mm_sub_epi16(_mm_set1_epi8(0xFF), t5);
		t6 = _mm_sub_epi16(t11, t5);

		t7 = _mm_mulhi_epu16(t3, t6);
		t8 = _mm_mulhi_epu16(t4, t5);

		t10 = _mm_adds_epu16(t7, t8);
		t10 = _mm_srli_epi16(t10, 8);

		t10 = _mm_packus_epi16(t9, t10);

		_mm_stream_si128((__m128i*)&(dest[i * 16]), t10);
	}
}

Uint32 check_pointer_aligment(const void* ptr)
{
	return (((ptrdiff_t)ptr) & 0x0F) == 0;
}
#endif

void unpack(const Uint8* source, const Uint32 count, const Uint32 red,
	const Uint32 green, const Uint32 blue, const Uint32 alpha, Uint8* dest)
{
	Uint32 i, pixel, temp, bpp;
	Uint32 r, g, b, a;
	Uint32 red_shift, red_mask;
	Uint32 green_shift, green_mask;
	Uint32 blue_shift, blue_mask;
	Uint32 alpha_shift, alpha_mask;

	bpp = popcount(red | green | blue | alpha) / 8;

	red_shift = 0;
	green_shift = 0;
	blue_shift = 0;
	alpha_shift = 0;

	while ((red & (1 << red_shift)) == 0)
	{
		red_shift++;
	}

	while ((green & (1 << green_shift)) == 0)
	{
		green_shift++;
	}

	while ((blue & (1 << blue_shift)) == 0)
	{
		blue_shift++;
	}

	while ((alpha & (1 << alpha_shift)) == 0)
	{
		alpha_shift++;
	}

	red_mask = red >> red_shift;
	green_mask = green >> green_shift;
	blue_mask = blue >> blue_shift;
	alpha_mask = alpha >> alpha_shift;

	for (i = 0; i < count; i++)
	{
		memcpy(&pixel, &source[i * bpp], bpp);

		/* Get Red component */
		temp = pixel >> red_shift;
		temp = temp & red_mask;
		temp = (temp * 255) / red_mask;
		r = (Uint8)temp;

		/* Get Green component */
		temp = pixel >> green_shift;
		temp = temp & green_mask;
		temp = (temp * 255) / green_mask;
		g = (Uint8)temp;

		/* Get Blue component */
		temp = pixel >> blue_shift;
		temp = temp & blue_mask;
		temp = (temp * 255) / blue_mask;
		b = (Uint8)temp;

		/* Get Alpha component */
		if (alpha_mask != 0)
		{
			temp = pixel >> alpha_shift;
			temp = temp & alpha_mask;
			temp = (temp * 255) / alpha_mask;
			a = (Uint8)temp;
		}
		else
		{
			a = 255;
		}

		dest[i * 4 + 0] = r;
		dest[i * 4 + 1] = g;
		dest[i * 4 + 2] = b;
		dest[i * 4 + 3] = a;
	}
}

void fast_unpack(const Uint8* source, const Uint32 size, const Uint32 red,
	const Uint32 green, const Uint32 blue, const Uint32 alpha, Uint8* dest)
{
#ifdef	USE_SIMD
	if (SDL_HasSSE2())
	{
		if (((size & 0x0F) == 0) && check_pointer_aligment(source) &&
			check_pointer_aligment(dest))
		{
			if ((red == 0x000000FF) && (green == 0x000000FF) &&
				(blue == 0x000000FF) && (alpha == 0x00000000))
			{
				unpack_l8_sse2(source, size, dest);
				return;
			}

			if ((red == 0x00000000) && (green == 0x00000000) &&
				(blue == 0x00000000) && (alpha == 0x000000FF))
			{
				unpack_a8_sse2(source, size, dest);
				return;
			}

			if ((red == 0x000000FF) && (green == 0x000000FF) &&
				(blue == 0x000000FF) && (alpha == 0x0000FF00))
			{
				unpack_la8_sse2(source, size * 2, dest);
				return;
			}

			if ((red == 0x00FF0000) && (green == 0x0000FF00) &&
				(blue == 0x000000FF) && (alpha == 0xFF000000))
			{
				unpack_rgba8_sse2(source, size * 4, dest);
				return;
			}

			if ((red == 0x0000F800) && (green == 0x000007E0) &&
				(blue == 0x0000001F) && (alpha == 0x00000000))
			{
				unpack_r5g6b5_sse2(source, size * 2, dest);
				return;
			}

			if ((red == 0x00007C00) && (green == 0x000003E0) &&
				(blue == 0x0000001F) && (alpha == 0x00008000))
			{
				unpack_rgb5a1_sse2(source, size * 2, dest);
				return;
			}

			if ((red == 0x00000F00) && (green == 0x000000F0) &&
				(blue == 0x0000000F) && (alpha == 0x0000F000))
			{
				unpack_rgba4_sse2(source, size * 2, dest);
				return;
			}
		}
	}
#endif
	unpack(source, size, red, green, blue, alpha, dest);
}

void fast_replace_a8_rgba8(const Uint8* alpha, const Uint32 size, Uint8* source)
{
	Uint32 i;

#ifdef	USE_SIMD
	if (SDL_HasSSE2())
	{
		if (((size & 0x03) == 0) && check_pointer_aligment(source))
		{
			replace_a8_rgba8_sse2(alpha, size, source);
			return;
		}
	}
#endif

	for (i = 0; i < size; i++)
	{
		source[i * 4 + 3] = alpha[i];
	}	
}

void fast_replace_alpha_rgba8(const Uint8 alpha, const Uint32 size, Uint8* source)
{
	Uint32 i;

#ifdef	USE_SIMD
	if (SDL_HasSSE2())
	{
		if (((size & 0x03) == 0) && check_pointer_aligment(source))
		{
			replace_alpha_rgba8_sse2(alpha, size, source);
			return;
		}
	}
#endif

	for (i = 0; i < size; i++)
	{
		source[i * 4 + 3] = alpha;
	}	
}

void fast_blend(const Uint8* alpha, const Uint32 size, const Uint8* source0,
	const Uint8* source1, Uint8* dest)
{
	Uint32 i, j, tmp;

#ifdef	USE_SIMD
	if (SDL_HasSSE2())
	{
		if (((size & 0x03) == 0) && check_pointer_aligment(source0) &&
			check_pointer_aligment(source1) &&
			check_pointer_aligment(dest))
		{
			blend_sse2(alpha, size, source0, source1, dest);
			return;
		}
	}
#endif

	for (i = 0; i < size; i++)
	{
		for (j = 0; j < 4; j++)
		{
			tmp = source1[i * 4 + j] * alpha[i];
			tmp += source0[i * 4 + j] * (255 - alpha[i]);
			dest[i * 4 + j] = tmp / 255;
		}
	}
}

