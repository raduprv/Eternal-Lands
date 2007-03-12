#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	display_utils
 * \brief 	Normal map calculation using SSE, if defined USE_SSE2 using SSE2
 * and if defined USE_SSE3 using SSE3.
 */
#ifndef NORMALS_SSE_H
#define NORMALS_SSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "simd.h"

#ifdef	USE_SSE3
#include <pmmintrin.h>
#else
#ifdef	USE_SSE2
#include <emmintrin.h>
#else
#include <xmmintrin.h>
#endif
#endif

/*!
 * \ingroup 	display_utils
 * \brief 	Calculates the current normal map of the float terrain height map.
 *
 * Calculates the current normal map of the float terrain height map with
 * SSE & MMX instructions, with SSE & SSE2 instructions if USE_SSE2 is defined or
 * with SSE, SSE2 & SSE3 if USE_SSE3 is defined. Then calls build_normal_texures for 
 * normal texture construction. 
 * Use this function with care because it frees h_map_f.
 * \param 	h_map The terrain height map. Address must be 16 Byte aligned for SSE2.
 * \param 	size_x The size of the terrain height map in x direction. Must be a multiple 
 * 		of four for SSE and a multiple of eight for SSE2.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static __inline__ void calc_normal_map_float_sse(float* h_map_f, const unsigned int size_x, 
		const unsigned int size_y)
{
	unsigned int i, j, n_index, n_row;
	__m128 h0, h1, h2, h3, h4, h5;
	__m128 t1, t2, t3, t4, t5, t6;
	__m128 v1_x, v1_y, v2_x, v2_y;
	__m128 v1, v2, v3, v4, v5, v6, v7, v8;
#ifdef	UNROLL4
	__m128 v9, v10, v11, v12, v13, v14, v15, v16, v17, v18;
#endif
	__m128 half, zero, one;	
	VECTOR4* surface_normals;
	VECTOR4* normal_map;
	
	surface_normals = (VECTOR4*)_mm_malloc((size_x+1)*(size_y+1)*2*sizeof(__m128), sizeof(__m128));
	normal_map = (VECTOR4*)_mm_malloc(size_x*size_y*sizeof(__m128), sizeof(__m128));
	one = _mm_set1_ps(1.0f);
	zero = _mm_set1_ps(0.0f);
	half = _mm_set1_ps(0.5f);
	n_index = 0;
	
	for (i = 0; i < size_x+1; i++)		
	{
		_mm_stream_ps(surface_normals[n_index+0], zero);
		_mm_stream_ps(surface_normals[n_index+1], zero);
		n_index += 2;
	}
	
	for (i = 0; i < size_y-1; i++)
	{
		_mm_stream_ps(surface_normals[n_index+0], zero);
		_mm_stream_ps(surface_normals[n_index+1], zero);
		n_index += 2;	
		h1 = _mm_load_ps(&h_map_f[i*size_x]);
		h3 = _mm_load_ps(&h_map_f[(i+1)*size_x]);
		for (j = 4; j < size_x; j+=4)
		{
			h0 = h1;
			h1 = _mm_load_ps(&h_map_f[i*size_x+j]);
			h2 = h3;
			h3 = _mm_load_ps(&h_map_f[(i+1)*size_x+j]);
			h4 = _mm_move_ss(h0, h1);
			h5 = _mm_move_ss(h2, h3);
			h4 = _mm_shuffle_ps(h4, h4, _MM_SHUFFLE(0, 3, 2, 1));
			h5 = _mm_shuffle_ps(h5, h5, _MM_SHUFFLE(0, 3, 2, 1));

			v1_x = _mm_sub_ps(h0, h2);
			v1_y = _mm_sub_ps(h0, h4);			
			v2_x = _mm_sub_ps(h4, h5);
			v2_y = _mm_sub_ps(h2, h5);
			
			t1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v1_x, v1_x), _mm_mul_ps(v1_y, v1_y)), one);
			t2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v2_x, v2_x), _mm_mul_ps(v2_y, v2_y)), one);
			
			t1 = _mm_rsqrt_ps(t1);	// t1 = [1/sqrt(t1.x), 1/sqrt(t1.y), 1/sqrt(t1.z), 1/sqrt(t1.w)]
			t2 = _mm_rsqrt_ps(t2);	// t2 = [1/sqrt(t2.x), 1/sqrt(t2.y), 1/sqrt(t2.z), 1/sqrt(t2.w)]

			v1_x = _mm_mul_ps(v1_x, t1);
			v1_y = _mm_mul_ps(v1_y, t1);
			v2_x = _mm_mul_ps(v2_x, t2);
			v2_y = _mm_mul_ps(v2_y, t2);
			
			t3 = _mm_unpacklo_ps(t1, zero);		// t3 = {t1.x, 0.0f, t1.y, 0.0f}
			t4 = _mm_unpackhi_ps(t1, zero);		// t4 = {t1.z, 0.0f, t1.w 0.0f}
			t5 = _mm_unpacklo_ps(t2, zero);		// t5 = {t2.x, 0.0f, t2.y, 0.0f}
			t6 = _mm_unpackhi_ps(t2, zero);		// t6 = {t2.z, 0.0f, t2.w, 0.0f}
						
			t1 = _mm_unpacklo_ps(v1_x, v1_y);	// t1 = {v1_x.0, v1_y.0, v1_x.1, v1_y.1}
			t2 = _mm_unpackhi_ps(v1_x, v1_y);	// t2 = {v1_x.2, v1_y.2, v1_x.3, v1_y.3}
			
			v1 = _mm_movelh_ps(t1, t3);		// v1 = {v1_x.0, v1_y.0, t1.x, 0.0f}
			v2 = _mm_movehl_ps(t3, t1);		// v2 = {v1_x.1, v1_y.1, t1.y, 0.0f}
			v3 = _mm_movelh_ps(t2, t4);		// v3 = {v1_x.2, v1_y.2, t1.z, 0.0f}
			v4 = _mm_movehl_ps(t4, t2);		// v4 = {v1_x.3, v1_y.3, t1.2, 0.0f}
						
			t1 = _mm_unpacklo_ps(v2_x, v2_y);	// t1 = {v2_x.0, v2_y.0, v2_x.1, v2_y.1}
			t2 = _mm_unpackhi_ps(v2_x, v2_y);	// t2 = {v2_x.2, v2_y.2, v2_x.3, v2_y.3}
			
			v5 = _mm_movelh_ps(t1, t5);		// v5 = {v2_x.0, v2_y.0, t2.x, 0.0f}
			v6 = _mm_movehl_ps(t5, t1);		// v6 = {v2_x.1, v2_y.1, t2.y, 0.0f}
			v7 = _mm_movelh_ps(t2, t6);		// v7 = {v2_x.2, v2_y.2, t2.z, 0.0f}
			v8 = _mm_movehl_ps(t6, t2);		// v8 = {v2_x.3, v2_y.3, t2.w, 0.0f}
			
			_mm_stream_ps(surface_normals[n_index+0], v1);
			_mm_stream_ps(surface_normals[n_index+2], v2);
			_mm_stream_ps(surface_normals[n_index+4], v3);
			_mm_stream_ps(surface_normals[n_index+6], v4);			
			_mm_stream_ps(surface_normals[n_index+1], v5);
			_mm_stream_ps(surface_normals[n_index+3], v6);
			_mm_stream_ps(surface_normals[n_index+5], v7);
			_mm_stream_ps(surface_normals[n_index+7], v8);
			
			n_index += 8;
		}
		h0 = h1;
		h2 = h3;
		
		h4 = _mm_shuffle_ps(h0, h0, _MM_SHUFFLE(0, 3, 2, 1));
		h5 = _mm_shuffle_ps(h2, h2, _MM_SHUFFLE(0, 3, 2, 1));

		v1_x = _mm_sub_ps(h0, h2);
		v1_y = _mm_sub_ps(h0, h4);			
		v2_x = _mm_sub_ps(h4, h5);
		v2_y = _mm_sub_ps(h2, h5);
			
		t1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v1_x, v1_x), _mm_mul_ps(v1_y, v1_y)), one);
		t2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v2_x, v2_x), _mm_mul_ps(v2_y, v2_y)), one);
			
		t1 = _mm_rsqrt_ps(t1);
		t2 = _mm_rsqrt_ps(t2);

		v1_x = _mm_mul_ps(v1_x, t1);
		v1_y = _mm_mul_ps(v1_y, t1);
		v2_x = _mm_mul_ps(v2_x, t2);
		v2_y = _mm_mul_ps(v2_y, t2);

		t3 = _mm_unpacklo_ps(t1, zero);		// t3 = {t1.x, 0.0f, t1.y, 0.0f}
		t4 = _mm_unpackhi_ps(t1, zero);		// t4 = {t1.z, 0.0f, t1.w 0.0f}
		t5 = _mm_unpacklo_ps(t2, zero);		// t5 = {t2.x, 0.0f, t2.y, 0.0f}
		t6 = _mm_unpackhi_ps(t2, zero);		// t6 = {t2.z, 0.0f, t2.w, 0.0f}

		t1 = _mm_unpacklo_ps(v1_x, v1_y);	// t1 = {v1_x.0, v1_y.0, v1_x.1, v1_y.1}
		t2 = _mm_unpackhi_ps(v1_x, v1_y);	// t2 = {v1_x.2, v1_y.2, v1_x.3, v1_y.3}
		
		v1 = _mm_movelh_ps(t1, t3);		// v1 = {v1_x.0, v1_y.0, t1.x, 0.0f}
		v2 = _mm_movehl_ps(t3, t1);		// v2 = {v1_x.1, v1_y.1, t1.y, 0.0f}
		v3 = _mm_movelh_ps(t2, t4);		// v3 = {v1_x.2, v1_y.2, t1.z, 0.0f}
						
		t1 = _mm_unpacklo_ps(v2_x, v2_y);	// t1 = {v2_x.0, v2_y.0, v2_x.1, v2_y.1}
		t2 = _mm_unpackhi_ps(v2_x, v2_y);	// t2 = {v2_x.2, v2_y.2, v2_x.3, v2_y.3}
			
		v5 = _mm_movelh_ps(t1, t5);		// v5 = {v2_x.0, v2_y.0, t2.x, 0.0f}
		v6 = _mm_movehl_ps(t5, t1);		// v6 = {v2_x.1, v2_y.1, t2.y, 0.0f}
		v7 = _mm_movelh_ps(t2, t6);		// v7 = {v2_x.2, v2_y.2, t2.z, 0.0f}

		_mm_stream_ps(surface_normals[n_index+0], v1);
		_mm_stream_ps(surface_normals[n_index+2], v2);
		_mm_stream_ps(surface_normals[n_index+4], v3);			
		_mm_stream_ps(surface_normals[n_index+1], v5);
		_mm_stream_ps(surface_normals[n_index+3], v6);
		_mm_stream_ps(surface_normals[n_index+5], v7);
		_mm_stream_ps(surface_normals[n_index+6], zero);
		_mm_stream_ps(surface_normals[n_index+7], zero);

		n_index += 8;
	}

	for (i = 0; i < size_x+1; i++)		
	{
		_mm_stream_ps(surface_normals[n_index+0], zero);
		_mm_stream_ps(surface_normals[n_index+1], zero);
		n_index += 2;
	}
		
	_mm_free(h_map_f);
	
	n_row = ((size_x+1)*2)-1;
	n_index = 1;
	
	for (i = 0; i < size_y; i++)
	{
#ifdef		UNROLL4
		v9 = _mm_load_ps(surface_normals[n_index]);
		v18 = _mm_load_ps(surface_normals[n_index+n_row]);
		for (j = 0; j < size_x; j += 4)
		{			
			v1 = v9;
			v2 = _mm_load_ps(surface_normals[n_index+1]);
			v3 = _mm_load_ps(surface_normals[n_index+2]);			
			v4 = _mm_load_ps(surface_normals[n_index+3]);
			v5 = _mm_load_ps(surface_normals[n_index+4]);			
			v6 = _mm_load_ps(surface_normals[n_index+5]);
			v7 = _mm_load_ps(surface_normals[n_index+6]);
			v8 = _mm_load_ps(surface_normals[n_index+7]);
			v9 = _mm_load_ps(surface_normals[n_index+8]);
			
			v10 = v18;
			v11 = _mm_load_ps(surface_normals[n_index+n_row+1]);
			v12 = _mm_load_ps(surface_normals[n_index+n_row+2]);			
			v13 = _mm_load_ps(surface_normals[n_index+n_row+3]);
			v14 = _mm_load_ps(surface_normals[n_index+n_row+4]);			
			v15 = _mm_load_ps(surface_normals[n_index+n_row+5]);
			v16 = _mm_load_ps(surface_normals[n_index+n_row+6]);
			v17 = _mm_load_ps(surface_normals[n_index+n_row+7]);
			v18 = _mm_load_ps(surface_normals[n_index+n_row+8]);

			t1 = _mm_add_ps(v1, v2);
			t1 = _mm_add_ps(t1, v3);
			t1 = _mm_add_ps(t1, v10);
			t1 = _mm_add_ps(t1, v11);
			t1 = _mm_add_ps(t1, v12);

			t2 = _mm_add_ps(v3, v4);
			t2 = _mm_add_ps(t2, v5);
			t2 = _mm_add_ps(t2, v12);
			t2 = _mm_add_ps(t2, v13);
			t2 = _mm_add_ps(t2, v14);

			t3 = _mm_add_ps(v5, v6);
			t3 = _mm_add_ps(t3, v7);
			t3 = _mm_add_ps(t3, v14);
			t3 = _mm_add_ps(t3, v15);
			t3 = _mm_add_ps(t3, v16);

			t4 = _mm_add_ps(v7, v8);
			t4 = _mm_add_ps(t4, v9);
			t4 = _mm_add_ps(t4, v16);
			t4 = _mm_add_ps(t4, v17);
			t4 = _mm_add_ps(t4, v18);
			
			h1 = _mm_mul_ps(t1, t1);
			h2 = _mm_mul_ps(t2, t2);
			h3 = _mm_mul_ps(t3, t3);
			h4 = _mm_mul_ps(t4, t4);
#ifdef		USE_SSE3
			v1 = _mm_hadd_ps(h1, h2);
			v2 = _mm_hadd_ps(h3, h4);
			h1 = _mm_hadd_ps(v1, v2);
#else
			v1 = _mm_unpacklo_ps(h1, h2);	// v1 = {h1.x, h2.x, t1.y, h2.y}
			v2 = _mm_unpackhi_ps(h1, h2);	// v2 = {h1.z, h2.z, t1.w h2.w}
			v3 = _mm_unpacklo_ps(h3, h4);	// v3 = {h3.x, h4.x, h3.y, h4.y}
			v4 = _mm_unpackhi_ps(h3, h4);	// v4 = {h3.z, h4.z, h3.w, h4.w}
			
			h1 = _mm_movelh_ps(v1, v3);
			h2 = _mm_movehl_ps(v3, v1);
			h3 = _mm_movelh_ps(v2, v4);
			h4 = _mm_movehl_ps(v4, v2);

			h1 = _mm_add_ps(h1, h2);
			h1 = _mm_add_ps(h1, h3);
			h1 = _mm_add_ps(h1, h4);
#endif
			
			h5 = _mm_rsqrt_ps(h1);	// h5 = [1/sqrt(h1.x), 1/sqrt(h1.y), 1/sqrt(h1.z), 1/sqrt(h1.w)]

#ifdef		USE_SSE2
			h1 = (__m128)_mm_shuffle_epi32(h5, _MM_SHUFFLE(0, 0, 0, 0));
			h2 = (__m128)_mm_shuffle_epi32(h5, _MM_SHUFFLE(1, 1, 1, 1));
			h3 = (__m128)_mm_shuffle_epi32(h5, _MM_SHUFFLE(2, 2, 2, 2));
			h4 = (__m128)_mm_shuffle_epi32(h5, _MM_SHUFFLE(3, 3, 3, 3));
#else
			h1 = _mm_shuffle_ps(h5, h5, _MM_SHUFFLE(0, 0, 0, 0));
			h2 = _mm_shuffle_ps(h5, h5, _MM_SHUFFLE(1, 1, 1, 1));
			h3 = _mm_shuffle_ps(h5, h5, _MM_SHUFFLE(2, 2, 2, 2));
			h4 = _mm_shuffle_ps(h5, h5, _MM_SHUFFLE(3, 3, 3, 3));
#endif

			t1 = _mm_mul_ps(t1, h1);
			t2 = _mm_mul_ps(t2, h2);
			t3 = _mm_mul_ps(t3, h3);
			t4 = _mm_mul_ps(t4, h4);

			t1 = _mm_add_ps(t1, one);
			t1 = _mm_mul_ps(t1, half);
			t2 = _mm_add_ps(t2, one);
			t2 = _mm_mul_ps(t2, half);
			t3 = _mm_add_ps(t3, one);
			t3 = _mm_mul_ps(t3, half);
			t4 = _mm_add_ps(t4, one);
			t4 = _mm_mul_ps(t4, half);
	
			_mm_stream_ps(normal_map[i*size_x+j+0], t1);
			_mm_stream_ps(normal_map[i*size_x+j+1], t2);
			_mm_stream_ps(normal_map[i*size_x+j+2], t3);
			_mm_stream_ps(normal_map[i*size_x+j+3], t4);
			
			n_index += 8;
		}
		n_index += 2;
#else		
		v3 = _mm_load_ps(surface_normals[n_index]);
		v6 = _mm_load_ps(surface_normals[n_index+n_row]);
		for (j = 0; j < size_x; j++)
		{
			v1 = v3;
			v2 = _mm_load_ps(surface_normals[n_index+1]);
			v3 = _mm_load_ps(surface_normals[n_index+2]);
			v4 = v6;
			v5 = _mm_load_ps(surface_normals[n_index+n_row+1]);
			v6 = _mm_load_ps(surface_normals[n_index+n_row+2]);

			v1 = _mm_add_ps(v1, v2);
			v1 = _mm_add_ps(v1, v3);
			v1 = _mm_add_ps(v1, v4);
			v1 = _mm_add_ps(v1, v5);
			v1 = _mm_add_ps(v1, v6);

			t1 = _mm_mul_ps(v1, v1);
#ifdef		USE_SSE3
			t1 = _mm_hadd_ps(t1, t1);
			t1 = _mm_hadd_ps(t1, t1);
#else
			t2 = _mm_movehl_ps(t2, t1);
#ifdef		USE_SSE2
			t3 = (__m128)_mm_shuffle_epi32(t1, _MM_SHUFFLE(1, 1, 1, 1));
#else
			t3 = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));
#endif			
			t1 = _mm_add_ss(t1, t2);
			t1 = _mm_add_ss(t1, t3);
#endif			
			t1 = _mm_rsqrt_ss(t1);					// t1 = [1/sqrt(t1.x), 0.0f, 0.0f, 0.0f]
			t1 = _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));	// t1 = [t1.x, t1.x, t1.x, t1.x]
			v1 = _mm_mul_ps(v1, t1);

			v1 = _mm_add_ps(v1, one);
			v1 = _mm_mul_ps(v1, half);

			_mm_stream_ps(normal_map[i*size_x+j], v1);
			
			n_index += 2;
		}
		n_index += 2;
#endif		
	}
	_mm_free(surface_normals);	
	build_normal_texures(normal_map, size_x, size_y);	
	_mm_free(normal_map);
}

/*!
 * \ingroup 	display_utils
 * \brief 	Calculates the current normal map of the terrain height map.
 *
 * Converts the signed short terrain height map to a float terrain height map and calls
 * calc_normal_map_float_sse.
 * \param 	h_map The terrain height map. Address must be 16 Byte aligned for SSE2.
 * \param 	size_x The size of the terrain height map in x direction. Must be a multiple 
 * 		of four for SSE and a multiple of eight for SSE2.
 * \param 	size_y The size of the terrain height map in y direction.
 * \param	h_scale The scale of the terrain height (z direction).
 *  
 * \callgraph
 */
