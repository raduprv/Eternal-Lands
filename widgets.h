/*!
 * \file
 * \brief Functions for the widgets used by EL
 * \ingroup misc
 * \internal check groups!
 */
#ifndef	__WIDGETS_H
#define	__WIDGETS_H

/*!
 * \name    LABEL, IMAGE, CHECKBOX, BUTTON, PROGRESSBAR, VSCROLLBAR
 */
/*! \{ */
#define LABEL		0x01     /*!< LABEL */
#define IMAGE		0x02     /*!< IMAGE */
#define CHECKBOX	0x03     /*!< CHECKBOX */
#define BUTTON		0x04     /*!< BUTTON */
#define PROGRESSBAR	0x05     /*!< PROGRESSBAR */
#define VSCROLLBAR	0x06     /*!< VSCROLLBAR */
/*! \} */


/*!
 *  TODO: struct widget_list
 */
typedef struct wl{
	// Common widget data
	Uint16 pos_x, pos_y, len_x, len_y; /*!< Widget area */
	Uint32 id;                         /*!< Widget unique id */
	Uint32 type;   /*!< Specifies what kind of widget it is */
	Uint32 Flags;  /*!< Status flags...visible,enbled,etc */
	float size;    /*!< Size of text, image, etc */
	float r, g, b; /*!< Associated color */

	// the handlers
	int (*OnDraw)();
	int (*OnClick)();
	int (*OnDrag)();
	int (*OnInit)();
	int (*OnMouseover)();

	void *widget_info; /*!< Pointer to specific widget data */
	struct wl *next;   /*!< Pointer to the next widget in the window */
}widget_list;


/*!
 *  TODO: struct label
 */
typedef struct {
	char text[256]; /*!< Text */
}label;


/*!
 *  TODO: struct image
 */
typedef struct {
	float u1,v1,u2,v2; /*!< Texture coordinates */
	int id;            /*!< Texture id */
}image;

/*!
 *  TODO: struct checkbox
 */
typedef struct {
	int checked;
}checkbox;

/*!
 *  TODO: struct button
 */
typedef struct {
	char text[256];
}button;

/*!
 *  TODO: struct progressbar
 */
typedef struct {
	float progress;
}progressbar;

/*!
 *  TODO: struct vcscrollbar
 */
typedef struct {
	int pos, pos_inc;
}vscrollbar;


// Common widget functions

/*!
 * \brief widget_find
 *
 *      TODO: widget_find
 *
 * \param   window_id
 * \param   widget_id
 * \return  widget_list*
 */
widget_list * widget_find(Uint32 window_id, Uint32 widget_id);

/*!
 * \brief widget_set_OnDraw
 *
 *      TODO: widget_set_OnDraw
 *
 * \param   window_id
 * \param   widget_id
 * \param   handler
 * \return  int
 */
int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)());

/*!
 * \brief widget_set_OnClick
 *
 *      TODO: widget_set_OnClick
 *
 * \param   window_id
 * \param   widget_id
 * \param   handler
 * \return  int
 */
int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)());

/*!
 * \brief widget_set_OnDrag
 *
 *      TODO: widget_set_OnDrag
 *
 * \param   window_id
 * \param   widget_id
 * \param   handler
 * \return  int
 */
int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)());

/*!
 * \brief widget_set_OnMouseover
 *
 *      TODO: widget_set_OnMouseover
 *
 * \param   window_id
 * \param   widget_id
 * \param   handler
 * \return  int
 */
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());

/*!
 * \brief widget_move
 *
 *      TODO: widget_move
 *
 * \param   window_id
 * \param   widget_id
 * \param   x
 * \param   y
 * \return  int
 */
int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);

/*!
 * \brief widget_resize
 *
 *      TODO: widget_resize
 *
 * \param   window_id
 * \param   widget_id
 * \param   x
 * \param   y
 * \return  int
 */
int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);

/*!
 * \brief widget_set_flags
 *
 *      TODO: widget_set_flags
 *
 * \param   window_id
 * \param   widget_id
 * \param   f
 * \return  int
 */
