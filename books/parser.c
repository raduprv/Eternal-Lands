#ifdef ENCYCLOPEDIA

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../global.h"
#include "../symbol_table.h"
#include "../textures.h"
#include "../errors.h"
#include "types.h"
#include "symbols.h"
#include "parser.h"

const bp_BlockOutsideAttributes bp_defaultBlockOutsideAttributes = {
		/* marginBefore: */ 0.0f,
		/* marginAfter: */ 0.0f,
		/* marginStart: */ 0.0f,
		/* marginEnd: */ 0.0f,
		/* next: */ NULL
	};

const bp_BlockBorderAttributes bp_defaultBlockBorderAttributes = {
		/* borderBefore: */ 0,
		/* borderAfter: */ 0,
		/* borderStart: */ 0,
		/* borderEnd: */ 0,
		/* borderBeforeColor: */ { 0.0, 0.0, 0.0 },
		/* borderAfterColor: */ { 0.0, 0.0, 0.0 },
		/* borderStartColor: */ { 0.0, 0.0, 0.0 },
		/* borderEndColor: */ { 0.0, 0.0, 0.0 },
		/* next: */ NULL
	};

const bp_BlockInsideAttributes bp_defaultBlockInsideAttributes = {
		/* paddingBefore: */ 0.0f,
		/* paddingAfter: */ 0.0f,
		/* paddingStart: */ 0.0f,
		/* paddingEnd: */ 0.0f,
		/* fontSize: */ 1.0f,
		/* alignment: */ BPAL_JUSTIFIED,
		/* minWordSpacing: */ 3.0f,
		/* maxWordSpacing: */ 6.0f,
		/* minLetterSpacing: */ 0.0f,
		/* maxLetterSpacing: */ 3.0f,
		/* lineSpacing: */ 1.0f,
		/* next: */ NULL
	};

const bp_InlineAttributes bp_defaultInlineAttributes = {
		/* color: */ { 0.0, 0.0, 0.0 },
		/* next: */ NULL
	};

const bp_FloatAttributes bp_defaultFloatAttributes = {
		/* align: */ BPAL_CENTER
	};

#define ADD(c)  bp_addNode(parent,&(c)->node)
#define DISCARD(n) bp_discardElement(n)
#define ELEM(n) (bp_determineElement((n)->name))
#define ATTR(n) (bp_determineAttribute((n)->name))

void bp_initNode(bp_Node * node, bp_Element elem);
void bp_addNode(bp_Node * parent, bp_Node * child);
void bp_discardElement(xmlNodePtr node);
bp_Element bp_determineElement(const char * name);
void bp_pushBlockOutside(bp_Context * context);
void bp_pushBlockBorder(bp_Context * context);
void bp_pushBlockInside(bp_Context * context);
void bp_pushInline(bp_Context * context);
void bp_popBlockOutside(bp_Context * context);
void bp_popBlockBorder(bp_Context * context);
void bp_popBlockInside(bp_Context * context);
void bp_popInline(bp_Context * context);
void bp_collectLabels(bp_Context * context, bp_Node * node);
void bp_linkReferences(bp_Context * context, bp_Node * node);

bp_Document * bp_parseFile(bp_Context * context, const char * name);
bp_Document * bp_parseDoc(bp_Context * context, xmlDocPtr doc);
bp_Book     * bp_parseBook(bp_Context * context, xmlNodePtr node);
bp_Page     * bp_parsePage(bp_Context * context, xmlNodePtr node);
bp_Block    * bp_parseBlock(bp_Context * context, xmlNodePtr node);
bp_Image    * bp_parseImage(bp_Context * context, xmlNodePtr node);
bp_Table    * bp_parseTable(bp_Context * context, xmlNodePtr node);
bp_Label    * bp_parseLabel(bp_Context * context, xmlNodePtr node);
void          bp_parseNavRef(bp_Context * context, xmlNodePtr node, bp_Page * page);
bp_Text     * bp_parseText(bp_Context * context, xmlNodePtr node);
bp_Ref      * bp_parseRef(bp_Context * context, xmlNodePtr node);
bp_Inline   * bp_parseInline(bp_Context * context, xmlNodePtr node);
bp_Cell     * bp_parseCell(bp_Context * context, xmlNodePtr node, int row, int col);
bp_Caption  * bp_parseCaption(bp_Context * context, xmlNodePtr node);
void          bp_parseInlineContent(bp_Context * context, bp_Node * const parent, xmlNodePtr node);

