/****************************************************************************
 *            memory.h
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef UUID_b98dd9af_3f58_462d_9b5c_30d5ace6b0b9
#define UUID_b98dd9af_3f58_462d_9b5c_30d5ace6b0b9

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup misc
 * @brief Allocs memory aligned.
 *
 * Allocs the memory aligned. Some simd instructions need aligned memory.
 * @param size The size of the memory to alloc.
 * @param alignment The alignment to use for the memory.
 * @callgraph
 */
void* malloc_aligned(const Uint64 size, const Uint64 alignment);

/**
 * @ingroup misc
 * @brief Frees the memory.
 *
 * Frees the memory that was allocated with malloc_aligned.
 * @param memory The memory to free.
 * @callgraph
 */
void free_aligned(void* memory);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	/* UUID_b98dd9af_3f58_462d_9b5c_30d5ace6b0b9 */