int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f);

/*!
 * \brief widget_set_size
 *
 *      TODO: widget_set_size
 *
 * \param   window_id
 * \param   widget_id
 * \param   size
 * \return  int
 */
int widget_set_size(Uint32 window_id, Uint32 widget_id, float size);

/*!
 * \brief widget_set_color
 *
 *      TODO: widget_set_color
 *
 * \param   window_id
 * \param   widget_id
 * \param   r
 * \param   g
 * \param   b
 * \return  int
 */
int widget_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b);



// Label

/*!
 * \brief label_add_extended
 *
 *      TODO: label_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   text
 * \return  int
 */
int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);

/*!
 * \brief label_add
 *
 *      TODO: label_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   text
 * \param   x
 * \param   y
 * \return  int
 */
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

/*!
 * \brief label_draw
 *
 *      TODO: label_draw
 *
 * \param   W
 * \return  int
 */
int label_draw(widget_list *W);

/*!
 * \brief label_set_text
 *
 *      TODO: label_set_text
 *
 * \param   window_id
 * \param   widget_id
 * \param   text
 * \return  int
 */
int label_set_text(Uint32 window_id, Uint32 widget_id, char *text);



// Image

/*!
 * \brief image_add_extended
 *
 *      TODO: image_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   id
 * \param   u1
 * \param   v1
 * \param   u2
 * \param   v2
 * \return  int
 */
int image_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float id, float u1, float v1, float u2, float v2);

/*!
 * \brief image_add
 *
 *      TODO: image_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   id
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   u1
 * \param   v1
 * \param   u2
 * \param   v2
 * \return  int
 */
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2);

/*!
 * \brief image_draw
 *
 *      TODO: image_draw
 *
 * \param   W
 * \return  int
 */
int image_draw(widget_list *W);

/*!
 * \brief image_set_id
 *
 *      TODO: image_set_id
 *
 * \param   window_id
 * \param   widget_id
 * \param   id
 * \return  int
 */
int image_set_id(Uint32 window_id, Uint32 widget_id, int id);

/*!
 * \brief image_set_uv
 *
 *      TODO: image_set_uv
 *
 * \param   window_id
 * \param   widget_id
 * \param   u1
 * \param   v1
 * \param   u2
 * \param   v2
 * \return  int
 */
int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2);



// Checkbox

/*!
 * \brief checkbox_add_extended
 *
 *      TODO: checkbox_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   checked
 * \return  int
 */
int checkbox_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int checked);

/*!
 * \brief checkbox_add
 *
 *      TODO: checkbox_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   checked
 * \return  int
 */
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int checked);

/*!
 * \brief checkbox_draw
 *
 *      TODO: checkbox_draw
 *
 * \param   W
 * \return  int
 */
int checkbox_draw(widget_list *W);

/*!
 * \brief checkbox_click
 *
 *      TODO: checkbox_click
 *
 * \param   W
 * \return  int
 */
int checkbox_click(widget_list *W);

/*!
 * \brief checkbox_get_checked
 *
 *      TODO: checkbox_get_checked
 *
 * \param   window_id
 * \param   widget_id
 * \return  int
 */
int checkbox_get_checked(Uint32 window_id, Uint32 widget_id);

/*!
 * \brief checkbox_set_checked
 *
 *      TODO: checkbox_set_checked
 *
 * \param   window_id
 * \param   widget_id
 * \param   checked
 * \return  int
 */
int checkbox_set_checked(Uint32 window_id, Uint32 widget_id, int checked);



// Button

/*!
 * \brief button_add_extended
 *
 *      TODO: button_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   text
 * \return  int
 */
int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);

/*!
 * \brief button_add
 *
 *      TODO: button_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   text
 * \param   x
 * \param   y
 * \return  int
 */
int button_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

/*!
 * \brief button_draw
 *
 *      TODO: button_draw
 *
 * \param   W
 * \return  int
 */