void bp_parseBlockOutsideAttribute(bp_Context * context, bp_BlockOutsideAttributes * dest, xmlAttrPtr attr);
void bp_parseBlockBorderAttribute(bp_Context * context, bp_BlockBorderAttributes * dest, xmlAttrPtr attr);
void bp_parseBlockInsideAttribute(bp_Context * context, bp_BlockInsideAttributes * dest, xmlAttrPtr attr);
void bp_parseInlineAttribute(bp_Context * context, bp_InlineAttributes * dest, xmlAttrPtr attr);
void bp_parseFloatAttribute(bp_Context * context, bp_FloatAttributes * _float, xmlAttrPtr attr);

int          bp_parseInt(bp_Context * context, xmlNodePtr data);
char         bp_parseBool(bp_Context * context, xmlNodePtr data);
float        bp_parseFloat(bp_Context * context, xmlNodePtr data);
bp_Mirroring bp_parseMirroring(bp_Context * context, xmlNodePtr data);
bp_Rotation  bp_parseRotation(bp_Context * context, xmlNodePtr data);
bp_Color     bp_parseColor(bp_Context * context, xmlNodePtr data);
bp_Alignment bp_parseAlignment(bp_Context * context, xmlNodePtr data);

void bp_initNode(bp_Node * node, bp_Element elem) {
	node->next = NULL;
	node->children = NULL;
	node->children_tail = NULL;
	node->element = elem;
}

void bp_addNode(bp_Node * parent, bp_Node * child) {
	bp_Node * const tail = parent->children_tail;
	if (tail) {
		tail->next = child;
	} else {
		parent->children = child;
	}
	parent->children_tail = child;
}

void bp_discardElement(xmlNodePtr node) {
	char errmsg[500];
	if (ELEM(node) == BPE_TEXT) {
		xmlChar * text = (void *) node->children;
		xmlChar c;
		char whitespace = 1;
		if (!text) return;
		while ((c = *text++) && whitespace) {
			switch(c) {
				case ' ':
				case '\n':
				case '\r':
				case '\t':
				case '\v':
					break;
				default:
					whitespace = 0;
			}
		}
		if (whitespace) return;
	}
#ifndef OSX
	snprintf(errmsg, sizeof(errmsg), "%s:%d: element of type \"%s\""
			" not allowed here!\n", node->doc->name, node->line, node->name);
#else
        snprintf(errmsg, sizeof(errmsg), "%s: element of type \"%s\""
                        " not allowed here!\n", node->doc->name, node->name);
#endif
	LOG_ERROR(errmsg);
	exit(1);
}

bp_Element bp_determineElement(const char * name) {
	st_data * const data = st_lookup(bp_elements, name);
	return (data)? (bp_Element) data->num : BPE_NONE;
}

bp_Attribute bp_determineAttribute(const char * name) {
	st_data * const data = st_lookup(bp_attributes, name);
	return (data)? (bp_Attribute) data->num : BPE_NONE;
}

void bp_pushBlockOutside(bp_Context * context) {
	bp_BlockOutsideAttributes * result = malloc(sizeof(bp_BlockOutsideAttributes));
	*result = *(context->blockOutside);
	result->next = context->blockOutside;
	context->blockOutside = result;
}

void bp_pushBlockBorder(bp_Context * context) {
	bp_BlockBorderAttributes * result = malloc(sizeof(bp_BlockBorderAttributes));
	*result = *(context->blockBorder);
	result->next = context->blockBorder;
	context->blockBorder = result;
}

void bp_pushBlockInside(bp_Context * context) {
	bp_BlockInsideAttributes * result = malloc(sizeof(bp_BlockInsideAttributes));
	*result = *(context->blockInside);
	result->next = context->blockInside;
	context->blockInside = result;
}

void bp_pushInline(bp_Context * context) {
	bp_InlineAttributes * result = malloc(sizeof(bp_InlineAttributes));
	*result = *(context->inlineAttr);
	result->next = context->inlineAttr;
	context->inlineAttr = result;
}

void bp_popBlockOutside(bp_Context * context) {
	bp_BlockOutsideAttributes * const top = context->blockOutside;
	context->blockOutside = top->next;
	free(top);
}

void bp_popBlockBorder(bp_Context * context) {
	bp_BlockBorderAttributes * const top = context->blockBorder;
	context->blockBorder = top->next;
	free(top);
}

void bp_popBlockInside(bp_Context * context) {
	bp_BlockInsideAttributes * const top = context->blockInside;
	context->blockInside = top->next;
	free(top);
}

void bp_popInline(bp_Context * context) {
	bp_InlineAttributes * const top = context->inlineAttr;
	context->inlineAttr = top->next;
	free(top);
}

void bp_collectLabels(bp_Context * context, bp_Node * node) {
	bp_Label * label;
	do {
		switch (node->element) {
			case BPE_PAGE:
				context->page = (void *) node;
				break;
			case BPE_LABEL:
				label = (void *) node;
				 // label->page still holds the label name
				st_addptr(context->labels, (char *) label->page, label);
				label->page = context->page;
				break;
			default:;
		}
		if (node->children) bp_collectLabels(context, node->children);
		node = node->next;
	} while(node);
}

