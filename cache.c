#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "cache.h"

cache_struct	*cache_system=NULL;

cache_struct	*cache_md2=NULL;
cache_struct	*cache_e3d=NULL;

texture_cache_struct texture_cache[1000];
obj_2d_cache_struct obj_2d_def_cache[max_obj_2d_def];

// top level cache system routines
void cache_system_init(Uint32 max_items)
{
	cache_system=cache_init(max_items, &cache_delete);
	cache_set_time_limit(cache_system, 1*60*1000);	// once per minute check for processing
}

void cache_system_shutdown()
{
	cache_delete(cache_system);
	cache_system=NULL;
}

void cache_dump_sizes(cache_struct *cache)
{
	Sint32	i;
	Uint8	str[256];

	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i])
				{
					Uint8 scale=' ';
					Uint32 size=cache->cached_items[i]->size;
					if(size > 100000000)
						{
							size/=1024*1024;
							scale='M';
						}
					else if(size > 100000)
						{
							size/=1024;
							scale='K';
						}
					sprintf(str, "%s %6d%c - %d: %s", cache_size_str, size, scale, i, cache->cached_items[i]->name);
					put_colored_text_in_buffer(c_yellow1, str, -1, 0);
					write_to_log(str, strlen(str));
				}
		}
}

void	cache_system_maint()
{
	if(!cur_time || !cache_system || !cache_system->time_limit || cache_system->LRU_time+cache_system->time_limit > cur_time) return;
	//clean anything we can delete
	cache_system_clean();
	//do an automated memory compaction
	cache_system_compact();
	//actually done already, just forcing it to assist in debugging
	cache_system->LRU_time = cur_time;
}

Uint32	cache_system_clean()
{
	Sint32	i;
	Uint32	mem_freed=0;

	if(!cache_system || !cache_system->time_limit || !cache_system->cached_items) return 0;
	// make sure we are in a safe place
	if(interface_mode != interface_game) return 0;
	for(i=0; i<cache_system->max_item; i++)
		{
			if(cache_system->cached_items[i] && cache_system->cached_items[i]->cache_item)
				{
					mem_freed+= cache_clean(cache_system->cached_items[i]->cache_item);
				}
		}
	//adjust the LRU time-stamp
	cache_system->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

Uint32	cache_system_compact()
{
	Sint32	i;
	Uint32	mem_freed=0;

	if(!cache_system || !cache_system->time_limit || !cache_system->cached_items) return 0;
	for(i=0; i<cache_system->max_item; i++)
		{
			if(cache_system->cached_items[i] && cache_system->cached_items[i]->cache_item)
				{
					mem_freed+= cache_compact(cache_system->cached_items[i]->cache_item);
				}
		}
	//adjust the LRU time-stamp
	cache_system->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

// individual caches
cache_struct *cache_init(Uint32 max_items, void (*free_item)())
{
	cache_struct *cache;

	cache=calloc(1, sizeof(cache_struct));
	if(!cache)	return NULL;	//oops, not enough memory

	cache->cached_items=calloc(max_items, sizeof(cache_item_struct *));
	if(!cache->cached_items)
		{
			free(cache);
			return NULL;	//oops, not enough memory
		}
	cache->recent_item=NULL;
	cache->num_allocated=max_items;
	cache->LRU_time=cur_time;
	cache->time_limit=0;	// 0 == no time based LRU check
	cache->size_limit=0;	// 0 == no space based LRU check
	cache->free_item=free_item;
	cache->compact_item=NULL;
	if(cache_system) cache_add_item(cache_system, "", cache, sizeof(cache_struct)+max_items*sizeof(cache_item_struct *));
	//all done, send the data back
	return(cache);
}

void cache_delete(cache_struct *cache)
{
	static int cache_delete_loop_block=0;

	if(!cache) return;
	if(cache != cache_system) cache_adj_size(cache_system, -cache->total_size, cache);
	if(cache->cached_items)
		{
			cache_remove_all(cache);
			free(cache->cached_items);
			cache->cached_items=NULL;	//failsafe
			cache->recent_item=NULL;	//failsafe
		}
	if(cache_system && cache != cache_system && !cache_delete_loop_block)
		{
			cache_delete_loop_block++;
			cache_remove(cache_system, cache_find_ptr(cache_system, cache));
			cache_delete_loop_block--;
		}
	free(cache);
}

void cache_set_free(cache_struct *cache, void (*free_item)())
{
	cache->free_item=free_item;
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


Uint32 cache_clean(cache_struct *cache)
{
	Sint32	i;
	Uint32	mem_freed=0;

	if(!cache->cached_items || !cache->time_limit || !cache->free_item) return 0;
	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i] && cache->cached_items[i]->cache_item)
				{
					//decide if this entry needs to be cleaned
					if(cache->cached_items[i]->access_count == 0 && cache->cached_items[i]->access_time+cache->time_limit < cur_time)
						{
							mem_freed+= cache->cached_items[i]->size;
							cache_remove(cache, cache->cached_items[i]);
						}
					//cache->cached_items[i]->access_count=0;
				}
		}
	//adjust the LRU time-stamp
	cache->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}

