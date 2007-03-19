#ifdef EYE_CANDY

#ifndef DONT_OPTIMIZE_H

// P R O T O T Y P E S ////////////////////////////////////////////////////////

#ifdef WINDOWS
__declspec(noinline) float fastsqrt(float f);
__declspec(noinline) float invsqrt(float f);
#else	
__attribute__ ((noinline)) float fastsqrt(float f);
__attribute__ ((noinline)) float invsqrt(float f);
#endif

///////////////////////////////////////////////////////////////////////////////

#endif	// defined DONT_OPTIMIZE_H

#endif	// #ifdef EYE_CANDY