void bp_linkReferences(bp_Context * context, bp_Node * node) {
	bp_Ref * ref; st_data * stdata;
	do {
		switch (node->element) {
			case BPE_REF:
				ref = (void *) node;
				// ref->label still holds the label name
				stdata = st_lookup(context->labels, (char *) ref->label);
				if (stdata) {
					ref->label = stdata->ptr;
				} else {
					char errmsg[500];
					snprintf(errmsg, sizeof(errmsg), "book parser: unknown label \"%s\" "
							"in reference\n", (char *) ref->label);
					LOG_ERROR(errmsg);
					exit(1);
				}
				break;
			default:;
		}
		if (node->children) bp_linkReferences(context, node->children);
		node = node->next;
	} while(node);
}

bp_Document * bp_parseFile(bp_Context * context, const char * name) {
	xmlDocPtr doc = xmlParseFile(name);
	bp_Document * result = bp_parseDoc(context, doc);
	xmlFreeDoc(doc);
	return result;
}

bp_Document * bp_parseDoc(bp_Context * context, xmlDocPtr doc) {
	bp_Document * result = malloc(sizeof(bp_Document));
	xmlNodePtr rootNode;
	context->doc = (void *) result;
	context->nLabels = 0;
	result->root = NULL;

	for (rootNode = doc->children; rootNode; rootNode = rootNode->next) {
		switch (ELEM(rootNode)) {
			case BPE_BOOK:
				result->root = (bp_Node *) bp_parseBook(context, rootNode);
				break;
			default:;
		}
	}

	if (!result->root)  {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "%s: root element must be \"book\""
				, doc->name);
		LOG_ERROR(errmsg);
		exit(1);
	}

	context->labels = st_create(context->nLabels + 1);
	st_addptr(context->labels, "", NULL); // allow for empty "to" attributes
	bp_collectLabels(context, result->root);
	st_commit(context->labels);
	bp_linkReferences(context, result->root);
	st_destroy(context->labels);
	return result;
}

bp_Book * bp_parseBook(bp_Context * context, xmlNodePtr node) {
	bp_Book * result = malloc(sizeof(bp_Book));
	bp_Node * const parent = &result->node;
	bp_Node * page;
	xmlAttrPtr attr;
	xmlNodePtr child;
	bp_Page ** pages;
	st_data * fontSym;

	context->blockOutside = (void *) &bp_defaultBlockOutsideAttributes;
	context->blockBorder  = (void *) &bp_defaultBlockBorderAttributes;
	context->blockInside  = (void *) &bp_defaultBlockInsideAttributes;
	context->inlineAttr      = (void *) &bp_defaultInlineAttributes;

	bp_pushBlockOutside(context);
	bp_pushBlockBorder(context);
	bp_pushBlockInside(context);
	bp_pushInline(context);

	bp_initNode(&result->node, BPE_BOOK);
	result->width = 528;
	result->height = 320;
	result->background = load_texture_cache_deferred("textures/book1.bmp",0);
	fontSym = st_lookup (bp_fonts, "default");
	if (fontSym != NULL) {
		result->fontFace = fontSym->num;
	} else {
		LOG_ERROR("FATAL: default font missing in font symbol table\n");
		exit(1);
	}
	result->layout = BPL_BOOK;
	result->blockProgression = BPD_DOWN;
	result->inlineProgression = BPD_RIGHT;
	result->pages = NULL;
	result->nPages = 0;

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseBlockOutsideAttribute(context, context->blockOutside, attr);
		bp_parseBlockBorderAttribute(context, context->blockBorder, attr);
		bp_parseBlockInsideAttribute(context, context->blockInside, attr);
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	for (child = node->children; child; child = child->next) {
		switch(ELEM(child)) {
			case BPE_PAGE:
				result->nPages++;
				ADD(bp_parsePage(context, child));
				break;
			default:
				DISCARD(child);
		}
	}

	bp_popBlockOutside(context);
	bp_popBlockBorder(context);
	bp_popBlockInside(context);
	bp_popInline(context);

	/* create page index */
	pages = calloc(result->nPages, sizeof(bp_Page *));
	result->pages = pages;
	for (page = parent->children; page; page = page->next) {
		*pages++ = (bp_Page *) page;
	}
	
	return result;
}

