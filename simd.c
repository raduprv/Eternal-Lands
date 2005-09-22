/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	Intel SIMD instruction (SSE, SSE2 and SSE3) check.
 */
#ifdef	USE_SSE
#include "cpuid.h"


/*!
 * The use_sse stores if we can use sse.
 */
unsigned int use_sse;

void init_sse()
{
	unsigned int simd, sse;
#ifdef	SSE3
	simd = 3;
#else
#ifdef	SSE2
	simd = 2
#else
	simd = 1;
#endif
#endif
	if (simd >= get_sse()) use_sse = 1;
	else use_sse = 0;
}
#endif
