/*!
 * \file
 * \ingroup	cache
 * \brief	Handles the cache system in EL
 */
#ifndef __file_cache_H__
#define __file_cache_H__

/*!
 * a single item storable in the cache
 */
typedef struct
{
	void	*cache_item;	/*!< pointer to the item we are caching */
	Uint32	size;			/*!< size of item */
	Uint32	access_time;	/*!< last time used */
	Uint32	access_count;	/*!< number of usages since last checkpoint */
	Uint8	*name;			/*!< original source or name, NOTE: this is NOT free()'d and allows dups! */
}cache_item_struct;

/*!
 * structure of the cache used
 */
typedef struct
{
	cache_item_struct	**cached_items; /*!< list of cached items */
	cache_item_struct	*recent_item; /*!< pointer to the last used item */
	Sint32	num_items;		/*!< the number of active items in the list */
	Sint32	max_item;		/*!< the highest slot used */
	Sint32	first_unused;	/*!< the lowest possible unused slow (might be in use!!) */
	Sint32	num_allocated;	/*!< the allocated space for the list */
	Uint32	LRU_time;		/*!< last time LRU processing done */
	Uint32	total_size;		/*!< total size currently allocated */
	Uint32	time_limit;		/*!< limit on LRU time before forcing a scan */
	Uint32	size_limit;		/*!< limit on size before forcing a scan */
	void	(*free_item)();	/*!< routine to call to free an item */
	Uint32	(*compact_item)();	/*!< routine to call to reduce memory usage without freeing */
}cache_struct;

/*!
 * we use a separate cache structure to cache textures.
 */
typedef struct
{
	int texture_id; /*!< the id of the texture */
    char file_name[128]; /*!< the filename of the texture */
	cache_item_struct	*cache_ptr; /*!< a pointer to the cached item */
	unsigned char alpha; /*!< used for alpha blending the texture */
}texture_cache_struct;

extern texture_cache_struct texture_cache[1000]; /*!< global texture cache */

/*!
 * \name Cache constants
 */
/*! @{ */
#define	MAX_CACHE_SYSTEM	32 /*!< max. number of cached items in \see cache_system */
/*! @} */

extern cache_struct	*cache_system; /*!< system cache */
extern cache_struct	*cache_md2; /*!< md2 cache */
extern cache_struct	*cache_e3d; /*!< e3d cache */
extern cache_struct	*cache_texture; /*!< texture cache */
//extern cache_struct *cache_system; // commented out due to redundant declaration */

//proto

/*!
 * \ingroup cache
 * \brief   initializes the cache system with the given number of items to max. use
 *
 *      Initializes the cache system. \a max_items determines the maximal number of items in the cache.
 *
 * \param max_items     maximum number of items in the cache
 *
 * \callgraph
 */
void cache_system_init(Uint32 max_items);

/*!
 * \ingroup cache
 * \brief clears and shuts down the cache system.
 *
 *      Clears and shuts down the cache system.
 *
 * \callgraph
 */
void cache_system_shutdown();

/*!
 * \ingroup cache
 * \brief      runs a cache maintenance routine
 *
 *      Runs a cache maintenance routine.
 *
 * \callgraph
 */
void cache_system_maint();

/*!
 * \ingroup cache
 * \brief   cleans out the cache system.
 *
 *      Cleans out the cache system of items no longer used.
 *
 * \retval Uint32
 * \callgraph
 */
Uint32 cache_system_clean();

/*!
 * \ingroup cache
 * \brief compacts the cache system.
 *
 *      Compacts the cache system.
 *
 * \retval Uint32
 * \callgraph
 */
Uint32 cache_system_compact();

/*!
 * \ingroup cache
 * \brief dumps the sizes of the given \a cache.
 *
 *      Dumps the sizes of the given \a cache to the console.
 *
 * \param cache     cache to query for its size.
 *
 * \callgraph
 */
void cache_dump_sizes(cache_struct *cache);


