/*!
 * \file 
 * \ingroup books
 * \brief types used for the new client book format
 */

#ifndef __BOOKS_TYPES_H__
#define __BOOKS_TYPES_H__

#ifdef ENCYCLOPEDIA

/*! \name the xml elements */
typedef enum { BPE_NONE, BPE_BOOK, BPE_PAGE, BPE_REF, BPE_BLOCK, BPE_IMAGE, 
		BPE_TABLE, BPE_CAPTION, BPE_TR, BPE_TC, BPE_LABEL, BPE_TD, BPE_INLINE, 
		BPE_TEXT, BPE_TITLE, BPE_COUNT } bp_Element;

/*! \name the xml attributes */
typedef enum { BPA_COLSPAN, BPA_ROWSPAN, BPA_NAME, BPA_TYPE, BPA_TO, BPA_ALIGN,
		BPA_COLOR, BPA_MARGIN, BPA_MARGIN_BEFORE, BPA_MARGIN_AFTER, BPA_MARGIN_START, 
		BPA_MARGIN_END, BPA_BORDER, BPA_BORDER_BEFORE, BPA_BORDER_AFTER, 
		BPA_BORDER_START, BPA_BORDER_END, BPA_BORDER_COLOR, BPA_BORDER_BEFORE_COLOR,
		BPA_BORDER_AFTER_COLOR, BPA_BORDER_START_COLOR, BPA_BORDER_END_COLOR,
		BPA_PADDING, BPA_PADDING_BEFORE, BPA_PADDING_AFTER, BPA_PADDING_START,
		BPA_PADDING_END, BPA_FONT_SIZE, BPA_ALIGNMENT, BPA_WORD_SPACING,
		BPA_MIN_WORD_SPACING, BPA_MAX_WORD_SPACING, BPA_LETTER_SPACING,
		BPA_MIN_LETTER_SPACING, BPA_MAX_LETTER_SPACING, BPA_WIDTH, BPA_HEIGHT,
		BPA_BACKGROUND, BPA_LAYOUT, BPA_BLOCK_PROGRESSION, BPA_INLINE_PROGRESSION,
		BPA_FONT_FACE, BPA_ROTATE, BPA_MIRROR, BPA_TEX_FILE, BPA_TEX_LEFT, 
		BPA_TEX_RIGHT, BPA_TEX_TOP, BPA_TEX_BOTTOM, BPA_LINE_SPACING, BPA_COUNT } 
		bp_Attribute;

/*! \name display layout */
typedef enum { BPL_BOOK, BPL_SCROLL, BPL_COUNT } bp_Layout;

/*! \name text alignments */
typedef enum { BPAL_START, BPAL_END, BPAL_CENTER, BPAL_JUSTIFIED, BPAL_SPLIT,
		BPAL_COUNT } bp_Alignment;

/*! \name text progression directions */
typedef enum { BPD_DOWN, BPD_UP, BPD_RIGHT, BPD_LEFT, BPD_COUNT } bp_Direction;

/*! \name navigation reference types */
typedef enum { BPR_PREV, BPR_NEXT, BPR_UP, BPR_TOC, BPR_COUNT } 
		bp_ReferenceType;

/*! \name image rotation */
typedef enum { BPRO_NONE, BPRO_90CCW, BPRO_180, BPRO_90CW, BPRO_COUNT } 
		bp_Rotation;

/*! \name imag mirroring */
typedef enum { BPM_NONE, BPM_VERT, BPM_HORIZ, BPM_COUNT } bp_Mirroring;
		
/*! \name color data */
typedef struct {
	float r,g,b;
} bp_Color;

/*! \name parsed book document */
typedef struct {
	struct _bp_Node * root;
} bp_Document;

/*! \name basic element node of parser */
typedef struct _bp_Node {
	struct _bp_Node * next, * children, * children_tail;
	bp_Element element;
} bp_Node;

