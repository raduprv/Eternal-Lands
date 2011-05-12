/****************************************************************************
 *            memory.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "memory.h"
#ifdef	SIMD
#include "mm_malloc.h"
#endif	/* SIMD */

void* malloc_aligned(const Uint64 size, const Uint64 alignment)
{
#ifdef	SIMD
	return _mm_malloc(size, alignment);
#else	/* SIMD */
	return malloc(size);
#endif	/* SIMD */
}

void free_aligned(void* memory)
{
#ifdef	SIMD
	_mm_free(memory);
#else	/* SIMD */
	free(memory);
#endif	/* SIMD */
}