bp_Page * bp_parsePage(bp_Context * context, xmlNodePtr node) {
	bp_Page * result = malloc(sizeof(bp_Page));
	bp_Node * const parent = &result->node;
	xmlAttrPtr attr;
	xmlNodePtr child;

	bp_pushBlockOutside(context);
	bp_pushBlockBorder(context);
	bp_pushBlockInside(context);
	bp_pushInline(context);

	bp_initNode(&result->node, BPE_PAGE);
	result->title = NULL;

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseBlockOutsideAttribute(context, context->blockOutside, attr);
		bp_parseBlockBorderAttribute(context, context->blockBorder, attr);
		bp_parseBlockInsideAttribute(context, context->blockInside, attr);
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	for (child = node->children; child; child = child->next) {
		switch(ELEM(child)) {
			case BPE_TITLE:
				if (!result->title) {
					xmlNodePtr grandchild = child->children;
					if (ELEM(grandchild) == BPE_TEXT) {
						int len = strlen(grandchild->content);
						char * title_string = malloc(++len);
						memcpy(title_string, grandchild->content, len);
						result->title = title_string;
					} else {
						DISCARD(grandchild);
					}
				} else {
					DISCARD(child);
				}
				break;
			case BPE_REF:
				bp_parseNavRef(context, child, result);
				break;
			case BPE_BLOCK:
				ADD(bp_parseBlock(context, child));
				break;
			case BPE_IMAGE:
				ADD(bp_parseImage(context, child));
				break;
			case BPE_TABLE:
				ADD(bp_parseTable(context, child));
				break;
			case BPE_LABEL:
				ADD(bp_parseLabel(context, child));
				break;
			default:
				DISCARD(child);
		}
	}

	bp_popBlockOutside(context);
	bp_popBlockBorder(context);
	bp_popBlockInside(context);
	bp_popInline(context);

	return result;
}

bp_Block * bp_parseBlock(bp_Context * context, xmlNodePtr node) {
	bp_Block * result = malloc(sizeof(bp_Block));
	xmlAttrPtr attr;

	bp_pushInline(context);

	bp_initNode(&result->node, BPE_BLOCK);
	result->blockOutside = *(context->blockOutside);
	result->blockBorder  = *(context->blockBorder);
	result->blockInside  = *(context->blockInside);

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseBlockOutsideAttribute(context, &result->blockOutside, attr);
		bp_parseBlockBorderAttribute(context, &result->blockBorder, attr);
		bp_parseBlockInsideAttribute(context, &result->blockInside, attr);
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	bp_parseInlineContent(context, &result->node, node);

	bp_popInline(context);

	return result;
}


bp_Image * bp_parseImage(bp_Context * context, xmlNodePtr node) {
	bp_Image * result = malloc(sizeof(bp_Image));
	xmlAttrPtr attr;

	bp_initNode(&result->node, BPE_IMAGE);
	result->_float = bp_defaultFloatAttributes;
	result->width = 32;
	result->height = 32;
	result->mirror = 0;
	result->rotate = 0;
	result->left = 0.0f;
	result->right = 1.0f;
	result->top = 0.0f;
	result->bottom = 1.0f;
	result->texture = 0;

	for (attr = node->properties; attr; attr = attr->next) {
		switch(ATTR(attr)) {
			case BPA_WIDTH:
				result->width = bp_parseInt(context, attr->children);
				break;
			case BPA_HEIGHT:
				result->height = bp_parseInt(context, attr->children);
				break;
			case BPA_MIRROR:
				result->mirror = bp_parseMirroring(context, attr->children);
				break;
			case BPA_ROTATE:
				result->rotate = bp_parseRotation(context, attr->children);
				break;
			case BPA_TEX_LEFT:
				result->left = bp_parseFloat(context, attr->children);
				break;
			case BPA_TEX_RIGHT:
				result->right = bp_parseFloat(context, attr->children);
				break;
			case BPA_TEX_TOP:
				result->top = bp_parseFloat(context, attr->children);
				break;
			case BPA_TEX_BOTTOM:
				result->bottom = bp_parseFloat(context, attr->children);
				break;
			case BPA_TEX_FILE:
				result->texture = load_texture_cache_deferred(attr->children->content, 0);
				break;
			default:
				bp_parseFloatAttribute(context, &result->_float, attr);
		}
	}

	return result;
}