Uint32 cache_compact(cache_struct *cache)
{
	Sint32	i;
	Uint32	freed;
	Uint32	mem_freed=0;

	if(!cache->cached_items || !cache->time_limit || !cache->compact_item) return 0;
	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i] && cache->cached_items[i]->cache_item)
				{
					//decide if this entry needs to be cleaned
					if(cache->cached_items[i]->access_count == 0 && cache->cached_items[i]->access_time+cache->time_limit < cur_time)
					//if(cache->cached_items[i]->access_time+cache->time_limit < cur_time)
						{
							freed= (*cache->compact_item)(cache->cached_items[i]->cache_item);
							mem_freed+= freed;
							cache_adj_size(cache, -freed, cache->cached_items[i]->cache_item);
						}
					else
						{
							cache->cached_items[i]->access_count=0;	// clear the counter
						}
				}
		}
	//adjust the LRU time-stamp
	cache->LRU_time = cur_time;
	//return how much memory was freed
	return mem_freed;
}


// detailed items
void	cache_use(cache_struct *cache, cache_item_struct *item_ptr)
{
	if(item_ptr)
		{
			item_ptr->access_time=cur_time;
			item_ptr->access_count++;
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

cache_item_struct *cache_find(cache_struct *cache, const Uint8 *name)
{
	Sint32	i;

	if(!cache->cached_items) return 0;
	// quick check for the most recent item
	if(cache->recent_item && cache->recent_item->name && !strcmp(cache->recent_item->name, name))
		{
			cache_use(cache, cache->recent_item);
			return(cache->recent_item);
		}
	// not the most recent, then scan the list
	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i] && cache->cached_items[i]->name && !strcmp(cache->cached_items[i]->name, name))
				{
					cache_use(cache, cache->cached_items[i]);
					cache->recent_item= cache->cached_items[i];
					return(cache->cached_items[i]);
				}
		}
	return NULL;
}

cache_item_struct *cache_find_ptr(cache_struct *cache, const void *item)
{
	Sint32	i;

	if(!cache->cached_items) return 0;
	// quick check for the most recent item
	if(cache->recent_item && cache->recent_item->name && cache->recent_item->cache_item == item)
		{
			cache_use(cache, cache->recent_item);
			return(cache->recent_item);
		}
	for(i=0; i<cache->max_item; i++)
		{
			if(cache->cached_items[i] && cache->cached_items[i]->name && cache->cached_items[i]->cache_item == item)
				{
					cache_use(cache, cache->cached_items[i]);
					cache->recent_item=cache->cached_items[i];
					return(cache->cached_items[i]);
				}
		}
	return NULL;
}

void *cache_find_item(cache_struct *cache, const Uint8 *name)
{
	cache_item_struct	*item_ptr;

	if(!cache->cached_items) return NULL;
	item_ptr= cache_find(cache, name);
	if(item_ptr)
		{
			//cache_use(cache, item_ptr);
			return(item_ptr->cache_item);
		}
	return(NULL);
}

cache_item_struct *cache_add_item(cache_struct *cache, Uint8 *name, void *item, Uint32 size)
{
	Sint32	i;

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
}

void cache_set_name(cache_struct *cache, Uint8 *name, void *item)
{
	cache_item_struct *item_ptr;

	item_ptr=cache_find_ptr(cache, item);
	if(item_ptr)
		{
			item_ptr->name=name;
		}
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
			cache_use(cache, item_ptr);
			//item_ptr->access_time=cur_time;
			//item_ptr->access_count++;
		}
}

void cache_adj_size(cache_struct *cache, Uint32 size, void *item)
{
	cache_item_struct *item_ptr;

	item_ptr=cache_find_ptr(cache, item);
	if(item_ptr)
		{
			// adjust the current size
			//if(item_ptr->size != size)
			//	{
					cache->total_size+=size;
					if(cache != cache_system)
						{
							cache_adj_size(cache_system, size, cache);
						}
			//	}
			item_ptr->size+=size;
			cache_use(cache, item_ptr);
			//item_ptr->access_time=cur_time;
			//item_ptr->access_count++;
		}
}

void cache_remove(cache_struct *cache, cache_item_struct *item)
{
	if(!item)	return;		//nothing to do
	if(!cache->cached_items) return;
	if(cache != cache_system) cache_adj_size(cache_system, -item->size, cache);
	if(item->cache_item && cache->free_item)	(*cache->free_item)(item->cache_item);
	cache->total_size-=item->size;
	cache->num_items--;
	item->cache_item=NULL;	//failsafe
	item->name=NULL;		//failsafe
	item->size=0;			//failsafe
	if(cache->max_item > 0)
		{
			// special case, at end (most common usage)
			if(cache->cached_items[cache->max_item-1] == item)
				{
					Sint32	i=cache->max_item-1;
					// remove it from the list
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
									cache->cached_items[i]=NULL;
									// and adjust first unused if needed
									if(cache->first_unused > i)	cache->first_unused= i;;
									break;
								}
						}
				}
		}
	cache->recent_item = NULL;	//forget where we are just incase
}

void cache_remove_item(cache_struct *cache, const Uint8 *name)
{
	cache_item_struct *item;

	if(!cache->cached_items) return;
	item=cache_find(cache, name);
	if(item) cache_remove(cache, item);
}

void cache_remove_all(cache_struct *cache)
{
	Sint32	i;

	if(!cache->cached_items) return;
	for(i=cache->max_item-1; i>=0; i--)
		{
			if(cache->cached_items[i])
				{
					cache_remove(cache, cache->cached_items[i]);
					cache->cached_items[i]=NULL;
				}
		}
	cache->num_items= cache->max_item= 0;
	cache->recent_item = NULL;	//forget where we are just incase
}

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