/*! \name "block-outside-attributes" attribute group (see schema) */
typedef struct _bp_BlockOutsideAttributes {
	float marginBefore, marginAfter, marginStart, marginEnd;
	struct _bp_BlockOutsideAttributes * next;
} bp_BlockOutsideAttributes;

/*! \name "block-border-attributes" attribute group (see schema) */
typedef struct _bp_BlockBorderAttributes {
	char borderBefore, borderAfter, borderStart, borderEnd;
	bp_Color borderBeforeColor, borderAfterColor, borderStartColor, 
					borderEndColor;
	struct _bp_BlockBorderAttributes * next;
} bp_BlockBorderAttributes;

/*! \name "block-inside-attributes" attribute group (see schema) */
typedef struct _bp_BlockInsideAttributes {
	float paddingBefore, paddingAfter, paddingStart, paddingEnd;
	float fontSize;
	bp_Alignment alignment;
	float minWordSpacing, maxWordSpacing, minLetterSpacing, maxLetterSpacing;
	float lineSpacing;
	struct _bp_BlockInsideAttributes * next;
} bp_BlockInsideAttributes;

/*! \name "inline-attributes" attribute group (see schema) */
typedef struct _bp_InlineAttributes {
	bp_Color color;
	struct _bp_InlineAttributes * next;
} bp_InlineAttributes;

/*! \name "float-attributes" attribute group (see schema) */
typedef struct {
	bp_Alignment align; /*!< note: only start, end, center used */
} bp_FloatAttributes;

/*! \name "book" element node */
typedef struct _bp_Book {
	bp_Node node;
	int width, height, background, fontFace;
	bp_Layout layout;
	bp_Direction blockProgression, inlineProgression;
	struct _bp_Page ** pages; /*< pages index for O(1) access */
	int nPages;
} bp_Book;

/*! \name "page" element node */
typedef struct _bp_Page {
	bp_Node node;
	const char * title;
	struct _bp_Ref * nav[BPR_COUNT];
} bp_Page;

/*! \name "block" element node */
typedef struct _bp_Block {
	bp_Node node;
	bp_BlockOutsideAttributes blockOutside;
	bp_BlockBorderAttributes blockBorder;
	bp_BlockInsideAttributes blockInside;
} bp_Block;

/*! \name "image" element node */
typedef struct _bp_Image {
	bp_Node node;
	bp_FloatAttributes _float;
	int width, height;
	bp_Rotation rotate;
	bp_Mirroring mirror;
	float left, right, top, bottom;
	int texture;
} bp_Image;

/*! \name "table" element node */
typedef struct _bp_Table {
	bp_Node node;
	bp_FloatAttributes _float;
	bp_BlockOutsideAttributes blockOutside;
	bp_BlockBorderAttributes blockBorder;
	struct _bp_Caption * caption;
	int nCols, nRows;
} bp_Table;

/*! \name "caption" element node */
typedef struct _bp_Caption {
	bp_Node node;
	bp_InlineAttributes inlineAttr;
	char * text;
} bp_Caption;

/*! \name "td" element node */
typedef struct _bp_Cell {
	bp_Node node;
	bp_BlockBorderAttributes blockBorder;
	bp_BlockInsideAttributes blockInside;
	int startRow, startCol, endRow, endCol;
} bp_Cell;

/*! \name "ref" element node */
typedef struct _bp_Ref {
	bp_Node node;
	struct _bp_Label * label;
} bp_Ref;

/*! \name "label" element node */
typedef struct _bp_Label {
	bp_Node node;
	bp_Page * page;
} bp_Label;

/*! \name "inline" element node */
typedef struct _bp_Inline {
	bp_Node node;
} bp_Inline;

/*! \name "inline" element node */
typedef struct _bp_Text {
	bp_Node node;
	bp_InlineAttributes inlineAttr;
	char * content;
} bp_Text;

#endif /* ENCYCLOPEDIA */

#endif // ndef __BOOKS_TYPES_H__
