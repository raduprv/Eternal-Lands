#ifndef	__WIDGETS_H
#define	__WIDGETS_H

#define LABEL		0x01
#define IMAGE		0x02
#define CHECKBOX	0x03
#define BUTTON		0x04
#define PROGRESSBAR	0x05
#define VSCROLLBAR	0x06

typedef struct wl{
	// Common widget data
	Uint16 pos_x, pos_y, len_x, len_y; // Widget area
	Uint32 id; // Widget unique id
	Uint32 type; // Specifies what kind of widget it is
	Uint32 Flags; // Status flags...visible,enbled,etc
	float size; // Size of text, image, etc
	float r, g, b; // Associated color

	// the handlers
	int (*OnDraw)();
	int (*OnClick)();
	int (*OnDrag)();
	int (*OnInit)();
	int (*OnMouseover)();

	void *widget_info; // Pointer to specific widget data
	struct wl *next; // Pointer to the next widget in the window
}widget_list;

typedef struct {
	char text[256]; // Text
}label;

typedef struct {
	float u1,v1,u2,v2; // Texture coordinates
	int id; // Texture id
}image;

typedef struct {
	int checked;
}checkbox;

typedef struct {
	char text[256];
}button;

typedef struct {
	float progress;
}progressbar;

typedef struct {
	int pos, pos_inc;
}vscrollbar;

// Common widget functions
widget_list * widget_find(Uint32 window_id, Uint32 widget_id);
int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);
int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);
int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f);
int widget_set_size(Uint32 window_id, Uint32 widget_id, float size);
int widget_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b);

// Label
int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);
int label_draw(widget_list *W);
int label_set_text(Uint32 window_id, Uint32 widget_id, char *text);

// Image
int image_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float id, float u1, float v1, float u2, float v2);
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2);
int image_draw(widget_list *W);
int image_set_id(Uint32 window_id, Uint32 widget_id, int id);
int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2);

// Checkbox
int checkbox_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int checked);
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int checked);
int checkbox_draw(widget_list *W);
int checkbox_click(widget_list *W);
int checkbox_get_checked(Uint32 window_id, Uint32 widget_id);
int checkbox_set_checked(Uint32 window_id, Uint32 widget_id, int checked);

// Button
int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);
int button_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);
int button_draw(widget_list *W);
int button_set_text(Uint32 window_id, Uint32 widget_id, char *text);

// Progressbar
int progressbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress);
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);
int progressbar_draw(widget_list *W);
float progressbar_get_progress(Uint32 window_id, Uint32 widget_id);
int progressbar_set_progress(Uint32 window_id, Uint32 widget_id, float progress);

// Vertical Scrollbar
int vscrollbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc);
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);
int vscrollbar_draw(widget_list *W);
int vscrollbar_click(widget_list *W, int x, int y);
int vscrollbar_drag(widget_list *W, int dx, int dy);
int vscrollbar_set_pos_inc(Uint32 window_id, Uint32 widget_id, int pos_inc);


// XML Windows
int AddXMLWindow(char *fn);
int ReadXMLWindow(xmlNode * a_node);
int ParseWindow(xmlAttr *a_node);
int ParseWidget(char *wn, int winid, xmlAttr *a_node);
int GetWidgetType(char *w);
#endif