bp_Table * bp_parseTable(bp_Context * context, xmlNodePtr node) {
	bp_Table * result = malloc(sizeof(bp_Table));
	bp_Node * const parent = &result->node;
	xmlAttrPtr attr;
	xmlNodePtr child, grand;

	bp_pushBlockBorder(context);
	bp_pushBlockInside(context);
	bp_pushInline(context);

	bp_initNode(&result->node, BPE_TABLE);
	result->_float = bp_defaultFloatAttributes;
	result->blockOutside = *(context->blockOutside);
	result->caption = NULL;
	result->nCols = 0;
	result->nRows = 0;
	
	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseFloatAttribute(context, &result->_float, attr);
		bp_parseBlockOutsideAttribute(context, &result->blockOutside, attr);
		bp_parseBlockBorderAttribute(context, context->blockBorder, attr);
		bp_parseBlockInsideAttribute(context, context->blockInside, attr);
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	result->blockBorder = *(context->blockBorder); // shared with children

	for (child = node->children; child; child = child->next) {
		switch(ELEM(child)) {
			case BPE_TR: result->nRows++; break;
			case BPE_TC: result->nCols++; break;
			case BPE_CAPTION:
				result->caption = bp_parseCaption(context, child);
			default: DISCARD(child);
		}
	}

	if (result->nRows) {
		int * colHeights = NULL; 
		int col, row = 0;
		result->nCols = 0;

		for (child = node->children; child; child = child->next) {
			if(ELEM(child) == BPE_TR) {
				bp_pushBlockBorder(context);
				bp_pushBlockInside(context);
				bp_pushInline(context);

				for (attr = child->properties; attr; attr = attr->next) {
					bp_parseBlockBorderAttribute(context, context->blockBorder, attr);
					bp_parseBlockInsideAttribute(context, context->blockInside, attr);
					bp_parseInlineAttribute(context, context->inlineAttr, attr);
				}
				
				col = 0;
				for (grand = child->children; grand; grand = grand->next) {
					while ((col < result->nCols) && (colHeights[col] >= row)) col++;
							
					if(ELEM(grand) == BPE_TD) {
						bp_Cell * cell = bp_parseCell(context, grand, row, col);

						ADD(cell);
						
						if (cell->endCol >= result->nCols) {
							result->nCols = cell->endCol + 1;
							colHeights = realloc(colHeights, result->nCols * sizeof(int));
						}
						
						for (col=cell->startCol; col <= cell->endCol; col++) {
							colHeights[col] = cell->endRow;
						}
					} else {
						DISCARD(grand);
					}
				}

				bp_popBlockBorder(context);
				bp_popBlockInside(context);
				bp_popInline(context);
				row++;
			}
		}

		if(colHeights) free(colHeights);
	} else if (result->nCols) {
		int * rowWidths = NULL; 
		int col = 0, row;
		result->nRows = 0;

		for (child = node->children; child; child = child->next) {
			if(ELEM(child) == BPE_TC) {
				bp_pushBlockBorder(context);
				bp_pushBlockInside(context);
				bp_pushInline(context);

				for (attr = child->properties; attr; attr = attr->next) {
					bp_parseBlockBorderAttribute(context, context->blockBorder, attr);
					bp_parseBlockInsideAttribute(context, context->blockInside, attr);
					bp_parseInlineAttribute(context, context->inlineAttr, attr);
				}
				
				row = 0;
				for (grand = child->children; grand; grand = grand->next) {
					while ((row < result->nRows) && (rowWidths[row] >= col)) row++;
							
					if(ELEM(grand) == BPE_TD) {
						bp_Cell * cell = bp_parseCell(context, grand, row, col);

						ADD(cell);
						
						if (cell->endRow >= result->nRows) {
							result->nRows = cell->endRow + 1;
							rowWidths = realloc(rowWidths, result->nRows * sizeof(int));
						}
						
						for (row=cell->startRow; row <= cell->endRow; row++) {
							rowWidths[row] = cell->endCol;
						}
					} else {
						DISCARD(grand);
					}
				}

				bp_popBlockBorder(context);
				bp_popBlockInside(context);
				bp_popInline(context);
				col++;
			}
		}

		if(rowWidths) free(rowWidths);
	}

	bp_popBlockBorder(context);
	bp_popBlockInside(context);
	bp_popInline(context);

	return result;
}


bp_Label * bp_parseLabel(bp_Context * context, xmlNodePtr node) {
	bp_Label * result = malloc(sizeof(bp_Label));
	char errmsg[500];
	
	bp_initNode(&result->node, BPE_LABEL);
	result->page = (void *) xmlGetProp(node, "name");

	if (!result->page) {
#ifndef OSX
		snprintf(errmsg, sizeof(errmsg), "%s:%d: \"label\" element "
				"requires \"name\" attribute", node->doc->name, node->line);
#else
                snprintf(errmsg, sizeof(errmsg), "%s: \"label\" element "
                                "requires \"name\" attribute", node->doc->name);
#endif
		LOG_ERROR(errmsg);
		exit(1);
	}

	context->nLabels++;
	
	return result;
}

