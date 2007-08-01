#include <stdlib.h>
#include "symbol_table.h"
#include "sort.h"

const char * st_get(void * pdata, int i) {
	return ((st_entry *)pdata)[i].symbol;
}

void st_swap(void * pdata, int i, int j) {
	st_entry * const E = pdata;
	st_entry tmp = E[i];
	E[i] = E[j];
	E[j] = tmp;
}

void st_put(void * psrc, int i, void * pdst, int j) {
	st_entry * const E = psrc;
	st_entry * const S = pdst;
	S[j] = E[i];
}

symbol_table * st_create(int capacity) {
	symbol_table * st = (symbol_table *) malloc(sizeof(symbol_table));

	st->added = 0;
	st->capacity = capacity;
	st->committed = 0;
	st->entries = (st_entry *) calloc(capacity, sizeof(symbol_table));
	st->shadow  = (st_entry *) calloc(capacity, sizeof(symbol_table));

	return st;
}

void st_addnum(symbol_table * st, const char * symbol, int data) {
	st_entry * const e = &st->entries[st->committed + st->added];
	e->symbol = symbol;
	e->data.num = data;
	st->added++;
}

void st_addptr(symbol_table * st, const char * symbol, void * data) {
	st_entry * const e = &st->entries[st->committed + st->added];
	e->symbol = symbol;
	e->data.ptr = data;
	st->added++;
}

void st_commit(symbol_table * st) {
	st_entry * const entries = st->entries;
	gen_mkeysort(entries + st->committed, st_get, st_swap, st->added);
	gen_mkeymerge(entries, st->shadow, st_get, st_put, st->committed, st->added);
	st->entries = st->shadow;
	st->shadow = entries;
	st->committed += st->added;
	st->added = 0;
}

st_data * st_lookup(symbol_table * st, const char * symbol) {
	int index = gen_mkeyfind(st->entries, st_get, st->committed, symbol);
	return (index < 0)? NULL : &st->entries[index].data;
}

void st_destroy(symbol_table * st) {
	free(st->entries);
	free(st->shadow);
}

void st_destroyExt(symbol_table * st, void (* freedata)(void *)) {
	int i, n = st->committed + st->added;
	for (i = 0; i < n; i++) {
		freedata(st->entries[i].data.ptr);
	}
	st_destroy(st);
}

