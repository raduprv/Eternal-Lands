#ifndef	_OPTIMIZER_HPP_
#define	_OPTIMIZER_HPP_

#ifdef	USE_BOOST
#include <boost/shared_array.hpp>
#endif	/* USE_BOOST */

#include "platform.h"
#include <SDL.h>

#ifdef	USE_BOOST
float calculate_average_cache_miss_ratio(const boost::shared_array<Uint32> &indices,
	const Uint32 offset, const Uint32 count, const Uint32 cache_size);
bool optimize_vertex_cache_order(boost::shared_array<Uint32> &tri_indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size);
#else	/* USE_BOOST */
float calculate_average_cache_miss_ratio(const Uint32* indices, const Uint32 offset,
	const Uint32 count, const Uint32 cache_size);
bool optimize_vertex_cache_order(Uint32* tri_indices, const Uint32 offset, const Uint32 count,
	const Uint32 cache_size);
#endif	/* USE_BOOST */

#endif	/* _OPTIMIZER_HPP_ */
