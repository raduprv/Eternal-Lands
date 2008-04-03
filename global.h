/*!
 * \file
 * \ingroup init
 * \brief global include file
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern Uint32 cur_time, last_time; /*!< timestamps to check whether we need to resync */

#ifdef SKY_FPV_OPTIONAL
#define font_scale 10.0f
#endif // SKY_FPV_OPTIONAL

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __GLOBAL_H__ */
