/*!
 * \file
 */

#ifndef __BOOKS_TYPESETTER_H__
#define __BOOKS_TYPESETTER_H__

#ifdef ENCYCLOPEDIA

#include "types.h"
#include "fontdef.h"

/*! \name */
typedef struct _ts_Topic {
	struct _ts_Page ** pages; // for O(1) access
	int nPages;
} ts_Topic;

/*! \name */
typedef struct _ts_Line {
	struct _ts_Line * next;
	float coords[4];
	float color[3];
} ts_Line;

/*! \name */
typedef struct _ts_Page {
	struct _ts_Topic * topic;
	struct _ts_Page * next;
	struct _ts_Line * lines;
} ts_Page;

/*! \name */
typedef float (*ts_binOp)(float dest, float src);

/*! \name */
typedef struct _ts_Context {
	struct _ts_Topic * topic;
	int winWidth, winHeight;
	bp_Layout layout;
	bp_Direction inlineProg, blockProg;
	fd_Font * font;
	int nPages;
	struct _ts_Page * page;
	float pageLeft, pageTop, pageRight, pageBottom, x, y;
	float *pageBefore, *pageAfter, *pageStart, *pageEnd, *blockPos, *inlinePos;
	ts_binOp blockMove, inlineMove;
	float lastMarginAfter;
	float blockStart, blockEnd, borderBefore, borderAfter, borderStart, borderEnd;
} ts_Context;

/*!
 * \brief
 */
ts_Topic * ts_setTopic(ts_Context * context, bp_Book * book, bp_Page * topic);

#endif /* ENCYCLOPEDIA */

#endif // not defined __BOOKS_TYPESETTER_H__