/*!
 * \ingroup cache
 * \brief   initializes a new cache system with \a max_items items and the given callback routine to free an item.
 *
 *      Initializes a new cache system with \a max_items items and the given callback routine to free an item.
 *
 * \param max_items         max. number of items in the cache
 * \param free_item         routine used to free items in the cache.
 * \retval cache_struct*    a pointer to a newly created cache.
 * \callgraph
 */
cache_struct *cache_init(Uint32 max_items, void (*free_item)());

/*!
 * \ingroup cache
 * \brief   sets the free item handler for the given \see cache_struct \a cache.
 *
 *      Sets the routine to handle freeing of items for the given \a cache.
 *
 * \param cache     the cache for which to set the free item handler.
 * \param free_item routine to use when items in \a cache get freed.
 */
void cache_set_free(cache_struct *cache, void (*free_item)());

/*!
 * \ingroup cache
 * \brief   sets the compact handler for the given \see cache_struct \a cache.
 *
 *      Sets the routine used to compact items in \a cache.
 *
 * \param cache         the cache for which to set the compact item handler.
 * \param compact_item  routine to use when items in \a cache get compacted.
 */
void cache_set_compact(cache_struct *cache, Uint32 (*compact_item)());

/*!
 * \ingroup cache
 * \brief   sets the \a time_limit for items in \a cache.
 *
 *      Sets a \a time_limit for items in the given \a cache.
 *
 * \param cache         the cache for which the time limit should be set.
 * \param time_limit    the max. amount of time to live for items in \a cache.
 */
void cache_set_time_limit(cache_struct *cache, Uint32 time_limit);

/*!
 * \ingroup cache
 * \brief   sets a \a size_limit for items in \a cache.
 *
 *      Sets a \a size_limit in bytes for items in the given \a cache.
 *
 * \param cache         the cache for which the size limit should be set.
 * \param size_limit    the max. size for items in \a cache (in bytes).
 */
void cache_set_size_limit(cache_struct *cache, Uint32 size_limit);

/*!
 * \ingroup cache
 * \brief       cleans the given \a cache.
 *
 *      Cleans the given \a cache.
 *
 * \param cache     cache to clean.
 * \retval Uint32
 * \callgraph
 */
Uint32 cache_clean(cache_struct *cache);

/*!
 * \ingroup cache
 * \brief       compacts the given \a cache.
 *
 *      Compacts the given \a cache.
 *
 * \param cache     the cache to compact.
 * \retval Uint32
 * \callgraph
 */
Uint32 cache_compact(cache_struct *cache);

/*!
 * \ingroup cache
 * \brief       deletes the given \a cache.
 *
 *      Deletes the given \a cache and frees up used memory.
 *
 * \param cache     the cache to delete.
 *
 * \callgraph
 */
void cache_delete(cache_struct *cache);

/*!
 * \ingroup cache
 * \brief       clears all the counters in the given \a cache.
 *
 *      Clears all the counters in the given \a cache.
 *
 * \param cache     the cache for which the counters should get cleared.
 */
void cache_clear_counter(cache_struct *cache);

/*!
 * \ingroup cache
 * \brief adds the given \a item to \a cache with the given \a name.
 *
 *      Adds the given \a item to \a cache with the given \a name. The parameter \a size determines the maximum size of items in \a cache.
 *
 * \param cache                 the cache to which \a item gets added
 * \param name                  the name to use for \a item in \a cache
 * \param item                  a pointer to the item to add
 * \param size                  max. size of items in \a cache.
 * \retval cache_item_struct*   a pointer to a \see cache_item_struct of the given \a item.
 * \callgraph
 */
cache_item_struct *cache_add_item(cache_struct *cache, Uint8 *name, void *item, Uint32 size);

/*!
 * \ingroup cache
 * \brief   sets the \a name of the given \a item in \a cache.
 *
 *      Sets the \a name of the given \a item in \a cache.
 *
 * \param cache     the cache which contains the \a item to change the \a name
 * \param name      the new name of \a item in \a cache
 * \param item      a pointer to the item which \a name should get changed.
 *
 * \callgraph
 */
void cache_set_name(cache_struct *cache, Uint8 *name, void *item);