void bp_parseNavRef(bp_Context * context, xmlNodePtr node, bp_Page * page) {
	xmlChar * cdata = xmlGetProp(node, "type");
	if (cdata) {
		st_data * stdata = st_lookup(bp_referenceTypes, cdata);

		if (stdata) {
			bp_Ref * result = bp_parseRef(context, node);
			page->nav[stdata->num] = result;
		} else {
			char errmsg[500];
#ifndef OSX
			snprintf(errmsg, sizeof(errmsg), "%s:%d: Unknown reference type \"%s\"\n"
					, node->doc->name, node->line, cdata);
#else
                        snprintf(errmsg, sizeof(errmsg), "%s: Unknown reference type \"%s\"\n"
                                        , node->doc->name, cdata);
#endif
			LOG_ERROR(errmsg);
			// non-fatal
		}
	} else {
		char errmsg[500];
#ifndef OSX
		snprintf(errmsg, sizeof(errmsg), "%s:%d: only navigation references allowed"
				" outside block elements\n", node->doc->name, node->line);
#else
                snprintf(errmsg, sizeof(errmsg), "%s: only navigation references allowed"
                                " outside block elements\n", node->doc->name);
#endif
		LOG_ERROR(errmsg);
		// non-fatal
	}
}

bp_Text * bp_parseText(bp_Context * context, xmlNodePtr node) {
	bp_Text * result = malloc(sizeof(bp_Text));
	int len;

	bp_initNode(&result->node, BPE_TEXT);
	result->inlineAttr = *(context->inlineAttr);
	len = strlen(node->content);
	result->content = malloc(++len);
	memcpy(result->content, node->content, len);

	return result;
}

bp_Ref * bp_parseRef(bp_Context * context, xmlNodePtr node) {
	bp_Ref * result = malloc(sizeof(bp_Ref));
	xmlAttrPtr attr;

	bp_pushInline(context);

	bp_initNode(&result->node, BPE_REF);
	result->label = (void *) xmlGetProp(node, "to");

	if (!result->label) {
		char errmsg[500];
#ifndef OSX
		snprintf(errmsg, sizeof(errmsg), "%s:%d: \"ref\" element requires \"to\" "
				"attribute", node->doc->name, node->line);
#else
                snprintf(errmsg, sizeof(errmsg), "%s: \"ref\" element requires \"to\" "
                                "attribute", node->doc->name);
#endif
		LOG_ERROR(errmsg);
		exit(1);
	}

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	bp_parseInlineContent(context, &result->node, node);

	bp_popInline(context);

	return result;
}

bp_Inline * bp_parseInline(bp_Context * context, xmlNodePtr node) {
	bp_Inline * result = malloc(sizeof(bp_Inline));
	xmlAttrPtr attr;

	bp_pushInline(context);

	bp_initNode(&result->node, BPE_INLINE);

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
	}

	bp_parseInlineContent(context, &result->node, node);

	bp_popInline(context);

	return result;
}

bp_Cell * bp_parseCell(bp_Context * context, xmlNodePtr node, int row, int col) {
	bp_Cell * result = malloc(sizeof(bp_Cell));
	xmlAttrPtr attr;

	bp_pushInline(context);

	bp_initNode(&result->node, BPE_TD);
	result->blockBorder  = *(context->blockBorder);
	result->blockInside  = *(context->blockInside);
	result->startRow     = row;
	result->startCol     = col;
	result->endRow       = row;
	result->endCol       = col;

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseBlockBorderAttribute(context, &result->blockBorder, attr);
		bp_parseBlockInsideAttribute(context, &result->blockInside, attr);
		bp_parseInlineAttribute(context, context->inlineAttr, attr);
		switch(ATTR(attr)) {
			case BPA_ROWSPAN:
				result->endRow = bp_parseInt(context, attr->children) + row - 1;
				break;
			case BPA_COLSPAN:
				result->endCol = bp_parseInt(context, attr->children) + col - 1;
				break;
			default:;
		}
	}

	bp_parseInlineContent(context, &result->node, node);

	bp_popInline(context);

	return result;
}

bp_Caption * bp_parseCaption(bp_Context * context, xmlNodePtr node) {
	bp_Caption * result = malloc(sizeof(bp_Caption));
	xmlAttrPtr attr;
	xmlNodePtr child;

	bp_initNode(&result->node, BPE_CAPTION);
	result->inlineAttr = *(context->inlineAttr);

	for (attr = node->properties; attr; attr = attr->next) {
		bp_parseInlineAttribute(context, &result->inlineAttr, attr);
	}

	child = node->children;
	if (ELEM(child) == BPE_TEXT) {
		int len;

		len = strlen(child->content);
		result->text = malloc(++len);
		memcpy(result->text, child->children, len);
	} else {
		DISCARD(child);
	}
	
	return result;
}

void bp_parseInlineContent(bp_Context * context, bp_Node * const parent, xmlNodePtr node) {
	xmlNodePtr child;

	for (child = node->children; child; child = child->next) {
		switch (ELEM(child)) {
			case BPE_TEXT:
				ADD(bp_parseText(context, child));
				break;
			case BPE_INLINE:
				ADD(bp_parseInline(context, child));
				break;
			case BPE_LABEL:
				ADD(bp_parseLabel(context, child));
				break;
			case BPE_REF:
				ADD(bp_parseRef(context, child));
				break;
			default: 
				DISCARD(child);
		}
	}
}

