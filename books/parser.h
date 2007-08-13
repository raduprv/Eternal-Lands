/*!
 * \file
 * \brief parser for new book format
 */

#ifndef __BOOKS_PARSER_H__
#define __BOOKS_PARSER_H__

#ifdef ENCYCLOPEDIA

#include "../symbol_table.h"
#include "types.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct _bp_Context {
	bp_Document * doc;
	bp_Page * page;
	int nLabels;
	bp_BlockOutsideAttributes * blockOutside;
	bp_BlockBorderAttributes * blockBorder;
	bp_BlockInsideAttributes * blockInside;
	bp_InlineAttributes * inlineAttr;
	symbol_table * labels;
} bp_Context;

/*!
 * \brief parses a file with the new book format
 *        Parses a file with the new book format.
 *
 * \param context pointer to a \see bp_Context structure to use during parsing
 * \param name    a string containing the path to the file to parse
 * \retval        a pointer to the parsed document in a \see bp_Document structure
 */
bp_Document * bp_parseFile(bp_Context * context, const char * name);

/*!
 * \brief parses a libxml2 document tree with the new book format
 *        Parses a libxml2 document tree with the new book format.
 *
 * \param context pointer to a \see bp_Context structure to use during parsing
 * \param doc     xmlDoxPtr to the document tree
 * \retval        a pointer to the parsed document in a \see bp_Document structure
 */
bp_Document * bp_parseDoc(bp_Context * context, xmlDocPtr doc);

#endif /* ENCYCLOPEDIA */

#endif // ndef __BOOKS_PARSER_H__