/*!
 * \ingroup cache
 * \brief       sets the \a size of the given \a item in \a cache.
 *
 *      Sets the \a size of the given \a item in \a cache.
 *
 * \param cache     the cache which contains the \a item to the the \a size.
 * \param size      the new size of \a item in \a cache.
 * \param item      a pointer to the item which \a size should get changed.
 *
 * \callgraph
 */
void cache_set_size(cache_struct *cache, Uint32 size, void *item);

/*!
 * \ingroup cache
 * \brief       adjusts the \a size of the given \a item in \a cache.
 *
 *      Adjusts the \a size of the given \a item in \a cache.
 *
 * \param cache     the cache which contains the \a item to adjust the \a size.
 * \param size      the new size of \a item in \a cache.
 * \param item      a pointer to the item which \a size should get adjusted.
 *
 * \callgraph
 */
void cache_adj_size(cache_struct *cache, Uint32 size, void *item);

/*!
 * \ingroup cache
 * \brief   determines whether \a item is an element of \a cache.
 *
 *      Determines whether \a item is an element of \a cache.
 *
 * \param cache     the cache to search
 * \param item      the item to search for
 */
void cache_use(cache_struct *cache, cache_item_struct *item);

/*!
 * \ingroup cache
 * \brief   determines whether the data in \a item_data belongs to a given item in \a cache.
 *
 *      Determines whether the data in \a item_data belongs to a given item in \a cache.
 *
 * \param cache         the cache to search
 * \param item_data     data of which item to search for
 *
 * \callgraph
 */
void cache_use_item(cache_struct *cache, const void *item_data);

/*!
 * \ingroup cache
 * \brief       looks up the \a cache for an item with the given \a name.
 *
 *      Looks up the given \a cache for an item with the given \a name.
 *
 * \param cache                 the cache to search
 * \param name                  the name to look for in \a cache.
 * \retval cache_item_struct*   a pointer to a cache item with the given \a name.
 * \callgraph
 */
cache_item_struct *cache_find(cache_struct *cache, const Uint8 *name);

/*!
 * \ingroup cache
 * \brief       retrieves the \a item at the given address in \a cache.
 *
 *      Retrieves the \a item at the given address in \a cache.
 *
 * \param cache                 the cache to search
 * \param item                  address of the item to look for
 * \retval cache_item_struct*   a pointer to a cache item stored at location of \a item.
 * \callgraph
 */
cache_item_struct *cache_find_ptr(cache_struct *cache, const void *item);

/*!
 * \ingroup cache
 * \brief       looks up the item with the given \a name in \a cache.
 *
 *      Looks up the item with given \a name in \a cache.
 *
 * \param cache         the cache to search
 * \param name          a pointer to the name to look for in \a cache.
 * \retval void*        a pointer to the cache item given by \a name
 * \callgraph
 */
void *cache_find_item(cache_struct *cache, const Uint8 *name);

/*!
 * \ingroup cache
 * \brief       removes the given \a item from \a cache.
 *
 *      Removes the given \a item from \a cache.
 *
 * \param cache         the cache which gets \a item removed.
 * \param item          the item to remove from \a cache.
 *
 * \callgraph
 */
void cache_remove(cache_struct *cache, cache_item_struct *item);

/*!
 * \ingroup cache
 * \brief       removes the item with the given \a name from \a cache.
 *
 *      Removes the item with the given \a name from \a cache.
 *
 * \param cache         the cache whichs gets \a name removed
 * \param name          the name to lookup in \a cache and delete.
 *
 * \callgraph
 */
void cache_remove_item(cache_struct *cache, const Uint8 *name);

/*!
 * \ingroup cache
 * \brief       clears the given \a cache.
 *
 *      Clears the given \a cache, removes all entries and frees up consumed memory.
 *
 * \param cache         the cache from which to remove all items.
 *
 * \callgraph
 */
void cache_remove_all(cache_struct *cache);

/*!
 * \ingroup cache
 * \brief       cleans up the given \a cache by removing unused items.
 *
 *      Clears up the given \a cache by removing any unused items.
 *
 * \param cache     the cache to clear
 *
 * \callgraph
 */
void cache_remove_unused(cache_struct *cache);

#endif
