#ifndef	__WIDGETS_H
#define	__WIDGETS_H

#define LABEL 0x01

typedef struct wl{
	// Common widget data
	Uint16 pos_x, pos_y, len_x, len_y; // Widget area
	Uint32 id; // Widget unique id
	Uint32 type; // Specifies what kind of widget it is
	Uint32 Flags; // Status flags...visible,enbled,etc
	
	// the handlers
	int (*OnDraw)(struct wl*);
	int (*OnClick)();
	int (*OnDrag)();
	int (*OnInit)(struct wl*);
	int (*OnMouseover)();

	void *widget_info; // Pointer to specific widget data
	struct wl *next; // Pointer to the next widget in the window
}widget_list;

typedef struct {
	float size; // Text size
	float r,g,b; // Text color
	char text[256]; // Text
}label;

int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)());
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());

int add_label(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y, Uint32 flags, float size, float r, float g, float b);
int draw_label(widget_list *W);

#endif
