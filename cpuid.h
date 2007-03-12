/*!
 * \file
 * \ingroup 	misc_utils
 * \brief 	CPUID instruction.
 */
#ifndef	CPUID_H
#define	CPUID_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The flag of SSE (Streaming SIMD Extensions).
 */
#define CPUID_FLAG_SSE          0x2000000
/*!
 * The flag of SSE2 (Streaming SIMD Extensions - #2).
 */
#define CPUID_FLAG_SSE2         0x4000000
/*!
 * The flag of SSE3 (Streaming SIMD Extensions - #3).
 */
#define CPUID_FLAG_SSE3         0x0000001

/*!
 * \brief The CPUID instructions.
 *
 * The CPUID instruction is used to detect SSE, SSE2 and SSE3.
 * \param	code The code for the CPUID instruction.
 * \param	a The result of register eax.
 * \param	b The result of register ebx.
 * \param	c The result of register ecx.
 * \param	d The result of register edx.
 *
 * \callgraph
 */
static __inline__ void cpuid(int code, unsigned int *a, unsigned int *b,
		unsigned int *c, unsigned int *d) 
{
	asm(	"cpuid\n\t"
		:"=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d)
		:"a"(code));
}

/*!
 * \brief get SSE support.
 *
 * Returns zero if no SSE was found, one for SSE, two for SSE2 and three for SSE3.
 * \retval	int Returns the type of SSE found.
 *
 * \callgraph
 */
static __inline__ int get_sse()
{
	unsigned int a, b, c, d, ret;
	cpuid(0, &a, &b, &c, &d);
	ret = 0;
	if (a >= 1)
	{
		cpuid(1, &a, &b, &c, &d);
		if (d & CPUID_FLAG_SSE)
		{
			ret++;
			if (d & CPUID_FLAG_SSE2) 
			{
				ret++;
				if (c & CPUID_FLAG_SSE3) ret++;
			}
		}
	}
	return ret;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
