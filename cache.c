#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "2d_objects.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "global.h"
#include "text.h"
#include "textures.h"
#include "translate.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void cache_system_shutdown();
 * void cache_clear_counter(cache_struct*);
 * void cache_set_size(cache_struct*, Uint32, void*);
 * void cache_use_item(cache_struct*, const void*);
 * void cache_remove_item(cache_struct*, cosnt Uint8*);
 * void cache_remove_unused(cache_struct*);
 */

cache_struct *cache_system = NULL;
cache_struct *cache_e3d = NULL;

#ifndef	NEW_TEXTURES
texture_cache_struct texture_cache[TEXTURE_CACHE_MAX];
#endif	/* NEW_TEXTURES */

static Uint32 cache_system_clean();
static Uint32 cache_system_compact();
static Uint32 cache_clean(cache_struct *cache);
static Uint32 cache_compact(cache_struct *cache);
static cache_item_struct *cache_find_ptr(cache_struct *cache, const void *item);
static void cache_remove(cache_struct *cache, cache_item_struct *item);
static void cache_remove_all(cache_struct *cache);

#ifdef FASTER_MAP_LOAD
// Compare two cache items by name. NULL pointer always go beyond "real" items.
static int cache_item_cmp(const void* i, const void *j)
{
	const cache_item_struct *item = *((const cache_item_struct **)i);
	const cache_item_struct *jtem = *((const cache_item_struct **)j);
	if (!item) return jtem ? 1 : 0;
	if (!jtem) return -1;
	return strcmp(item->name, jtem->name);
}

// Compare string \a str with the name of item \a iptr.
static int cache_item_cmp_str(const void* str, const void *iptr)
{
	const cache_item_struct *item = *((const cache_item_struct **)iptr);
	if (!item) return -1;
	return strcmp(str, item->name);
}
#endif

// top level cache system routines
void cache_system_init(Uint32 max_items)
{
	cache_system = cache_init("", max_items, &cache_delete);
	cache_set_time_limit(cache_system, 1*60*1000);	// once per minute check for processing
}

#ifdef	ELC
void cache_dump_sizes(const cache_struct *cache)
{
	char str[256];
	const cache_item_struct *item;
	Sint32 i;

#ifdef FASTER_MAP_LOAD
	for (i = 0; i < cache->num_items; i++)
#else
	for (i = 0; i < cache->max_item; i++)
#endif
	{
		item = cache->cached_items[i];
		if (item)
		{
			Uint8 scale = ' ';
			Uint32 size = item->size;
			if (size > 100000000)
			{
				size /= 1024*1024;
				scale = 'M';
			}
			else if (size > 100000)
			{
				size /= 1024;
				scale = 'K';
			}
			if(cache==cache_system)
			{
				cache_struct *temp = item->cache_item;
				safe_snprintf(str, sizeof(str), "%s %6d%c - %d: %s (%d %s)",
#ifdef FASTER_MAP_LOAD
					cache_size_str, size, scale, i, item->name, temp->num_items, cache_items_str);
#else
					cache_size_str, size, scale, i, item->name, temp->max_item, cache_items_str);
#endif
			}
			else
				safe_snprintf(str, sizeof(str), "%s %6d%c - %d: %s",
					cache_size_str, size, scale, i, item->name);
			put_colored_text_in_buffer(c_yellow1, CHAT_SERVER, (unsigned char*)str, -1);
#ifdef MAP_EDITOR2
			log_error(str);
#else
			write_to_log(CHAT_SERVER, (unsigned char*)str, strlen(str));
#endif
		}
	}
}
#endif	/* ELC */

void cache_system_maint()
{
	if (!cur_time || !cache_system || !cache_system->time_limit
		|| cache_system->LRU_time+cache_system->time_limit > cur_time)
		return;
	//clean anything we can delete
	cache_system_clean();
	//do an automated memory compaction
	cache_system_compact();
	//actually done already, just forcing it to assist in debugging
	cache_system->LRU_time = cur_time;
}