int button_draw(widget_list *W);

/*!
 * \brief button_set_text
 *
 *      TODO: button_set_text
 *
 * \param   window_id
 * \param   widget_id
 * \param   text
 * \return  int
 */
int button_set_text(Uint32 window_id, Uint32 widget_id, char *text);



// Progressbar

/*!
 * \brief progressbar_add_extended
 *
 *      TODO: progressbar_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   progress
 * \return  int
 */
int progressbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress);

/*!
 * \brief progressbar_add
 *
 *      TODO: progressbar_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \return  int
 */
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \brief progressbar_draw
 *
 *      TODO: progressbar_draw
 *
 * \param   W
 * \return  int
 */
int progressbar_draw(widget_list *W);

/*!
 * \brief progressbar_get_progress
 *
 *      TODO: progressbar_get_progress
 *
 * \param   window_id
 * \param   widget_id
 * \return  float
 */
float progressbar_get_progress(Uint32 window_id, Uint32 widget_id);

/*!
 * \brief progressbar_set_progress
 *
 *      TODO: progressbar_set_progress
 *
 * \param   window_id
 * \param   widget_id
 * \param   progress
 * \return  int
 */
int progressbar_set_progress(Uint32 window_id, Uint32 widget_id, float progress);



// Vertical Scrollbar

/*!
 * \brief vscrollbar_add_extended
 *
 *      TODO: vscrollbar_add_extended
 *
 * \param   window_id
 * \param   wid
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \param   Flags
 * \param   size
 * \param   r
 * \param   g
 * \param   b
 * \param   pos
 * \param   pos_inc
 * \return  int
 */
int vscrollbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc);

/*!
 * \brief vscrollbar_add
 *
 *      TODO: vscrollbar_add
 *
 * \param   window_id
 * \param   OnInit
 * \param   x
 * \param   y
 * \param   lx
 * \param   ly
 * \return  int
 */
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \brief vscrollbar_draw
 *
 *      TODO: vscrollbar_draw
 *
 * \param   W
 * \return  int
 */
int vscrollbar_draw(widget_list *W);

/*!
 * \brief vscrollbar_click
 *
 *      TODO: vscrollbar_click
 *
 * \param   W
 * \param   x
 * \param   y
 * \return  int
 */
int vscrollbar_click(widget_list *W, int x, int y);

/*!
 * \brief vscrollbar_drag
 *
 *      TODO: vscrollbar_drag
 *
 * \param   W
 * \param   dx
 * \param   dy
 * \return  int
 */
int vscrollbar_drag(widget_list *W, int dx, int dy);

/*!
 * \brief vscrollbar_set_pos_inc
 *
 *      TODO: vscrollbar_set_pos_inc
 *
 * \param   window_id
 * \param   widget_id
 * \param   pos_inc
 * \return  int
 */
int vscrollbar_set_pos_inc(Uint32 window_id, Uint32 widget_id, int pos_inc);



// XML Windows

/*!
 * \brief AddXMLWindow
 *
 *      TODO: AddXMLWindow
 *
 * \param   fn
 * \return  int
 */
int AddXMLWindow(char *fn);

/*!
 * \brief ReadXMLWindow
 *
 *      TODO: ReadXMLWindow
 *
 * \param   a_node
 * \return  int
 */
int ReadXMLWindow(xmlNode * a_node);

/*!
 * \brief ParseWindow
 *
 *      TODO: ParseWindow
 *
 * \param   a_node
 * \return  int
 */
int ParseWindow(xmlAttr *a_node);

/*!
 * \brief ParseWidget
 *
 *      TODO: ParseWidget
 *
 * \param   wn
 * \param   winid
 * \param   a_node
 * \return  int
 */
int ParseWidget(char *wn, int winid, xmlAttr *a_node);

/*!
 * \brief GetWidgetType
 *
 *      TODO: GetWidgetType
 *
 * \param   W
 * \return  int
 */
int GetWidgetType(char *w);
#endif

