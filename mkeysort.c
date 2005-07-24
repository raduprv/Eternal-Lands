#include <stdlib.h>
#include <string.h>
#include "mkeysort.h"
#include "global.h"

#ifndef WINDOWS //This function is provided by windows.h
inline int min(int x, int y) {
	return x <= y ? x : y;
}
#endif //WINDOWS

void mkeysort_recursive(char * A[], int r, int h) {
	if (r <= 1) {
		if (r == 1) {
			if (strcmp(A[0]+h,A[1]+h) > 0) {
				char * t = *A;
				*A = A[1];
				A[1] = t;
			}
		}
	} else {
		int a=0, b=0, c=r, d=r, i;
		
		do {
			int nmove;
			char m = A[r >> 1][h];
			do {
				while (b <= c && A[b][h] <= m) {
					if (A[b][h] == m) {
						char * t = A[a];
						A[a] = A[b];
						A[b] = t;
						a++;
					}
					b++;
				}
				while (b <= c && A[c][h] >= m) {
					if (A[c][h] == m) {
						char * t = A[d];
						A[d] = A[c];
						A[c] = t;
						d--;
					}
					c--;
				}
				if (b < c) {
					char * t = A[b];
					A[b] = A[c];
					A[c] = t;
					b++; c--;
				}
			} while (b <= c);
			nmove = min(a,b-a);
			for (i = 0; i < nmove; i++) {
				 char * t = A[i];
				 A[i] = A[c-i];
				 A[c-i] = t;
			}
			nmove = min(r-d,d-c);
			for (i = 0; i < nmove; i++) {
				 char * t = A[b+i];
				 A[b+i] = A[r-i];
				 A[r-i] = t;
			}
			mkeysort_recursive(A+c-a+1,a+r-d-1,h+1);
			mkeysort_recursive(A+b+r-d,d-b,h);
			
			r = c-a; c = d = r; a = b = 0;
		} while (r > 1);
		
		if (r == 1) {
			if (strcmp(A[0]+h,A[1]+h) > 0) {
				char * t = *A;
				*A = A[1];
				A[1] = t;
			}
		}

	}
}

void mkeysort(char * A[], int n) {
	mkeysort_recursive(A,--n,0);
}

int mkeyfind(char * A[], int n, const char * key) {
	int l=0, r=n-1, lo=0, ro=0;
	while (l <= r) {
		int o = min(lo,ro), m = (l + r) >> 1;
		char * Am = A[m];
		char c = Am[o], k = key[o];
		if (k < c) {
			r = m - 1;
		} else if (k > c) {
			l = m + 1;
		} else {
			if (k) {
				for(;;) {
					++o;
					c =  Am[o]; k = key[o];
					if (k < c) {
						r = m - 1;
						ro = o;
						break;
					} else if (k > c) {
						l = m + 1;
						lo = o;
						break;
					} else {
						if (k) {
							continue;
						} else {
							return m;
						}
					}
				}
			} else {
				return m;
			}
		}
	}
	return -1;
}
