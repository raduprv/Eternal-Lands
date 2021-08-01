#ifndef	_OPTIMIZER_HPP_
#define	_OPTIMIZER_HPP_

#include "platform.h"
#include <SDL.h>

float calculate_average_cache_miss_ratio(const Uint32* indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size);
bool optimize_vertex_cache_order(Uint32* tri_indices, const Uint32 offset, const Uint32 count,
	const Uint32 cache_size);

#endif	/* _OPTIMIZER_HPP_ */