static __inline__ void calc_normal_map_sse(unsigned short* h_map, unsigned int size_x, 
		unsigned int size_y, float h_scale)
{
	unsigned int i, j, index;
	float* h_map_f;
	__m128 vf1, vf2, v_scale;
#ifdef 		USE_SSE2
	__m128i vi0, vi1, vi2, zero;
	
	zero = (__m128i)_mm_set1_ps(0.0f);
#else
	__m64 vi1, vi2;
#endif
	
	v_scale = _mm_set1_ps(h_scale);
	h_map_f = (float*)_mm_malloc(size_x*size_y*sizeof(__m128), sizeof(__m128));	
	index = 0;
	
	for (i = 0; i < size_y; i++)
	{
		for (j = 0; j < size_x; j += 8)
		{
#ifdef 		USE_SSE2
			vi0 = (__m128i)_mm_load_ps((float*)&h_map[index]);
			vi1 = _mm_unpacklo_epi16(vi0, zero);
			vi2 = _mm_unpackhi_epi16(vi0, zero);
			vf1 = _mm_cvtepi32_ps(vi1);
			vf2 = _mm_cvtepi32_ps(vi2);
#else
			vi1 = *((__m64*)&h_map[index]);
			vi2 = *((__m64*)&h_map[index+4]);
			vf1 = _mm_cvtpu16_ps(vi1);
			vf2 = _mm_cvtpu16_ps(vi2);
#endif
			vf1 = _mm_mul_ps(vf1, v_scale);
			vf2 = _mm_mul_ps(vf2, v_scale);
			_mm_stream_ps(&h_map_f[index], vf1);
			_mm_stream_ps(&h_map_f[index+4], vf2);
			index += 8;
		}
	}
#ifndef		USE_SSE2
	_mm_empty();
#endif
	calc_normal_map_float_sse(h_map_f, size_x, size_y);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
#endif
