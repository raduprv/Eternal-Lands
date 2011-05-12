/****************************************************************************
 *            dds.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "dds.h"

void unpack_dxt_color(DXTColorBlock *block, Uint8 *values, Uint32 dxt1)
{
	float colors[4][4];
	Uint32 i, j, index;

	colors[0][0] = (block->m_colors[0] & 0xF800) >> 11;
	colors[0][1] = (block->m_colors[0] & 0x07E0) >> 5;
	colors[0][2] = block->m_colors[0] & 0x001F;
	colors[0][3] = 255.0f;

	colors[0][0] *= 255.0f / 31.0f;
	colors[0][1] *= 255.0f / 63.0f;
	colors[0][2] *= 255.0f / 31.0f;

	colors[1][0] = (block->m_colors[1] & 0xF800) >> 11;
	colors[1][1] = (block->m_colors[1] & 0x07E0) >> 5;
	colors[1][2] = block->m_colors[1] & 0x001F;
	colors[1][3] = 255.0f;

	colors[1][0] *= 255.0f / 31.0f;
	colors[1][1] *= 255.0f / 63.0f;
	colors[1][2] *= 255.0f / 31.0f;

	if ((dxt1 == 1) && (block->m_colors[0] <= block->m_colors[1]))
	{
		// 1-bit alpha
		// one intermediate colour, half way between the other two
		colors[2][0] = (colors[0][0] + colors[1][0]) / 2.0f;
		colors[2][1] = (colors[0][1] + colors[1][1]) / 2.0f;
		colors[2][2] = (colors[0][2] + colors[1][2]) / 2.0f;
		colors[2][3] = (colors[0][3] + colors[1][3]) / 2.0f;
		// transparent colour
		colors[3][0] = 0.0f;
		colors[3][1] = 0.0f;
		colors[3][2] = 0.0f;
		colors[3][3] = 0.0f;
	}
	else
	{
		// first interpolated colour, 1/3 of the way along
		colors[2][0] = (2.0f * colors[0][0] + colors[1][0]) / 3.0f;
		colors[2][1] = (2.0f * colors[0][1] + colors[1][1]) / 3.0f;
		colors[2][2] = (2.0f * colors[0][2] + colors[1][2]) / 3.0f;
		colors[2][3] = (2.0f * colors[0][3] + colors[1][3]) / 3.0f;
		// second interpolated colour, 2/3 of the way along
		colors[3][0] = (colors[0][0] + 2.0f * colors[1][0]) / 3.0f;
		colors[3][1] = (colors[0][1] + 2.0f * colors[1][1]) / 3.0f;
		colors[3][2] = (colors[0][2] + 2.0f * colors[1][2]) / 3.0f;
		colors[3][3] = (colors[0][3] + 2.0f * colors[1][3]) / 3.0f;
	}

	// Process 4x4 block of texels
	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < 4; ++j)
		{
			// LSB come first
			index = block->m_indices[i] >> (j * 2) & 0x3;

			values[((i * 4) + j) * 4 + 0] = colors[index][0];
			values[((i * 4) + j) * 4 + 1] = colors[index][1];
			values[((i * 4) + j) * 4 + 2] = colors[index][2];

			if (dxt1)
			{
				// Overwrite entire colour
				values[((i * 4) + j) * 4 + 3] = colors[index][3];
			}
		}
	}
}

void unpack_dxt_explicit_alpha(DXTExplicitAlphaBlock *block, Uint8 *values)
{
	float value;
	Uint32 i, j, index;

	index = 0;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			value = block->m_alphas[i] >> (j * 4) & 0xF;
			values[index] = value * 17.0f;	// = (value * 255.0f) / 15.0f;
			index++;
		}
	}
}

void unpack_dxt_interpolated_alpha(DXTInterpolatedAlphaBlock *block, Uint8 *values)
{
	float alphas[8];
	float scale, f0, f1;
	Uint32 i, index, idx0, idx1;

	alphas[0] = block->m_alphas[0];
	alphas[1] = block->m_alphas[1];

	if (block->m_alphas[0] > block->m_alphas[1])
	{
		scale = 1.0f / 7.0f;

		for (i = 0; i < 6; i++)
		{
			f0 = (6 - i) * scale;
			f1 = (i + 1) * scale;
			alphas[i + 2] = (f0 * block->m_alphas[0]) + (f1 * block->m_alphas[1]);
		}
	}
	else
	{
		// 4 interpolated alphas, plus zero and one
		// full range including extremes at [0] and [5]
		// we want to fill in [1] through [4] at weights ranging
		// from 1/5 to 4/5
		scale = 1.0f / 5.0f;

		for (i = 0; i < 4; i++)
		{
			f0 = (4 - i) * scale;
			f1 = (i + 1) * scale;
			alphas[i + 2] = (f0 * block->m_alphas[0]) + (f1 * block->m_alphas[1]);
		}

		alphas[6] = 0.0f;
		alphas[7] = 255.0f;
	}

	for (i = 0; i < 16; i++)
	{
		idx0 = (i * 3) / 8;
		idx1 = (i * 3) % 8;
		index = (block->m_indices[idx0] >> idx1) & 0x07;

		if (idx1 > 5)
		{
			index |= (block->m_indices[idx0 + 1] << (8 - idx1)) & 0x07;
		}

		values[i] = alphas[index];
	}
}

void unpack_dxt1(DXTColorBlock *block, Uint8 *values)
{
	unpack_dxt_color(block, values, 1);
}

void unpack_dxt3(DXTExplicitAlphaBlock *alpha_block, DXTColorBlock *color_block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_color(color_block, values, 0);
	unpack_dxt_explicit_alpha(alpha_block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_dxt5(DXTInterpolatedAlphaBlock *alpha_block, DXTColorBlock *color_block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_color(color_block, values, 0);
	unpack_dxt_interpolated_alpha(alpha_block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_ati1(DXTInterpolatedAlphaBlock *block, Uint8 *values)
{
	Uint8 alpha_values[16];
	Uint32 i;

	unpack_dxt_interpolated_alpha(block, alpha_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 0] = alpha_values[i];
		values[i * 4 + 1] = alpha_values[i];
		values[i * 4 + 2] = alpha_values[i];
		values[i * 4 + 3] = alpha_values[i];
	}
}

void unpack_ati2(DXTInterpolatedAlphaBlock *first_block, DXTInterpolatedAlphaBlock *second_block,
	Uint8 *values)
{
	Uint8 first_values[16], second_values[16];
	Uint32 i;

	unpack_dxt_interpolated_alpha(first_block, first_values);
	unpack_dxt_interpolated_alpha(second_block, second_values);

	for (i = 0; i < 16; i++)
	{
		values[i * 4 + 0] = first_values[i];
		values[i * 4 + 1] = first_values[i];
		values[i * 4 + 2] = first_values[i];
		values[i * 4 + 3] = second_values[i];
	}
}
