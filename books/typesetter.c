#include <stdlib.h>
#include <SDL_types.h>

#include "../errors.h"
#include "types.h"
#include "typesetter.h"

ts_Page * ts_startPage(ts_Context * context);
void ts_closePage(ts_Context * context);

ts_Topic * ts_setTopic(ts_Context * context, bp_Book * book, bp_Page * topic) {
	ts_Topic * result;
	ts_Page * pagelist;
	bp_Node * child;
	int i;

	result = malloc(sizeof(ts_Topic));

	// init context
	context->topic = result;
	context->winWidth = book->width;
	context->winHeight = book->height;
	context->layout = book->layout;
	context->inlineProg = book->inlineProgression;
	context->blockProg = book->blockProgression;
	context->nPages = 0;
	context->page = NULL;

	// open the first page
	pagelist = ts_startPage(context);

	// typeset all blocks
	for (child = topic->node.children; child; child = child->next) {
		switch (child->element) {
			case BPE_BLOCK:
				break;
			case BPE_IMAGE:
				break;
			case BPE_LABEL:
				break;
			case BPE_TABLE:
				break;
			default:
				;
		}
	}

	// close last page
	ts_closePage(context);
	
	// create page index
	result->nPages = context->nPages;
	result->pages = calloc(result->nPages, sizeof(ts_Page *));
	for (i = 0; pagelist; pagelist = pagelist->next) {
		result->pages[i++] = pagelist;
	}

	return result;
}

ts_Page * ts_startPage(ts_Context * context) {
	ts_Page * newPage;

	context->nPages++;
	newPage = malloc(sizeof(ts_Page));
	newPage->topic = context->topic;
	newPage->next = NULL;
	context->page->next = newPage;
	context->page = newPage;

	switch (context->layout) {
		case BPL_SCROLL:
			context->pageLeft   = 0.0625f*context->winWidth;
			context->pageTop    = 0.0625f*context->winHeight;
			context->pageWidth  = 0.8750f*context->winWidth;
			context->pageHeight = 0.8750f*context->winHeight;
			break;
		case BPL_BOOK:
			// distinguish left/right pages
			if (context->nPages & 1) {
				// put page one on the right
				context->pageLeft   = 0.5625f*context->winWidth;
				context->pageTop    = 0.0625f*context->winHeight;
				context->pageWidth  = 0.3750f*context->winWidth;
				context->pageHeight = 0.8125f*context->winHeight;
			} else {
				context->pageLeft   = 0.0625f*context->winWidth;
				context->pageTop    = 0.0625f*context->winHeight;
				context->pageWidth  = 0.3750f*context->winWidth;
				context->pageHeight = 0.8125f*context->winHeight;
			}
			break;
		default:
			LOG_ERROR("Typesetter encountered illegal book layout!\n");
			exit(1);
	}
	
	return newPage;
}

void ts_closePage(ts_Context * context) {
	return;
}

