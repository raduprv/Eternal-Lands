/*!
 * \file 
 * \brief symbol tables for the book parser
 */

#ifndef __BOOKS_SYMBOLS_H__
#define __BOOKS_SYMBOLS_H__

#ifdef ENCYCLOPEDIA

#include "../symbol_table.h"
#include "types.h"

/*!
 * \name symbol tables for the book parser 
 *
 * \{
 */
extern symbol_table * bp_elements;
extern symbol_table * bp_attributes;
extern symbol_table * bp_layouts;
extern symbol_table * bp_fonts;
extern symbol_table * bp_alignments;
extern symbol_table * bp_directions;
extern symbol_table * bp_referenceTypes;
extern symbol_table * bp_colors;
extern symbol_table * bp_rotations;
extern symbol_table * bp_mirrorings;
/*! \} */

/*!
 * \name color values for the book parser
 *
 * \{
 */
extern const bp_Color bp_black;
extern const bp_Color bp_navy;
extern const bp_Color bp_green;
extern const bp_Color bp_teal;
extern const bp_Color bp_maroon;
extern const bp_Color bp_purple;
extern const bp_Color bp_olive;
extern const bp_Color bp_silver;
extern const bp_Color bp_gray;
extern const bp_Color bp_blue;
extern const bp_Color bp_lime;
extern const bp_Color bp_aqua;
extern const bp_Color bp_red;
extern const bp_Color bp_fuchsia;
extern const bp_Color bp_yellow;
extern const bp_Color bp_white;
/*! \} */

/*!
 * \ingroup books
 * \brief   initialises the symbol tables for the book parser
 *          Initialises (fills) the symbol tables for the book parser
 *
 */
void bp_init_symbols();

#endif /* ENCYCLOPEDIA */

#endif // ndef __BOOKS_SYMBOLS_H__
