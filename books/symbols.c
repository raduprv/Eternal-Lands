#ifdef ENCYCLOPEDIA

#include <stdlib.h>
#include "../symbol_table.h"
#include "../textures.h"
#include "symbols.h"
#include "types.h"

symbol_table * bp_elements;
symbol_table * bp_attributes;
symbol_table * bp_layouts;
symbol_table * bp_fonts = NULL;
symbol_table * bp_alignments;
symbol_table * bp_directions;
symbol_table * bp_referenceTypes;
symbol_table * bp_colors;
symbol_table * bp_rotations;
symbol_table * bp_mirrorings;

const bp_Color bp_black   = { 0.0, 0.0, 0.0 };
const bp_Color bp_navy    = { 0.0, 0.0, 0.5 };
const bp_Color bp_green   = { 0.0, 0.5, 0.0 };
const bp_Color bp_teal    = { 0.0, 0.5, 0.5 };
const bp_Color bp_maroon  = { 0.5, 0.0, 0.0 };
const bp_Color bp_purple  = { 0.5, 0.0, 0.5 };
const bp_Color bp_olive   = { 0.5, 0.5, 0.0 };
const bp_Color bp_silver  = { 0.8, 0.8, 0.8 };
const bp_Color bp_gray    = { 0.5, 0.5, 0.5 };
const bp_Color bp_blue    = { 0.0, 0.0, 1.0 };
const bp_Color bp_lime    = { 0.0, 1.0, 0.0 };
const bp_Color bp_aqua    = { 0.0, 1.0, 1.0 };
const bp_Color bp_red     = { 1.0, 0.0, 0.0 };
const bp_Color bp_fuchsia = { 1.0, 0.0, 1.0 };
const bp_Color bp_yellow  = { 1.0, 1.0, 0.0 };
const bp_Color bp_white   = { 1.0, 1.0, 1.0 };

