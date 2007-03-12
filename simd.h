#ifdef	TERRAIN
/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	Intel SIMD instruction (SSE, SSE2 and SSE3) check.
 */
#ifndef	SIMD_H
#define SIMD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	X86_64
#ifndef	SSE2
#define	SSE2
#endif
#endif

#ifdef	SSE3
#ifndef SSE2
#define	SSE2
#endif
#endif

#ifdef	SSE2
#ifndef	SSE
#define	SSE
#endif
#endif

#ifdef USE_SSE
#define SIMD_MALLOC(size) _mm_malloc((size), sizeof(__m128))
#define SIMD_FREE(ptr) _mm_free((ptr))
/*!
 * The use_sse stores if we can use SSE.
 */
extern unsigned int use_sse;
/*!
 * \brief Checks if we can use SSE and sets use_sse.
 *
 * Checks if we can use SSE and sets use_sse.
 *
 * \callgraph
 * 
 */
extern void init_sse();
#else
#define SIMD_MALLOC(size) malloc((size))
#define SIMD_FREE(ptr) free((ptr))

#endif

#define use_low_mem 0

#ifdef __cplusplus
} // extern "C"
#endif

#endif
#endif
