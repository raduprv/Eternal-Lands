#ifndef __file_cache_H__
#define __file_cache_H__

typedef struct
{
	void	*cache_item;	// pointer to the item we are caching
	Uint32	size;			// size of item
	Uint32	access_time;	// last time used
	Uint32	access_count;	// number of usages since last checkpoint
	Uint8	*name;			// original source or name, NOTE: this is NOT free()'d and allows dups!
}cache_item_struct;

typedef struct
{
	cache_item_struct	**cached_items;
	cache_item_struct	*recent_item;
	Sint32	num_items;		// the number of active items in the list
	Sint32	max_item;		// the highest slot used
	Sint32	first_unused;	// the lowest possible unused slow (might be in use!!)
	Sint32	num_allocated;	// the allocated space for the list
	Uint32	LRU_time;		// last time LRU processing done
	Uint32	total_size;		// total size currently allocated
	Uint32	time_limit;		// limit on LRU time before forcing a scan
	Uint32	size_limit;		// limit on size before forcing a scan
	void	(*free_item)();	// routine to call to free an item
	Uint32	(*compact_item)();	// routine to call to reduce memory usage without freeing
}cache_struct;

typedef struct
{
	int texture_id;
    char file_name[128];
	cache_item_struct	*cache_ptr;
	unsigned char alpha;
}texture_cache_struct;

extern texture_cache_struct texture_cache[1000];

#define	MAX_CACHE_SYSTEM	32
extern cache_struct	*cache_system;
extern cache_struct	*cache_md2;
extern cache_struct	*cache_e3d;
extern cache_struct	*cache_texture;

//proto
extern cache_struct *cache_system;
void cache_system_init(Uint32 max_items);
void cache_system_shutdown();
void cache_system_maint();
Uint32 cache_system_clean();
Uint32 cache_system_compact();
void cache_dump_sizes(cache_struct *cache);

cache_struct *cache_init(Uint32 max_items, void (*free_item)());
void cache_set_free(cache_struct *cache, void (*free_item)());
void cache_set_compact(cache_struct *cache, Uint32 (*compact_item)());
void cache_set_time_limit(cache_struct *cache, Uint32 time_limit);
void cache_set_size_limit(cache_struct *cache, Uint32 size_limit);
Uint32 cache_clean(cache_struct *cache);
Uint32 cache_compact(cache_struct *cache);
void cache_delete(cache_struct *cache);
void cache_clear_counter(cache_struct *cache);

cache_item_struct *cache_add_item(cache_struct *cache, Uint8 *name, void *item, Uint32 size);
void cache_set_name(cache_struct *cache, Uint8 *name, void *item);
void cache_set_size(cache_struct *cache, Uint32 size, void *item);
void cache_adj_size(cache_struct *cache, Uint32 size, void *item);
void cache_use(cache_struct *cache, cache_item_struct *item);
void cache_use_item(cache_struct *cache, const void *item_data);
cache_item_struct *cache_find(cache_struct *cache, const Uint8 *name);
cache_item_struct *cache_find_ptr(cache_struct *cache, const void *item);
void *cache_find_item(cache_struct *cache, const Uint8 *name);
void cache_remove(cache_struct *cache, cache_item_struct *item);
void cache_remove_item(cache_struct *cache, const Uint8 *name);
void cache_remove_all(cache_struct *cache);
void cache_remove_unused(cache_struct *cache);

#endif
