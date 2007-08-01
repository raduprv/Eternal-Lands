#include <string.h>
#include "sort.h"
#include "misc.h"

void gen_mkeysort_recursive(void * pdata, gen_mkey_retrieve_func get, gen_mkey_swap_func 
		swap, int l, int r, int h) {
	if (r <= l+1) {
		if (r == l+1) {
			if (strcmp(get(pdata,l) + h, get(pdata,r) + h) > 0) swap(pdata,l,r);
		}
	} else {
		int a=l, b=l, c=r, d=r, i;
		
		do {
			int nmove;
			char m = get(pdata, (l+r) >> 1)[h];
			do {
				while (b <= c && get(pdata,b)[h] <= m) {
					if (get(pdata,b)[h] == m) {
						swap(pdata,a,b);
						a++;
					}
					b++;
				}
				while (b <= c && get(pdata,c)[h] >= m) {
					if (get(pdata,c)[h] == m) {
						swap(pdata,d,c);
						d--;
					}
					c--;
				}
				if (b < c) {
					swap(pdata,b,c);
					b++; c--;
				}
			} while (b <= c);
			nmove = min2i(a-l,b-a);
			for (i = 0; i < nmove; i++) {
				swap(pdata,l+i,c-i);
			}
			nmove = min2i(r-d,d-c);
			for (i = 0; i < nmove; i++) {
				swap(pdata,b+i,r-i);
			}
			gen_mkeysort_recursive(pdata,get,swap,r+b-d,r,h);
			gen_mkeysort_recursive(pdata,get,swap,l+b-a,r+c-d,h+1);
			
			r = l+c-a; c = d = r; a = b = l;
		} while (r > l+1);
		
		if (r == l+1) {
			if (strcmp(get(pdata,l) + h, get(pdata,r) + h) > 0) {
				swap(pdata,l,r);
			}
		}

	}
}

void gen_mkeysort(void * pdata, gen_mkey_retrieve_func get, gen_mkey_swap_func
		swap, int n) {
	gen_mkeysort_recursive(pdata, get, swap, 0, --n, 0);
}

int gen_mkeyfind(void * pdata, gen_mkey_retrieve_func get, int n, const char * key) {
	int l=0, r=n-1, lo=0, ro=0;
	while (l <= r) {
		int o = min2i(lo,ro), m = (l + r) >> 1;
		const char * Am = get(pdata,m);
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

void gen_mkeymerge(void * psrc, void * pdst, gen_mkey_retrieve_func get,
		gen_mkey_copy_func put, int n, int m) {
	int a = 0, b = n, d = 0;
	/* merge until at least one of the parts is completely inserted */
	while ((n > 0) && (m > 0)) {
		if (strcmp(get(psrc,a), get(psrc,b)) <= 0) {
			put(psrc,a,pdst,d);
			a++; d++; n--;
		} else {
			put(psrc,b,pdst,d);
			b++; d++; m--;
		}
	}
	/* if a has not been completely inserted, do that */
	while (n > 0) {
		put(psrc,a,pdst,d);
		a++; d++; n--;
	}
	while (m > 0) {
		put(psrc,b,pdst,d);
		b++; d++; m--;
	}
}
