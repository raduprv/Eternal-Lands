#ifndef __NAMED_COLOURS_H
#define __NAMED_COLOURS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

/*!< The id return if the colour was not found by elglGetColourId() */
#define ELGL_INVALID_COLOUR ((size_t)0)

/*!
 * \ingroup named_colours
 * \brief Initialise the named colours state, including reading from file.
 *
 * \callgraph
 */
void init_named_colours(void);

/*!
 * \ingroup named_colours
 * \brief Get the unique id for the named colour
 *
 * \param	name The name of the colour.
 * \return	The unique id, once read, will not change.  If not found ELGL_INVALID_COLOUR will be returned.
 * \callgraph
 */
size_t elglGetColourId(const char *name);

/*!
 * \ingroup named_colours
 * \brief Set the current GL colour.  This will be faster than using elglColourN().
 *
 * \param	index The id of the colour, previously obtained using elglGetColourId().
 * \callgraph
 */
void elglColourI(size_t index);

/*!
 * \ingroup named_colours
 * \brief Set the current GL colour.
 *
 * \param	name The name of the colour.
 * \callgraph
 */
void elglColourN(const char *name);

/*!
 * \ingroup named_colours
 * \brief Get the 3 GLfloat tuple of a colour.
 *
 * \param	name The name of the colour.
 * \param	buf Buffer to store tuple
 * \callgraph
 */
void elglGetColour3v(const char *name, GLfloat *buf);

/*!
 * \ingroup named_colours
 * \brief Get the 4 GLfloat tuple of a colour.
 *
 * \param	name The name of the colour.
 * \param	buf Buffer to store tuple
 * \callgraph
 */
void elglGetColour4v(const char *name, GLfloat *buf);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
