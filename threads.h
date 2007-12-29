#ifndef	_THREADS_H_
#define	_THREADS_H_

#include <SDL/SDL_error.h>
#include <SDL/SDL_mutex.h>

#define CHECK_AND_LOCK_MUTEX(mutex)	\
do	\
{	\
	if (SDL_LockMutex(mutex) != 0)	\
	{	\
		fprintf(stderr, "Lock error '%s' at file '%s' in funcion '%s' line %d\n",	\
			SDL_GetError(), __FILE__, __FUNCTION__, __LINE__);	\
	}	\
}	\
while (0)

#define CHECK_AND_UNLOCK_MUTEX(mutex)	\
do	\
{	\
	if (SDL_UnlockMutex(mutex) != 0)	\
	{	\
		fprintf(stderr, "Unlock error '%s' at file '%s' in funcion '%s' line %d\n",	\
			SDL_GetError(), __FILE__, __FUNCTION__, __LINE__);	\
	}	\
}	\
while (0)

#endif	// _THREADS_H_

