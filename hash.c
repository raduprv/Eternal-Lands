#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


hash_table *create_hash_table(int size, 
			     unsigned long int (*hashfn)(void *), 
			     int (*keyfn)(void *, void*),
			     void (*freefn)(void *)){
	hash_table *new_table;
	
	new_table=(hash_table*)calloc(1,sizeof(hash_table));
	if(!new_table) return NULL;

	new_table->store=(hash_entry**)calloc(size,sizeof(hash_entry*));
	if(!new_table->store) { free(new_table); return NULL;}
	
	new_table->size=size;
	new_table->items=0;
	new_table->hash_fun=hashfn;
	new_table->key_cmp=keyfn;
	new_table->free_fun=freefn;

	return new_table;
}

int destroy_hash_table(hash_table *table){
	int i;
	hash_entry *he=NULL, *ht=NULL;

	if(table){
		if(table->store) {
			if (table->free_fun)
				for(i=0;i<table->size;i++){
					he=table->store[i];
					while(he){
						ht=he->next;
						table->free_fun(he->item);
						free(he);
						he=ht;
					}
				}
			free(table->store);
		}
		free(table);
		return 1;
	}
	return 0;
}

hash_entry *hash_get(hash_table *table, void* key){
	unsigned int pos;
	hash_entry *he=NULL;

	if(!table||!table->hash_fun||!table->key_cmp) return NULL;
	
	pos=(table->hash_fun(key))%table->size;

	he=table->store[pos];
	while(he){
		if(table->key_cmp(key,he->key)!=0) break;
		he=he->next;
	}
	return he;
}

int hash_add(hash_table *table, void* key, void *item){
	unsigned int pos;
	hash_entry *he=NULL;

	if(!table||!table->hash_fun) return 0;
	
	pos=table->hash_fun(key)%table->size;
	
	he=(hash_entry*)calloc(1,sizeof(hash_entry));
	if(!he) return 0;
	he->key=key;
	he->item=item;
	he->next=table->store[pos];
	table->store[pos]=he;
	table->items++;
	return 1;
}

int hash_delete(hash_table *table, void *key){

	hash_entry *he,*ht,*hp;
	int del=0,pos;

	if(!table||!table->hash_fun||!table->key_cmp) return del;
	
	pos=table->hash_fun(key)%table->size;
	he=table->store[pos];
	ht=NULL;
	while(he){
		hp=he->next;
		if(table->key_cmp(key,he->key)!=0) {
			if (ht) ht->next=he->next;
			else table->store[pos]=he->next;
			if (table->free_fun) 
				table->free_fun(he->item);			
			free(he);
			del++;
			table->items--;	
		} else ht=he;
		he=hp;
	}
	return del;
}


void hash_start_iterator(hash_table *table){ 
	if(!table) return;
	table->cur=table->store[0];
	table->where=-1;
}

hash_entry *hash_get_next(hash_table *table){

	if(!table||table->where>=table->size) {
		return NULL;
	}

	if(table->where==-1&&table->cur&&table->cur==table->store[0]) {
		table->where++;
		return table->store[0];
	}

	if(table->cur&&table->cur->next){
			return (table->cur = table->cur->next);
			}

	table->cur=NULL;
	table->where++;
	while(table->where<table->size){
		if(table->store[table->where]){
			table->cur=table->store[table->where];
			break;
		}
		table->where++;	  
	}
	return table->cur;
}


//HASH AND COMPARE FNs
unsigned long int hash_fn_int(void *key){
	return (unsigned long int) key;
}
int cmp_fn_int(void *key1, void *key2){
	return key1==key2;
}

unsigned long int hash_fn_str(void *key)
{
	unsigned long int hash = 5381;
	char c;
	char *k=(char*)key;

	while ( (c=*k++) )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	hash--;

	return hash;
}

int cmp_fn_str(void *key1, void *key2){
	return !strcmp((char*)key1,(char*)key2);
}

Uint32 mem_hash(const void* str, const Uint32 len)
{
	Uint32 hash, i;

	hash = 2166136261u;

	for (i = 0; i < len; i++)
	{
		hash = hash * 1607;
		hash = hash ^ ((Uint8*)str)[i];
	}

	return hash;
}

