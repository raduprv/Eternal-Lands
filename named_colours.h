#ifndef __NAMED_COLOURS_H
#define __NAMED_COLOURS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ELC)
#include "platform.h"
#else
#include <GL/gl.h>
#endif

extern size_t ELGL_INVALID_COLOUR;

void init_named_colours(void);
void elglColourI(size_t index);
void elglColourN(const char *name);
size_t elglGetColourIndex(const char *name);
void elglGetColour3v(const char *name, GLfloat *buf);
void elglGetColour4v(const char *name, GLfloat *buf);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
