#ifndef	__WIDGETS_H
#define	__WIDGETS_H

#define LABEL 0x01
#define IMAGE 0x02

typedef struct wl{
	// Common widget data
	Uint16 pos_x, pos_y, len_x, len_y; // Widget area
	Uint32 id; // Widget unique id
	Uint32 type; // Specifies what kind of widget it is
	Uint32 Flags; // Status flags...visible,enbled,etc
	
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
	float size; // Text size
	float r,g,b; // Text color
	char text[256]; // Text
}label;

typedef struct {
	float size; // Image size
	float u1,v1,u2,v2; // Texture coordinates
	int id; // Texture id
}image;

// Common widget functions
int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);
int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);
int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f);

// Label
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);
int label_draw(widget_list *W);
int label_set_size(Uint32 window_id, Uint32 widget_id, float size);
int label_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b);
int label_set_text(Uint32 window_id, Uint32 widget_id, char *text);

// Image
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 xe, Uint16 ye, float u1, float v1, float u2, float v2);
int image_draw(widget_list *W);
int image_set_size(Uint32 window_id, Uint32 widget_id, float size);
int image_set_id(Uint32 window_id, Uint32 widget_id, int id);
int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2);

#endif