static Uint32 cache_system_clean()
{
	cache_item_struct *item;
	Sint32 i;
	Uint32 mem_freed = 0;

	if (!cache_system || !cache_system->time_limit
		|| !cache_system->cached_items) return 0;
	// make sure we are in a safe place
#ifdef	ELC
	if ( !get_show_window (game_root_win) ) return 0;
#endif	/* ELC */
#ifdef FASTER_MAP_LOAD
	for (i = 0; i < cache_system->num_items; i++)
#else
	for (i = 0; i < cache_system->max_item; i++)
#endif
	{
		item = cache_system->cached_items[i];
		if (item && item->cache_item)
			mem_freed += cache_clean(item->cache_item);
	}

	//adjust the LRU time-stamp
	cache_system->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

static Uint32 cache_system_compact()
{
	cache_item_struct *item;
	Sint32 i;
	Uint32 mem_freed = 0;

	if (!cache_system || !cache_system->time_limit
		|| !cache_system->cached_items) return 0;

#ifdef FASTER_MAP_LOAD
	for (i = 0; i < cache_system->num_items; i++)
#else
	for (i = 0; i < cache_system->max_item; i++)
#endif
	{
		item = cache_system->cached_items[i];
		if (item && item->cache_item)
			mem_freed += cache_compact(item->cache_item);
	}

	//adjust the LRU time-stamp
	cache_system->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

// individual caches
cache_struct *cache_init(const char* name, Uint32 max_items,
	void (*free_item)())
{
	cache_struct *cache;

	cache = calloc(1, sizeof(cache_struct));
	if(!cache)
		return NULL;	//oops, not enough memory

	cache->cached_items = calloc(max_items, sizeof(cache_item_struct *));
	if (!cache->cached_items)
	{
		free(cache);
		return NULL;	//oops, not enough memory
	}
	cache->recent_item = NULL;
	cache->num_allocated = max_items;
	cache->LRU_time = cur_time;
	cache->time_limit = 0;	// 0 == no time based LRU check
	cache->size_limit = 0;	// 0 == no space based LRU check
	cache->free_item = free_item;
	cache->compact_item = NULL;
	if (cache_system)
	{
		cache_add_item(cache_system, name, cache,
			sizeof(cache_struct) + max_items*sizeof(cache_item_struct *));
	}

	//all done, send the data back
	return cache;
}

void cache_delete(cache_struct *cache)
{
	static int cache_delete_loop_block = 0;

	if (!cache) return;
	if (cache != cache_system)
		cache_adj_size(cache_system, -cache->total_size, cache);
	if (cache->cached_items)
	{
		cache_remove_all(cache);
		free(cache->cached_items);
		cache->cached_items = NULL;	//failsafe
		cache->recent_item = NULL;	//failsafe
	}
	if (cache_system && cache != cache_system && !cache_delete_loop_block)
	{
		cache_delete_loop_block++;
		cache_remove(cache_system, cache_find_ptr(cache_system, cache));
		cache_delete_loop_block--;
	}
	else
	{
		/* Make sure it wasn't free()d by the cache_remove() call */
		free(cache);
	}
}

void cache_set_compact(cache_struct *cache, Uint32 (*compact_item)())
{
	cache->compact_item=compact_item;
}

void cache_set_time_limit(cache_struct *cache, Uint32 time_limit)
{
	cache->time_limit=time_limit;
}

void cache_set_size_limit(cache_struct *cache, Uint32 size_limit)
{
	cache->size_limit=size_limit;
}

void cache_set_free(cache_struct *cache, void (*free_item)())
{
	cache->free_item = free_item;
}

static Uint32 cache_clean(cache_struct *cache)
{
	cache_item_struct *item;
	Sint32 i;
	Uint32 mem_freed = 0;

	if (!cache->cached_items || !cache->time_limit || !cache->free_item)
		return 0;

#ifdef FASTER_MAP_LOAD
	for (i = cache->num_items-1; i >= 0; i--)
#else
	for (i = 0; i < cache->max_item; i++)
#endif
	{
		item = cache->cached_items[i];
		if (item && item->cache_item)
		{
			// decide if this entry needs to be cleaned
			if (item->access_count == 0
				&& item->access_time + cache->time_limit < cur_time)
			{
				cache_remove(cache, item);
				mem_freed += item->size;
			}
			//item->access_count=0;
		}
	}

	//adjust the LRU time-stamp
	cache->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

static Uint32 cache_compact(cache_struct *cache)
{
	cache_item_struct *item;
	Sint32 i;
	Uint32 freed;
	Uint32 mem_freed=0;

	if (!cache->cached_items || !cache->time_limit || !cache->compact_item)
		return 0;

#ifdef FASTER_MAP_LOAD
	for (i = 0; i < cache->num_items; i++)
#else
	for (i = 0; i < cache->max_item; i++)
#endif
	{
		item = cache->cached_items[i];
		if (item && item->cache_item)
		{
			//decide if this entry needs to be cleaned
			if (item->access_count == 0
				&& item->access_time + cache->time_limit < cur_time)
			//if(cache->cached_items[i]->access_time+cache->time_limit < cur_time)
			{
				freed = (*cache->compact_item)(cache->cached_items[i]->cache_item);
				mem_freed += freed;
				cache_adj_size(cache, -freed, cache->cached_items[i]->cache_item);
			}
			else
			{
				cache->cached_items[i]->access_count = 0;	// clear the counter
			}
		}
	}

	//adjust the LRU time-stamp
	cache->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}


#ifndef	USE_INLINE
// detailed items
void cache_use(cache_item_struct *item_ptr)
{
	if (item_ptr)
	{
		item_ptr->access_time = cur_time;
		item_ptr->access_count++;
	}
}
#endif	//USE_INLINE

cache_item_struct *cache_find(cache_struct *cache, const char *name)
{
#ifdef FASTER_MAP_LOAD
	cache_item_struct **item;
#else
	Sint32 i;
#endif

	if (!cache->cached_items)
		return NULL;

	// quick check for the most recent item
	if (cache->recent_item && cache->recent_item->name
		&& strcmp(cache->recent_item->name, name) == 0)
	{
		cache_use(cache->recent_item);
		return cache->recent_item;
	}


#ifdef FASTER_MAP_LOAD
	item = bsearch(name, cache->cached_items, cache->num_items,
		sizeof(cache_item_struct*), cache_item_cmp_str);
	if (!item)
		return NULL;
	cache_use(*item);
	cache->recent_item = *item;
	return *item;
#else
	// not the most recent, then scan the list
	for (i = 0; i < cache->max_item; i++)
	{
		if (cache->cached_items[i] && cache->cached_items[i]->name
			&& strcmp(cache->cached_items[i]->name, name) == 0)
		{
			cache_use(cache->cached_items[i]);
			cache->recent_item = cache->cached_items[i];
			return cache->cached_items[i];
		}
	}

	return NULL;
#endif
}

static cache_item_struct *cache_find_ptr(cache_struct *cache, const void *item)
{
	cache_item_struct *citem;
	Sint32 i;

	if (!cache->cached_items)
		return NULL;

	// quick check for the most recent item
	if (cache->recent_item && cache->recent_item->name
		&& cache->recent_item->cache_item == item)
	{
		cache_use(cache->recent_item);
		return cache->recent_item;
	}

	// TODO: how about a sorted list or a hash system?
#ifdef FASTER_MAP_LOAD
	for (i = 0; i < cache->num_items; i++)
#else
	for (i = 0; i < cache->max_item; i++)
#endif
	{
		citem = cache->cached_items[i];
		if (citem && citem->name && citem->cache_item == item)
		{
			cache_use(citem);
			cache->recent_item = citem;
			return citem;
		}
	}

	return NULL;
}

void *cache_find_item(cache_struct *cache, const char *name)
{
	cache_item_struct *item_ptr = cache_find(cache, name);
	if (!item_ptr)
		return NULL;

	//cache_use(item_ptr);
	return item_ptr->cache_item;
}

cache_item_struct *cache_add_item(cache_struct *cache, const char* name,
	void *item, Uint32 size)
{
#ifdef FASTER_MAP_LOAD
	cache_item_struct *new_item;
	int i;

	if (!cache->cached_items)
		return NULL;

	if (cache->num_items >= cache->num_allocated)
		// make sure we have room - consider dynamic expansion if not
		return NULL;

	new_item = calloc(1, sizeof(cache_item_struct));
	if (!new_item)
		return NULL;

	new_item->cache_item = item;
	new_item->size = size;
	new_item->name = name;
	new_item->access_time = cur_time;
	new_item->access_count = 1;	//start at 0 or 1? Is this a usage

	for (i = 0; i < cache->num_items; i++)
	{
		if (strcmp(name, cache->cached_items[i]->name) <= 0)
		{
			memmove(cache->cached_items + (i+1), cache->cached_items + i,
				(cache->num_items-i) * sizeof(cache_item_struct*));
			break;
		}
	}
	cache->recent_item = cache->cached_items[i] = new_item;
	cache->num_items++;
	cache->total_size += size;

	if (cache != cache_system)
		cache_adj_size(cache_system, size, cache);

	return new_item;
#else
	Sint32 i;

	if(!cache->cached_items) return NULL;
	//find an empty slot
	for(i=cache->first_unused; i<cache->max_item; i++)
		{
			//is the slot empty
			if(!cache->cached_items[i])
				{
					break;
				}
		}
	// do we have a slot or expand the list?
	if(i >= cache->max_item)
		{
			// make sure we have room - consider dynamic expansion if not
			if(cache->max_item >= cache->num_allocated) return NULL;
			// keep track of the highesr used
			i= cache->max_item;
			cache->max_item++;
		}
	//allocate the memory
	cache->cached_items[i]=calloc(1,sizeof(cache_item_struct));
	if(!cache->cached_items[i]) return NULL;
	//adjusted the lowest unsued size
	cache->first_unused= i+1;
	//memorize the information
	cache->cached_items[i]->cache_item=item;
	cache->cached_items[i]->size=size;
	cache->cached_items[i]->name=name;
	cache->cached_items[i]->access_time=cur_time;
	cache->cached_items[i]->access_count=1;	//start at 0 or 1? Is this a usage
	cache->num_items++;
	cache->total_size+=size;
	if(cache != cache_system) cache_adj_size(cache_system, size, cache);
	//return the pointer to the detailed item
	cache->recent_item = cache->cached_items[i];
	return(cache->recent_item);
#endif
}

void cache_set_name(cache_struct *cache, const char* name, void *item)
{
	cache_item_struct *item_ptr = cache_find_ptr(cache, item);
	if (item_ptr)
	{
		item_ptr->name = name;
#ifdef FASTER_MAP_LOAD
		qsort(cache->cached_items, cache->num_items, sizeof(item_ptr),
			cache_item_cmp);
#endif
	}
}

void cache_adj_size(cache_struct *cache, Uint32 size, void *item)
{
	cache_item_struct *item_ptr = cache_find_ptr(cache, item);
	if (item_ptr)
	{
		// adjust the current size
		//if(item_ptr->size != size)
		//{
			cache->total_size += size;
			if (cache != cache_system)
				cache_adj_size(cache_system, size, cache);
		//}
		item_ptr->size += size;
		cache_use(item_ptr);
		//item_ptr->access_time=cur_time;
		//item_ptr->access_count++;
	}
}

static void cache_remove(cache_struct *cache, cache_item_struct *item)
{
	if (!item || !cache->cached_items)
		return;		//nothing to do
	if (cache != cache_system)
		cache_adj_size(cache_system, -item->size, cache);
	if (item->cache_item && cache->free_item)
		(*cache->free_item)(item->cache_item);
	cache->total_size -= item->size;
	cache->num_items--;
	cache->recent_item = NULL;	//forget where we are just incase

	item->cache_item = NULL;	//failsafe
	item->name = NULL;		//failsafe
	item->size = 0;			//failsafe
#ifdef FASTER_MAP_LOAD
	if (cache->num_items > 0)
	{
		cache_item_struct **ci = cache->cached_items + cache->num_items;
		if (*ci == item)
		{
			free(*ci);
		}
		else
		{
			while (ci-- > cache->cached_items)
			{
				if (*ci == item)
				{
					free(*ci);
					memmove(ci, ci+1,
						(cache->num_items-(ci-cache->cached_items))*sizeof(cache_item_struct*));
					break;
				}
			}
		}
	}
#else  // FASTER_MAP_LOAD
	if(cache->max_item > 0)
		{
			// special case, at end (most common usage)
			if(cache->cached_items[cache->max_item-1] == item)
				{
					Sint32	i=cache->max_item-1;
					// remove it from the list
					free(cache->cached_items[i]);
					cache->cached_items[i]=NULL;
					// work backwards to skip over empty slots
					while(i>0 && cache->cached_items[i]==NULL)
						{
							i--;
						}
					// now memorize the new high mark
					cache->max_item=i+1;
					// and adjust first unused if needed
					if(cache->first_unused > i)	cache->first_unused= i;;
				}
			else
				{
					Sint32	i;
					// start at the end and work backwards looking for this item
					for(i=cache->max_item-1; i>=0 ; i--)
						{
							if(cache->cached_items[i] == item)
								{
									//remove it from the list
									free(cache->cached_items[i]);
									cache->cached_items[i]=NULL;
									// and adjust first unused if needed
									if(cache->first_unused > i)	cache->first_unused= i;;
									break;
								}
						}
				}
		}
#endif // FASTER_MAP_LOAD
}

static void cache_remove_all(cache_struct *cache)
{
	Sint32 i;

	if (!cache->cached_items)
		return;

#ifdef FASTER_MAP_LOAD
	for (i = cache->num_items-1; i >= 0; i--)
#else
	for(i=cache->max_item-1; i>=0; i--)
#endif
	{
		if(cache->cached_items[i])
		{
			cache_remove(cache, cache->cached_items[i]);
			cache->cached_items[i]=NULL;
		}
	}

#ifdef FASTER_MAP_LOAD
	cache->num_items = 0;
#else
	cache->num_items = cache->max_item = 0;
#endif
	cache->recent_item = NULL;	//forget where we are just incase
}

/* currently UNUSED
void cache_remove_unused(cache_struct *cache)
{
	Sint32	i;

	if(!cache->cached_items) return;
	for(i=cache->max_item-1; i>=0; i--)
		{
			if(cache->cached_items[i] && cache->cached_items[i]->access_count == 0)
				{
					cache_remove(cache, cache->cached_items[i]);
					cache->cached_items[i]=NULL;
				}
		}
	cache->recent_item = NULL;	//forget where we are just incase
}

void cache_system_shutdown()
{
	cache_delete(cache_system);
	cache_system=NULL;
}

void cache_clear_counter(cache_struct *cache)
{
	Sint32	i;

	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i])
				{
					cache->cached_items[i]->access_count=0;
				}
		}
}

void cache_use_item(cache_struct *cache, const void *item_data)
{
	cache_item_struct *item_ptr=NULL;

	if(!cache->cached_items) return;
	item_ptr=cache_find_ptr(cache, item_data);
	//if(item_ptr)
	//	{
	//		item_ptr->access_time=cur_time;
	//		item_ptr->access_count++;
	//	}
}

void cache_set_size(cache_struct *cache, Uint32 size, void *item)
{
	cache_item_struct *item_ptr;

	item_ptr=cache_find_ptr(cache, item);
	if(item_ptr)
		{
			// adjust from the old size to the new siae
			if(item_ptr->size != size)
				{
					cache->total_size-=item_ptr->size;
					cache->total_size+=size;
					if(cache != cache_system)
						{
							cache_adj_size(cache_system, size-item_ptr->size, cache);
						}
				}
			item_ptr->size=size;
			cache_use(item_ptr);
			//item_ptr->access_time=cur_time;
			//item_ptr->access_count++;
		}
}

void cache_remove_item(cache_struct *cache, const Uint8 *name)
{
	cache_item_struct *item;

	if(!cache->cached_items) return;
	item=cache_find(cache, name);
	if(item) cache_remove(cache, item);
}
*/
