#ifdef MEMORY_DEBUG
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <SDL.h>
#include <SDL_thread.h>
#include "asc.h"
#include "init.h"
#include "io/elpathwrapper.h"

/* The size of the array */
#define ELM_INITIAL_SIZE 1024
/* How many times we have reallocated ELM_INITIAL_SIZE bytes */
int elm_allocs = 0;
/* A mutex, we always need a mutex */
SDL_mutex *elm_mutex = NULL;

struct elm_memory_struct {
	size_t size;
	void *pointer;
	char *file;
	int line;
} *elm_memory;

void elm_init()
{
	elm_mutex = SDL_CreateMutex();
	elm_memory = calloc(ELM_INITIAL_SIZE, sizeof(*elm_memory));
	elm_allocs++;
}

void elm_cleanup()
{
	unsigned int i;
	unsigned int malloc_calls_remaining = 0;
	FILE *fp;

	fp = open_file_config ("elmemory.log", "w");
	if(fp ==NULL){
		return;
	}
	fprintf(fp, "-------Pointers not free'd-------\n");
	for(i = 0; i < ELM_INITIAL_SIZE*elm_allocs; i++) {
		if(elm_memory[i].pointer != NULL && elm_memory[i].size != 0) {
			malloc_calls_remaining++;
			fprintf(fp, "%s:%i:%p (%zu bytes)\n", elm_memory[i].file, elm_memory[i].line, elm_memory[i].pointer, elm_memory[i].size);
			free(elm_memory[i].pointer);
		}
	}
	fclose(fp);
	fprintf(stderr, "EL Memory debugger: %i malloc/calloc calls not free'd\n", malloc_calls_remaining);
	SDL_DestroyMutex(elm_mutex);
	free(elm_memory);
}

void elm_add_to_list(void *pointer, const size_t size, char *file, const int line)
{
	int i;
	/* Look for a free slot */
	for(i = 0; i < ELM_INITIAL_SIZE*elm_allocs; i++) {
		if(elm_memory[i].size == 0 && elm_memory[i].pointer == NULL) {
			/* Yay! A free slot! */
			elm_memory[i].size = size;
			elm_memory[i].pointer = pointer;
			elm_memory[i].file = file;
			elm_memory[i].line = line;
			return;
		}
	}
	/* If we reach this point, there are no free slots,
	 * which means we have to allocate more space */
	elm_memory = realloc(elm_memory, ELM_INITIAL_SIZE*(elm_allocs+1)*sizeof(*elm_memory));
	memset(&elm_memory[i], 0, ELM_INITIAL_SIZE*sizeof(*elm_memory));
	elm_allocs++;
	elm_memory[i].size = size;
	elm_memory[i].pointer = pointer;
	elm_memory[i].file = file;
	elm_memory[i].line = line;
	return;
}

int elm_delete_from_list(const void *pointer)
{
	int i;

	/* Locate the pointer in the list. */
	for(i = 0; i < ELM_INITIAL_SIZE*elm_allocs; i++) {
		if(elm_memory[i].pointer == pointer) {
			/* We found it! */
			elm_memory[i].pointer = NULL;
			elm_memory[i].size = 0;
			return 1;
		}
	}
	return 0;
}

void *elm_in_list(const void *pointer)
{
	int i;

	for(i = 0; i < ELM_INITIAL_SIZE*elm_allocs; i++) {
		if(elm_memory[i].pointer == pointer) {
			/* We found it! */
			return &elm_memory[i];
		}
	}
	/* If we reach this point,
	 * the pointer wasn't in the list. */
	return NULL;
}

void *elm_malloc(const size_t size, char *file, const int line)
{
	void *pointer;

	pointer = malloc(size);
	if(pointer == NULL) {
		/* malloc failed, sound the alarms */
		fprintf(stderr, "%s:%i:malloc() failed\n", file, line);
	} else {
		SDL_LockMutex(elm_mutex);
		elm_add_to_list(pointer, size, file, line);
		SDL_UnlockMutex(elm_mutex);
	}
	return pointer;
}

void *elm_calloc(const size_t nmemb, const size_t size, char *file, const int line)
{
	void *pointer;

	pointer = calloc(nmemb, size);
	if(pointer == NULL) {
		fprintf(stderr, "%s:%i:calloc() failed\n", file, line);
	} else {
		SDL_LockMutex(elm_mutex);
		elm_add_to_list(pointer, nmemb*size, file, line);
		SDL_UnlockMutex(elm_mutex);
	}
	return pointer;
}

void elm_free(void *ptr, char *file, const int line)
{
	if(ptr == NULL) {
		fprintf(stderr, "%s:%i:Freeing a NULL pointer\n", file, line);
	}
	SDL_LockMutex(elm_mutex);
	if(elm_delete_from_list(ptr)) {
		/* We found the pointer in the list
		 * and it's safe to free() it. */
		free (ptr);
	} else {
		/* We're trying to free 
		 * something that's not allocated, which is evil.
		 * Sound the alarms. */
		fprintf(stderr, "%s:%i:Attempting to free a pointer(%p) which isn't allocated!\n", file, line, ptr);
	}
	SDL_UnlockMutex(elm_mutex);
}

void *elm_realloc(void *ptr, const size_t size, char *file, const int line)
{
	void *new_pointer;
	struct elm_memory_struct *mem = NULL;
	SDL_LockMutex(elm_mutex);
	if(ptr == NULL) {
		/* man realloc: If ptr is NULL,
		 * behave as malloc() */
		SDL_UnlockMutex(elm_mutex);
		return elm_malloc(size, file, line);
	} else if (size == 0) {
		/* man realloc: If size is 0,
		 * behave as free() */
		SDL_UnlockMutex(elm_mutex);
		elm_free(ptr, file, line);
		return NULL;
	} else if((mem = elm_in_list(ptr)) != NULL) {
		new_pointer = realloc(ptr, size);
		if(new_pointer != ptr) {
			/* The area was moved. Update the list. */
			mem->pointer = new_pointer;
		}
		/* Update the size in the struct */
		mem->size = size;
		mem->file = file;
		mem->line = line;
		SDL_UnlockMutex(elm_mutex);
		return new_pointer;
	} else {
		/* The pointer wasn't found in the list,
		 * which means we'd get an error */
		fprintf(stderr, "%s:%i:Attempting to realloc a pointer(%p), which isn't allocated! %i\n", file, line, ptr, ELM_INITIAL_SIZE*elm_allocs);
		SDL_UnlockMutex(elm_mutex);
		return ptr;
	}
	SDL_UnlockMutex(elm_mutex);
}
#endif //MEMORY_DEBUG