void bp_parseBlockOutsideAttribute(bp_Context * context, bp_BlockOutsideAttributes * dest, xmlAttrPtr attr) {
	float tmp;
	switch(ATTR(attr)) {
		case BPA_MARGIN:
			tmp = bp_parseFloat(context, attr->children);
			dest->marginBefore = tmp;
			dest->marginAfter  = tmp;
			dest->marginStart  = tmp;
			dest->marginEnd    = tmp;
			break;
		case BPA_MARGIN_BEFORE:
			dest->marginBefore = bp_parseFloat(context, attr->children);
			break;
		case BPA_MARGIN_AFTER:
			dest->marginAfter = bp_parseFloat(context, attr->children);
			break;
		case BPA_MARGIN_START:
			dest->marginStart = bp_parseFloat(context, attr->children);
			break;
		case BPA_MARGIN_END:
			dest->marginEnd = bp_parseFloat(context, attr->children);
			break;
		default:;
	}
}

void bp_parseBlockBorderAttribute(bp_Context * context, bp_BlockBorderAttributes * dest, xmlAttrPtr attr) {
	char tmpb; bp_Color tmpc;
	switch(ATTR(attr)) {
		case BPA_BORDER:
			tmpb = bp_parseBool(context, attr->children);
			dest->borderBefore = tmpb;
			dest->borderAfter  = tmpb;
			dest->borderStart  = tmpb;
			dest->borderEnd    = tmpb;
			break;
		case BPA_BORDER_BEFORE:
			dest->borderBefore = bp_parseBool(context, attr->children);
			break;
		case BPA_BORDER_AFTER:
			dest->borderAfter = bp_parseBool(context, attr->children);
			break;
		case BPA_BORDER_START:
			dest->borderStart = bp_parseBool(context, attr->children);
			break;
		case BPA_BORDER_END:
			dest->borderEnd = bp_parseBool(context, attr->children);
			break;
		case BPA_BORDER_COLOR:
			tmpc = bp_parseColor(context, attr->children);
			dest->borderBeforeColor = tmpc;
			dest->borderAfterColor  = tmpc;
			dest->borderStartColor  = tmpc;
			dest->borderEndColor    = tmpc;
			break;
		case BPA_BORDER_BEFORE_COLOR:
			dest->borderBeforeColor = bp_parseColor(context, attr->children);
			break;
		case BPA_BORDER_AFTER_COLOR:
			dest->borderAfterColor = bp_parseColor(context, attr->children);
			break;
		case BPA_BORDER_START_COLOR:
			dest->borderStartColor = bp_parseColor(context, attr->children);
			break;
		case BPA_BORDER_END_COLOR:
			dest->borderEndColor = bp_parseColor(context, attr->children);
			break;
		default:;
	}
}

void bp_parseBlockInsideAttribute(bp_Context * context, bp_BlockInsideAttributes * dest, xmlAttrPtr attr) {
	float tmp;
	switch(ATTR(attr)) {
		case BPA_PADDING:
			tmp = bp_parseFloat(context, attr->children);
			dest->paddingBefore = tmp;
			dest->paddingAfter  = tmp;
			dest->paddingStart  = tmp;
			dest->paddingEnd    = tmp;
			break;
		case BPA_PADDING_BEFORE:
			dest->paddingBefore = bp_parseFloat(context, attr->children);
			break;
		case BPA_PADDING_AFTER:
			dest->paddingAfter = bp_parseFloat(context, attr->children);
			break;
		case BPA_PADDING_START:
			dest->paddingStart = bp_parseFloat(context, attr->children);
			break;
		case BPA_PADDING_END:
			dest->paddingEnd = bp_parseFloat(context, attr->children);
			break;
		case BPA_FONT_SIZE:
			dest->fontSize = bp_parseFloat(context, attr->children);
			break;
		case BPA_ALIGNMENT:
			dest->alignment = bp_parseAlignment(context, attr->children);
			break;
		case BPA_WORD_SPACING:
			tmp = bp_parseFloat(context, attr->children);
			dest->minWordSpacing = tmp;
			dest->maxWordSpacing = tmp;
			break;
		case BPA_MIN_WORD_SPACING:
			dest->minWordSpacing = bp_parseFloat(context, attr->children);
			break;
		case BPA_MAX_WORD_SPACING:
			dest->maxWordSpacing = bp_parseFloat(context, attr->children);
			break;
		case BPA_LETTER_SPACING:
			tmp = bp_parseFloat(context, attr->children);
			dest->minLetterSpacing = tmp;
			dest->maxLetterSpacing = tmp;
			break;
		case BPA_MIN_LETTER_SPACING:
			dest->minLetterSpacing = bp_parseFloat(context, attr->children);
			break;
		case BPA_MAX_LETTER_SPACING:
			dest->maxLetterSpacing = bp_parseFloat(context, attr->children);
			break;
		case BPA_LINE_SPACING:
			dest->lineSpacing = bp_parseFloat(context, attr->children);
		default:;
	}
}