void bp_init_symbols() {
	bp_elements = st_create(BPE_COUNT);
	st_addnum(bp_elements, "book",    BPE_BOOK);
	st_addnum(bp_elements, "page",    BPE_PAGE);
	st_addnum(bp_elements, "ref",     BPE_REF);
	st_addnum(bp_elements, "block",   BPE_BLOCK);
	st_addnum(bp_elements, "image",   BPE_IMAGE);
	st_addnum(bp_elements, "table",   BPE_TABLE);
	st_addnum(bp_elements, "caption", BPE_CAPTION);
	st_addnum(bp_elements, "tr",      BPE_TR);
	st_addnum(bp_elements, "tc",      BPE_TC);
	st_addnum(bp_elements, "label",   BPE_LABEL);
	st_addnum(bp_elements, "td",      BPE_TD);
	st_addnum(bp_elements, "inline",  BPE_INLINE);
	st_addnum(bp_elements, "text",    BPE_TEXT);
	st_addnum(bp_elements, "title",   BPE_TITLE);
	st_commit(bp_elements);

	bp_attributes = st_create(BPA_COUNT);
	st_addnum(bp_attributes, "colspan", BPA_COLSPAN);
	st_addnum(bp_attributes, "rowspan", BPA_ROWSPAN);
	st_addnum(bp_attributes, "name", BPA_NAME);
	st_addnum(bp_attributes, "type", BPA_TYPE);
	st_addnum(bp_attributes, "to", BPA_TO);
	st_addnum(bp_attributes, "align", BPA_ALIGN);
	st_addnum(bp_attributes, "color", BPA_COLOR);
	st_addnum(bp_attributes, "margin", BPA_MARGIN);
	st_addnum(bp_attributes, "margin-before", BPA_MARGIN_BEFORE);
	st_addnum(bp_attributes, "margin-after", BPA_MARGIN_AFTER);
	st_addnum(bp_attributes, "margin-start", BPA_MARGIN_START);
	st_addnum(bp_attributes, "margin-end", BPA_MARGIN_END);
	st_addnum(bp_attributes, "border", BPA_BORDER);
	st_addnum(bp_attributes, "border-before", BPA_BORDER_BEFORE);
	st_addnum(bp_attributes, "border-after", BPA_BORDER_AFTER);
	st_addnum(bp_attributes, "border-start", BPA_BORDER_START);
	st_addnum(bp_attributes, "border-end", BPA_BORDER_END);
	st_addnum(bp_attributes, "border-color", BPA_BORDER_COLOR);
	st_addnum(bp_attributes, "border-before-color", BPA_BORDER_BEFORE_COLOR);
	st_addnum(bp_attributes, "border-after-color", BPA_BORDER_AFTER_COLOR);
	st_addnum(bp_attributes, "border-start-color", BPA_BORDER_START_COLOR);
	st_addnum(bp_attributes, "border-end-color", BPA_BORDER_END_COLOR);
	st_addnum(bp_attributes, "padding", BPA_PADDING);
	st_addnum(bp_attributes, "padding-before", BPA_PADDING_BEFORE);
	st_addnum(bp_attributes, "padding-after", BPA_PADDING_AFTER);
	st_addnum(bp_attributes, "padding-start", BPA_PADDING_START);
	st_addnum(bp_attributes, "padding-end", BPA_PADDING_END);
	st_addnum(bp_attributes, "font-size", BPA_FONT_SIZE);
	st_addnum(bp_attributes, "alignment", BPA_ALIGNMENT);
	st_addnum(bp_attributes, "word-spacing", BPA_WORD_SPACING);
	st_addnum(bp_attributes, "min-word-spacing", BPA_MIN_WORD_SPACING);
	st_addnum(bp_attributes, "max-word-spacing", BPA_MAX_WORD_SPACING);
	st_addnum(bp_attributes, "letter-spacing", BPA_LETTER_SPACING);
	st_addnum(bp_attributes, "min-letter-spacing", BPA_MIN_LETTER_SPACING);
	st_addnum(bp_attributes, "max-letter-spacing", BPA_MAX_LETTER_SPACING);
	st_addnum(bp_attributes, "line-spacing", BPA_LINE_SPACING);
	st_addnum(bp_attributes, "width", BPA_WIDTH);
	st_addnum(bp_attributes, "height", BPA_HEIGHT);
	st_addnum(bp_attributes, "background", BPA_BACKGROUND);
	st_addnum(bp_attributes, "layout", BPA_LAYOUT);
	st_addnum(bp_attributes, "block-progression-direction", BPA_BLOCK_PROGRESSION);
	st_addnum(bp_attributes, "inline-progression-direction", BPA_INLINE_PROGRESSION);
	st_addnum(bp_attributes, "font-face", BPA_FONT_FACE);
	st_addnum(bp_attributes, "rotate", BPA_ROTATE);
	st_addnum(bp_attributes, "mirror", BPA_MIRROR);
	st_addnum(bp_attributes, "tex-left", BPA_TEX_LEFT);
	st_addnum(bp_attributes, "tex-right", BPA_TEX_RIGHT);
	st_addnum(bp_attributes, "tex-top", BPA_TEX_TOP);
	st_addnum(bp_attributes, "tex-bottom", BPA_TEX_BOTTOM);
	st_addnum(bp_attributes, "tex-file", BPA_TEX_FILE);
	st_commit(bp_attributes);

	bp_layouts = st_create(BPL_COUNT);
	st_addnum(bp_layouts, "book", BPL_BOOK);
	st_addnum(bp_layouts, "scroll", BPL_SCROLL);
	st_commit(bp_layouts);

	bp_alignments = st_create(BPAL_COUNT);
	st_addnum(bp_alignments, "start", BPAL_START);
	st_addnum(bp_alignments, "end", BPAL_END);
	st_addnum(bp_alignments, "center", BPAL_CENTER);
	st_addnum(bp_alignments, "justified", BPAL_JUSTIFIED);
	st_addnum(bp_alignments, "split", BPAL_SPLIT);
	st_commit(bp_alignments);

	bp_directions = st_create(BPD_COUNT);
	st_addnum(bp_directions, "top-down", BPD_DOWN);
	st_addnum(bp_directions, "bottom-up", BPD_UP);
	st_addnum(bp_directions, "left-to-right", BPD_RIGHT);
	st_addnum(bp_directions, "right-to-left", BPD_LEFT);
	st_commit(bp_directions);

	bp_referenceTypes = st_create(BPR_COUNT);
	st_addnum(bp_referenceTypes, "previous", BPR_PREV);
	st_addnum(bp_referenceTypes, "next", BPR_NEXT);
	st_addnum(bp_referenceTypes, "up", BPR_UP);
	st_addnum(bp_referenceTypes, "contents", BPR_TOC);
	st_commit(bp_referenceTypes);

	bp_colors = st_create(16);
	st_addptr(bp_colors, "black",   (void *) &bp_black);
	st_addptr(bp_colors, "navy",    (void *) &bp_navy);
	st_addptr(bp_colors, "green",   (void *) &bp_green);
	st_addptr(bp_colors, "teal",    (void *) &bp_teal);
	st_addptr(bp_colors, "maroon",  (void *) &bp_maroon);
	st_addptr(bp_colors, "purple",  (void *) &bp_purple);
	st_addptr(bp_colors, "olive",   (void *) &bp_olive);
	st_addptr(bp_colors, "silver",  (void *) &bp_silver);
	st_addptr(bp_colors, "gray",    (void *) &bp_gray);
	st_addptr(bp_colors, "blue",    (void *) &bp_blue);
	st_addptr(bp_colors, "lime",    (void *) &bp_lime);
	st_addptr(bp_colors, "aqua",    (void *) &bp_aqua);
	st_addptr(bp_colors, "red",     (void *) &bp_red);
	st_addptr(bp_colors, "fuchsia", (void *) &bp_fuchsia);
	st_addptr(bp_colors, "yellow",  (void *) &bp_yellow);
	st_addptr(bp_colors, "white",   (void *) &bp_white);
	st_commit(bp_colors);

	bp_rotations = st_create(BPRO_COUNT);
	st_addnum(bp_rotations, "none", BPRO_NONE);
	st_addnum(bp_rotations, "90cw", BPRO_90CW);
	st_addnum(bp_rotations, "90ccw", BPRO_90CCW);
	st_addnum(bp_rotations, "180", BPRO_180);
	st_commit(bp_rotations);

	bp_mirrorings = st_create(BPM_COUNT);
	st_addnum(bp_mirrorings, "none", BPM_NONE);
	st_addnum(bp_mirrorings, "horizontal", BPM_HORIZ);
	st_addnum(bp_mirrorings, "vertical", BPM_VERT);
	st_commit(bp_mirrorings);
}

#endif /* ENCYCLOPEDIA */
