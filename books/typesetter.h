/*!
 * \file
 */

#ifndef __BOOKS_TYPESETTER_H__
#define __BOOKS_TYPESETTER_H__

#include "types.h"

/*! \name */
typedef struct _ts_Topic {
	struct _ts_Page ** pages; // for O(1) access
	int nPages;
} ts_Topic;

typedef struct _ts_Page {
	struct _ts_Topic * topic;
	struct _ts_Page * next;
} ts_Page;

/*! \name */
typedef struct _ts_Context {
	struct _ts_Topic * topic;
	int winWidth, winHeight;
	bp_Layout layout;
	bp_Direction inlineProg, blockProg;
	int nPages;
	struct _ts_Page * page;
	int pageLeft, pageTop, pageWidth, pageHeight;
} ts_Context;

/*!
 * \brief
 */
ts_Topic * ts_setTopic(ts_Context * context, bp_Book * book, bp_Page * topic);

#endif // not defined __BOOKS_TYPESETTER_H__
