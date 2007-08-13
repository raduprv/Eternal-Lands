#ifdef ENCYCLOPEDIA

#include <stdlib.h>
#include <SDL_types.h>

#include "../global.h" // because of misc.h
#include "../misc.h"
#include "../errors.h"
#include "types.h"
#include "fontdef.h"
#include "typesetter.h"

float ts_add(float dest, float src);
float ts_sub(float dest, float src);

ts_Page * ts_startPage(ts_Context * context); 
void ts_closePage(ts_Context * context); 
void ts_startLine(ts_Context * context); 
void ts_closeLine(ts_Context * context);
void ts_addBorders(ts_Context * context, const bp_BlockBorderAttributes * attributes);
void ts_addLine(ts_Context * context, float before, float after, float start, float end, const bp_Color * color);
void ts_addLineToPage(ts_Page * page, float x1, float y1, float x2, float y2, const bp_Color * color);

void ts_setBlock(ts_Context * context, bp_Block * block);

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
	context->font = fd_getFont(book->fontFace);
	context->nPages = 0;
	context->page = NULL;

	if (!context->font) {
		LOG_ERROR("WTH? Bad font id!\n");
		exit(1);
	}

	switch (context->blockProg) {
		case BPD_UP:
			context->pageBefore = &context->pageBottom;
			context->pageAfter  = &context->pageTop;
			context->blockPos   = &context->y;
			context->blockMove  = &ts_sub;
			break;
		case BPD_RIGHT:
			context->pageBefore = &context->pageLeft;
			context->pageAfter  = &context->pageRight;
			context->blockPos   = &context->x;
			context->blockMove  = &ts_add;
			break;
		case BPD_DOWN:
			context->pageBefore = &context->pageTop;
			context->pageAfter  = &context->pageBottom;
			context->blockPos   = &context->y;
			context->blockMove  = &ts_add;
			break;
		case BPD_LEFT:
			context->pageBefore = &context->pageRight;
			context->pageAfter  = &context->pageLeft;
			context->blockPos   = &context->x;
			context->blockMove  = &ts_sub;
			break;
		default:
			LOG_ERROR("Typesetter encountered illegal block progression!\n");
			exit(1);
	}

	switch (context->inlineProg) {
		case BPD_UP:
			context->pageStart  = &context->pageBottom;
			context->pageEnd    = &context->pageTop;
			context->inlinePos  = &context->y;
			context->inlineMove = &ts_sub;
			break;
		case BPD_RIGHT:
			context->pageStart  = &context->pageLeft;
			context->pageEnd    = &context->pageRight;
			context->inlinePos  = &context->x;
			context->inlineMove = &ts_add;
			break;
		case BPD_DOWN:
			context->pageStart  = &context->pageTop;
			context->pageEnd    = &context->pageBottom;
			context->inlinePos  = &context->y;
			context->inlineMove = &ts_add;
			break;
		case BPD_LEFT:
			context->pageStart  = &context->pageRight;
			context->pageEnd    = &context->pageLeft;
			context->inlinePos  = &context->x;
			context->inlineMove = &ts_sub;
			break;
		default:
			LOG_ERROR("Typesetter encountered illegal inline progression!\n");
			exit(1);
	}

	// open the first page
	pagelist = ts_startPage(context);

	// typeset all blocks
	for (child = topic->node.children; child; child = child->next) {
		switch (child->element) {
			case BPE_BLOCK:
				ts_setBlock(context, (bp_Block *) child);
				break;
			case BPE_IMAGE:
				// TODO
				break;
			case BPE_LABEL:
				// TODO
				break;
			case BPE_TABLE:
				// TODO
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

float ts_add(float dest, float src) {
	return dest + src;
}

float ts_sub(float dest, float src) {
	return dest - src;
}

ts_Page * ts_startPage(ts_Context * context) {
	ts_Page * newPage;

	context->nPages++;
	newPage = malloc(sizeof(ts_Page));
	newPage->topic = context->topic;
	newPage->next = NULL;
	newPage->lines = NULL;
	context->page->next = newPage;
	context->page = newPage;

	switch (context->layout) {
		case BPL_SCROLL:
			context->pageLeft   = 0.0625f*context->winWidth;
			context->pageTop    = 0.0625f*context->winHeight;
			context->pageRight  = 0.9375f*context->winWidth;
			context->pageBottom = 0.9375f*context->winHeight;
			break;
		case BPL_BOOK:
			// distinguish left/right pages
			if (context->nPages & 1) {
				// put page one on the right
				context->pageLeft   = 0.5625f*context->winWidth;
				context->pageTop    = 0.0625f*context->winHeight;
				context->pageRight  = 0.9375f*context->winWidth;
				context->pageBottom = 0.8750f*context->winHeight;
			} else {
				context->pageLeft   = 0.0625f*context->winWidth;
				context->pageTop    = 0.0625f*context->winHeight;
				context->pageRight  = 0.4375f*context->winWidth;
				context->pageBottom = 0.8750f*context->winHeight;
			}
			break;
		default:
			LOG_ERROR("Typesetter encountered illegal book layout!\n");
			exit(1);
	}

	*context->blockPos  = *context->pageBefore;
	*context->inlinePos = *context->pageStart;
	context->lastMarginAfter = 0.0f;

	return newPage;
}

void ts_closePage(ts_Context * context) {
	return;
}

void ts_startLine(ts_Context * context) {
	*context->inlinePos = context->blockStart;
}

void ts_closeLine(ts_Context * context) {
	*context->blockPos = context->blockMove(*context->blockPos, context->font->linesize);
}

void ts_addBorders(ts_Context * context, const bp_BlockBorderAttributes * attributes) {
	if (attributes->borderBefore) ts_addLine(context, context->borderBefore, context->borderBefore, 
			context->borderStart, context->borderEnd, &attributes->borderBeforeColor);
	if (attributes->borderAfter) ts_addLine(context, context->borderAfter, context->borderAfter, 
			context->borderStart, context->borderEnd, &attributes->borderAfterColor);
	if (attributes->borderStart) ts_addLine(context, context->borderBefore, context->borderAfter, 
			context->borderStart, context->borderStart, &attributes->borderStartColor);
	if (attributes->borderEnd) ts_addLine(context, context->borderBefore, context->borderAfter, 
			context->borderEnd, context->borderEnd, &attributes->borderEndColor);
}

void ts_addLine(ts_Context * context, float before, float after, float start, float end, const bp_Color * color) {
	float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
	switch (context->blockProg) {
		case BPD_UP:
		case BPD_DOWN:
			y1 = before;
			y2 = after;
			break;
		case BPD_LEFT:
		case BPD_RIGHT:
			x1 = before;
			x2 = after;
		default:;
	}
	switch (context->inlineProg) {
		case BPD_UP:
		case BPD_DOWN:
			y1 = start;
			y2 = end;
			break;
		case BPD_LEFT:
		case BPD_RIGHT:
			x1 = start;
			x2 = end;
		default:;
	}

	ts_addLineToPage(context->page, x1, y1, x2, y2, color);
}

void ts_addLineToPage(ts_Page * page, float x1, float y1, float x2, float y2, const bp_Color * color) {
	ts_Line * line = malloc(sizeof(ts_Line));

	line->next = page->lines;
	line->coords[0] = x1;
	line->coords[1] = y1;
	line->coords[2] = x2;
	line->coords[3] = y2;
	line->color[0] = color->r;
	line->color[1] = color->g;
	line->color[2] = color->b;

	page->lines = line;
}

void ts_setBlock(ts_Context * context, bp_Block * block) {
	// add margin & padding
	context->borderBefore = context->blockMove(*context->blockPos, max2f(context->lastMarginAfter, block->blockOutside.marginBefore));
	*context->blockPos = context->blockMove(context->borderBefore, block->blockInside.paddingBefore);

	// set block boundaries
	context->borderStart = context->inlineMove(*context->pageStart, block->blockOutside.marginStart);
	context->blockStart = context->inlineMove(context->borderStart, block->blockInside.paddingStart);
	
	context->borderEnd = context->inlineMove(*context->pageEnd, - block->blockOutside.marginEnd);
	context->blockEnd = context->inlineMove(context->borderEnd, - block->blockInside.paddingEnd);

	// start the first line
	ts_startLine(context);

	// typeset children
	// TODO
	
	// close the last line
	ts_closeLine(context);

	// add margin & padding
	context->borderAfter = context->blockMove(*context->blockPos, block->blockInside.paddingAfter);
	*context->blockPos = context->borderAfter;
	context->lastMarginAfter = block->blockOutside.marginAfter;

	// add borders
	ts_addBorders(context, &block->blockBorder);
}

#endif /* ENCYCLOPEDIA */
