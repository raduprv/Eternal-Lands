#ifdef EYE_CANDY

/*
                                --== NOTICE ==--
            
        This code is very sensitive to optimization.  That is, to say,
compilers trying to optimize it tend to break it.  Due to workarounds, it
now optimizes properly in Linux.  However, there reports that Visual C++,
it does not.  Only if you disable SSE2 for this file in Visual C++ will it
work.  If you still have problems, you can accept a 2-4fold slowdown on
inverse squareroots and enable SLOW_BUT_RELIABLE

*/

//#define SLOW_BUT_RELIABLE

// F U N C T I O N S //////////////////////////////////////////////////////////

#ifdef WINDOWS
__declspec(noinline) float invsqrt_workaround(int i)
#else
__attribute__ ((noinline)) float invsqrt_workaround(int i)
#endif
{
  return *(float*)((void*)&i);
}

#ifdef WINDOWS
__declspec(noinline) float invsqrt(float f)	// The famous Quake3 inverse square root function.  About 4x faster in my benchmarks!
#else
__attribute__ ((noinline)) float invsqrt(float f)	// The famous Quake3 inverse square root function.  About 4x faster in my benchmarks!
#endif
{
#ifdef SLOW_BUT_RELIABLE
  return 1.0 / sqrt(f);
#else
  float half = 0.5f * f;
  int i = *(int*)((void*)&f);
  i = 0x5f3759df - (i >> 1);
  f = invsqrt_workaround(i);
  f = f * (1.5f - half * f * f);
  return f;
#endif
}

///////////////////////////////////////////////////////////////////////////////

#endif
