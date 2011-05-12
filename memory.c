/****************************************************************************
 *            memory.c
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "memory.h"
#include "elloggingwrapper.h"
#ifdef	USE_SIMD
#include "mm_malloc.h"
#endif	/* SIMD */

void* malloc_aligned(const Uint64 size, const Uint64 alignment)
{
	void* result;

#ifdef	USE_SIMD
	result = _mm_malloc(size, alignment);
#else	/* USE_SIMD */
	result = malloc(size);
#endif	/* USE_SIMD */
	LOG_DEBUG_VERBOSE("size: %d, alignment: %d, memory: %p", size, alignment, result);

	return result;
}

void free_aligned(void* memory)
{
	LOG_DEBUG_VERBOSE("memory: %p", memory);
#ifdef	USE_SIMD
	_mm_free(memory);
#else	/* USE_SIMD */
	free(memory);
#endif	/* USE_SIMD */
}