void bp_parseInlineAttribute(bp_Context * context, bp_InlineAttributes * dest, xmlAttrPtr attr) {
	switch(ATTR(attr)) {
		case BPA_COLOR:
			dest->color = bp_parseColor(context, attr->children);
			break;
		default:;
	}
}

void bp_parseFloatAttribute(bp_Context * context, bp_FloatAttributes * _float, xmlAttrPtr attr) {
	switch(ATTR(attr)) {
		case BPA_ALIGN:
			_float->align = bp_parseAlignment(context, attr->children);
			switch (_float->align) {
				case BPAL_START:
				case BPAL_CENTER:
				case BPAL_END:
					break;
				default: {
						char errmsg[500];
#ifndef OSX
						snprintf(errmsg, sizeof(errmsg), "%s:%d: invalid value for \"align"
								"\" attribute\n", attr->doc->name, attr->parent->line);
#else
                                                snprintf(errmsg, sizeof(errmsg), "%s: invalid value for \"align"
                                                                "\" attribute\n", attr->doc->name);
#endif
						LOG_ERROR(errmsg);
						exit(1);
					}
			}
			break;
		default:;
	}
}

int bp_parseInt(bp_Context * context, xmlNodePtr data) {
	return atoi(data->content);
}

char bp_parseBool(bp_Context * context, xmlNodePtr data) {
	return (atoi(data->content))? 1 : 0;
}

float bp_parseFloat(bp_Context * context, xmlNodePtr data) {
	return atof(data->content);
}

bp_Mirroring bp_parseMirroring(bp_Context * context, xmlNodePtr data) {
	st_data * const stdata = st_lookup(bp_mirrorings, data->content);
	if (stdata) {
		return (bp_Mirroring) stdata->num;
	} else {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "error: invalid value for \"mirror\" "
				"attribute: %s\n", data->content);
		LOG_ERROR(errmsg);
		exit(1);
	}
}

bp_Rotation  bp_parseRotation(bp_Context * context, xmlNodePtr data) {
	st_data * const stdata = st_lookup(bp_rotations, data->content);
	if (stdata) {
		return (bp_Rotation) stdata->num;
	} else {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "error: invalid value for \"rotate\" "
				"attribute: %s\n", data->content);
		LOG_ERROR(errmsg);
		exit(1);
	}
}

bp_Color bp_parseColor(bp_Context * context, xmlNodePtr data) {
	char * cdata = data->content;
	int tmp;
	bp_Color result;
	if (*cdata == '#') {
		switch (strlen(++cdata)) {
			case 3:
				sscanf(cdata, "%03x", &tmp);
				result.r = (float)((tmp & 0xf00) >> 8) / 15.0f;
				result.g = (float)((tmp & 0x0f0) >> 4) / 15.0f;
				result.b = (float)((tmp & 0x00f) >> 0) / 15.0f;
				break;
			case 6:
				sscanf(cdata, "%06x", &tmp);
				result.r = (float)((tmp & 0xff0000) >> 16) / 255.0f;
				result.g = (float)((tmp & 0x00ff00) >>  8) / 255.0f;
				result.b = (float)((tmp & 0x0000ff) >>  0) / 255.0f;
				break;
			default: {
					char errmsg[500];
					snprintf(errmsg, sizeof(errmsg), "error: invalid color value: %s\n", 
							--cdata);
					LOG_ERROR(errmsg);
					exit(1);
				}
		}
	} else {
		st_data * const stdata = st_lookup(bp_colors, cdata);
		if (stdata) {
			return *((bp_Color *) stdata->ptr);
		} else {
			char errmsg[500];
			snprintf(errmsg, sizeof(errmsg), "error: invalid color value: %s\n", 
					cdata);
			LOG_ERROR(errmsg);
			exit(1);
		}
	}
	return result;
}

bp_Alignment bp_parseAlignment(bp_Context * context, xmlNodePtr data) {
	st_data * const stdata = st_lookup(bp_alignments, data->content);
	if (stdata) {
		return (bp_Alignment) stdata->num;
	} else {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "error: invalid value for \"align\" "
				"or \"alignment\" attribute: \"%s\"\n", data->content);
		LOG_ERROR(errmsg);
		exit(1);
	}
}

#endif /* ENCYCLOPEDIA */
