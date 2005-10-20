
__inline__ int min2i (int x, int y)
{
	return (x <= y)? x : y;
}

__inline__ int max2i (int x, int y)
{
	return (x >= y)? x : y;
}

__inline__ unsigned min2u (unsigned x, unsigned y)
{
	return (x <= y)? x : y;
}

__inline__ unsigned max2u (unsigned x, unsigned y)
{
	return (x >= y)? x : y;
}

__inline__ float min2f (float x, float y)
{
	return (x <= y)? x : y;
}

__inline__ float max2f (float x, float y)
{
	return (x >= y)? x : y;
}
