#ifndef __dbuffer_h__
#define __dbuffer_h__

/*!
 * \ingroup dbuffer
 * \file dbuffer.h
 * \brief Dynamic buffer implementation.
 *
 * A dynamic buffer is a buffer that reallocates more memory if needed to
 * accomodate a certain data size. The reallocation is done in chunks.
 * 
 * This dynamic buffer implementation only uses one structrure/pointer, so to
 * ease manipulation.
 * 
 * Basically you should do:
 * 
 * dbuffer_t *mybuffer = dbuffer_new();
 * 
 * mybuffer = dbuffer_append_data( mybuffer, somedatapointer, somedatasize );
 * 
 * do_something_with_contents( mybuffer->data, mybuffer->current_size );
 * 
 * dbuffer_destroy( mybuffer );
 * 
 *
 */


#include <sys/types.h>

/**
 \ingroup dbuffer
 \brief Main dbuffer structure

 The size of this "structure" is not fixed.

 */
typedef struct {
	size_t alloc_size; /**< Current allocated size of buffer */
	size_t current_size; /**< Current size used in buffer */
	unsigned char data[0]; /**< Data placeholder */
} dbuffer_t;

/**
 \ingroup dbuffer
 \brief Default allocation chunk size.
 */
#define DBUFFER_CHUNK_SIZE 128

#ifndef DOXYGEN_SHOULD_SKIP_THIS

# define DBUFFER_ALIGN(x) (((x)+(DBUFFER_CHUNK_SIZE)-1)&~(DBUFFER_CHUNK_SIZE-1))
# define DBUFFER_HDRSIZE (sizeof(size_t)+sizeof(size_t))

# ifndef UNUSED_RESULT_DECL
#  ifdef	__GNUC__
#   define UNUSED_RESULT_DECL __attribute__ ((warn_unused_result))
#  else
#   define UNUSED_RESULT_DECL
#  endif
# endif // UNUSED_RESULT_DECL

#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 \ingroup dbuffer
 \brief Destroy a dbuffer
 \param dbuf The dbuffer to destroy
 \returns nothing
 */
static __inline__ void dbuffer_destroy( dbuffer_t *dbuf )
{
    free(dbuf);
}

/**
 \ingroup dbuffer
 \brief Append data to a dbuffer

 Append the specified data (bytes) to dbuffer. Make sure you reassign your dbuffer to the return value,
 cause it can be reallocated.

 \param dbuf The dbuffer
 \param data The data to append
 \param datalen The data size
 \returns The dbuffer (maybe a different pointer)
 */


static __inline__ UNUSED_RESULT_DECL dbuffer_t *dbuffer_append_data( dbuffer_t *dbuf, const unsigned char *data, size_t datalen )
{
	size_t next_alloc_size;

	if ( NULL!= dbuf ) {

		next_alloc_size = DBUFFER_ALIGN( dbuf->current_size+datalen+DBUFFER_HDRSIZE );
		if (next_alloc_size>dbuf->alloc_size) {
			dbuf = (dbuffer_t*)realloc((void*)dbuf, next_alloc_size);
			if (NULL==dbuf)
                return NULL;
			dbuf->alloc_size = next_alloc_size;
		}
	} else {
        next_alloc_size = DBUFFER_ALIGN( datalen+DBUFFER_HDRSIZE );
		dbuf = (dbuffer_t*)malloc( next_alloc_size );
		if (NULL==dbuf)
            return NULL;
		dbuf->alloc_size = next_alloc_size;
        dbuf->current_size = 0;
	}
	if (data) {
		memcpy( dbuf->data + dbuf->current_size, data, datalen );
		dbuf->current_size += datalen;
	}
	return dbuf;
}

/**
 \ingroup dbuffer
 \brief Create a new (empty) dbuffer
 \returns The newly created dbuffer.
 */

static __inline__ dbuffer_t *dbuffer_new()
{
	return dbuffer_append_data(NULL, NULL, 0); /* Will alloc a new buffer */
}

/**
 \ingroup dbuffer
 \brief Create a new dbuffer, and preallocate size for it
 \param datalen The size to preallocate
 \returns The newly created dbuffer.
 */
static __inline__ dbuffer_t *dbuffer_sized(size_t datalen)
{
	return dbuffer_append_data(NULL, NULL, datalen); /* Will alloc a new buffer */
}

/**
 \ingroup dbuffer
 \brief Create a new dbuffer, and append the specified data
 \param data The data to append to newly created buffer
 \param datalen The size of data
 \returns The newly created dbuffer.
 */

static __inline__ dbuffer_t *dbuffer_new_with_data( const unsigned char *data, size_t datalen )
{
	return dbuffer_append_data(NULL, data, datalen); /* Will alloc a new buffer */
}


#endif
