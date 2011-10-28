/*!
 * \file
 * \ingroup	cache
 * \brief	Handles the cache system in EL
 */
#ifndef __FILE_CACHE_H__
#define __FILE_CACHE_H__

#include <SDL_types.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * a single item storable in the cache
 */
typedef struct
{
	void	*cache_item;	/*!< pointer to the item we are caching */
	Uint32	size;			/*!< size of item */
	Uint32	access_time;	/*!< last time used */
	Uint32	access_count;	/*!< number of usages since last checkpoint */
	const char *name;	/*!< original source or name, NOTE: this is NOT free()'d and allows dups! */
} cache_item_struct;

/*!
 * structure of the cache used
 */
typedef struct
{
	cache_item_struct	**cached_items; /*!< list of cached items */
	cache_item_struct	*recent_item; /*!< pointer to the last used item */
	Sint32	num_items;		/*!< the number of active items in the list */
#ifndef FASTER_MAP_LOAD
	Sint32	max_item;		/*!< the highest slot used */
	Sint32	first_unused;	/*!< the lowest possible unused slow (might be in use!!) */
#endif
	Sint32	num_allocated;	/*!< the allocated space for the list */
	Uint32	LRU_time;		/*!< last time LRU processing done */
	Uint32	total_size;		/*!< total size currently allocated */
	Uint32	time_limit;		/*!< limit on LRU time before forcing a scan */
	Uint32	size_limit;		/*!< limit on size before forcing a scan */
	void	(*free_item)();	/*!< routine to call to free an item */
	Uint32	(*compact_item)();	/*!< routine to call to reduce memory usage without freeing */
} cache_struct;

#ifndef	NEW_TEXTURES
/*!
 * we use a separate cache structure to cache textures.
 */
typedef struct
{
	int texture_id;               /*!< the id of the texture */
	char file_name[128];          /*!< the filename of the texture */
	cache_item_struct *cache_ptr; /*!< a pointer to the cached item */
	int alpha;                    /*!< used for alpha blending the texture */
	int has_alpha;                /*!< specify if the texture has an alpha map */
	char load_err;                /*!< if true, we tried to load this texture before and failed */
} texture_cache_struct;
#endif

/*!
 * \name Cache constants
 */
/*! @{ */
#define	MAX_CACHE_SYSTEM	32 /*!< max. number of cached items in \see cache_system */
/*! @} */

extern cache_struct	*cache_system; /*!< system cache */
extern cache_struct	*cache_e3d; /*!< e3d cache */

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
 * \brief      runs a cache maintenance routine
 *
 *      Runs a cache maintenance routine.
 *
 * \callgraph
 */
void cache_system_maint(void);

#ifdef	ELC
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
void cache_dump_sizes(const cache_struct *cache);
#endif	/* ELC */

/*!
 * \ingroup cache
 * \brief   initializes a new cache system with \a max_items items and the given callback routine to free an item.
 *
 *      Initializes a new cache system with \a max_items items and the given callback routine to free an item.
 *
 * \param name              the name of the cache
 * \param max_items         max. number of items in the cache
 * \param free_item         routine used to free items in the cache.
 * \retval cache_struct*    a pointer to a newly created cache.
 * \callgraph
 */
cache_struct *cache_init(const char* name, Uint32 max_items,
	void (*free_item)());

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
 * \brief   sets the function to free items
 *
 *      Sets the function to apply to each item before it is evicted from the
 *      cache.
 *
 * \param free_item     pointer to the free function
 */
void cache_set_free(cache_struct *cache, void (*free_item)());

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
cache_item_struct *cache_add_item(cache_struct *cache, const char* name,
	void *item, Uint32 size);

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
void cache_set_name(cache_struct *cache, const char* name, void *item);

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
 * \brief   update the last use time of a cache item
 *
 *      Sets the time a cache item was accessed last to the current time
 *
 * \param item      the item for which to set the access time
 */
#ifndef	USE_INLINE
void cache_use(cache_item_struct *item);
#else	//USE_INLINE
#include "global.h"

static __inline__ void	cache_use(cache_item_struct *item_ptr)
{
	if (item_ptr)
	{
		item_ptr->access_time = cur_time;
		item_ptr->access_count++;
	}
}
#endif	//USE_INLINE


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
void *cache_find_item (cache_struct *cache, const char* name);

void cache_delete(cache_struct *cache);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
