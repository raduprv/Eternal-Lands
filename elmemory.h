#ifndef ELMEMORY_H__
#define ELMEMORY_H__

// #include this header here, so that our redefines of *alloc and free aren't
// overwritten when stdlib.h is included after this file.
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

 #ifdef MEMORY_DEBUG
  #define malloc(size) elm_malloc(size, __FILE__, __LINE__)
  #define free(pointer) elm_free(pointer, __FILE__, __LINE__)
  #define realloc(pointer, size) elm_realloc(pointer, size, __FILE__, __LINE__)
  #define calloc(nmemb, size) elm_calloc((nmemb), (size), __FILE__, __LINE__)

void elm_init();
void elm_cleanup();
void *elm_malloc(const size_t size, char *file, const int line);
void elm_free(void *ptr, char *file, const int line);
void *elm_calloc(const size_t nmemb, const size_t size, char *file, const int line);
void *elm_realloc(void *ptr, const size_t size, char *file, const int line);
void *elm_in_list(const void *pointer);
 #endif //MEMORY_DEBUG

#ifdef __cplusplus
} // extern "C"
#endif

#endif //ELMEMORY_H__
