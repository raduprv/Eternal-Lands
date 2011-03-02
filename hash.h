#ifndef __HASH__
#define __HASH__

#include <SDL.h>

typedef struct _hash_entry{
	void *key;
	void *item;
	struct _hash_entry *next;
} hash_entry;

typedef struct _hash_table{
	int size;
	int items;
	hash_entry **store;

	hash_entry *cur;
	int where;
	
	unsigned long int (*hash_fun)(void *);
	int (*key_cmp)(void *, void *);
	void (*free_fun)(void *);
} hash_table;


hash_table *create_hash_table(int size, 
			     unsigned long int (*hashfn)(void *), 
			     int (*keyfn)(void *, void*),
			     void (*freefn)(void *)
);

int destroy_hash_table(hash_table *table);

hash_entry *hash_get(hash_table *table, void* key);
int hash_add(hash_table *table, void* key, void *item);
int hash_delete(hash_table *table, void *key);
void hash_start_iterator(hash_table *table);
hash_entry *hash_get_next(hash_table *table);


//HASH & KEY_CMP
unsigned long int hash_fn_int(void *key);
int cmp_fn_int(void *key1, void *key2);

unsigned long int hash_fn_str(void *key);
int cmp_fn_str(void *key1, void *key2);

Uint32 mem_hash(const void* str, const Uint32 len);

#endif
