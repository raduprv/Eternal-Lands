#include "global.h"
#include "widgets.h"
#include "elwindows.h"
#include <string.h>

/*!
 * \name    Available Widget types -> moved to widgets.c
 */
/*! \{ */
#define LABEL		1	/*!< a label widget (static text) */
#define IMAGE		2	/*!< image widget */
#define CHECKBOX	3	/*!< checkbox widget type */
#define BUTTON		4	/*!< button widget */
#define PROGRESSBAR	5	/*!< progressbar widget */
#define VSCROLLBAR	6	/*!< vertical scrollbar widget */
#define TABCOLLECTION	7	/*!< tabbed windows collection widget */
#define TEXTFIELD	8	/*!< text field widget */
/*! \} */

/*!
 * Widget label
 */
typedef struct {
	char text[256]; /*!< Text */
}label;

/*!
 * Image structure
 */
typedef struct {
	float u1,v1,u2,v2; /*!< Texture coordinates */
	int id;            /*!< Texture id */
}image;

/*!
 *  Checkbox structure
 */
typedef struct {
	int checked;
}checkbox;

/*!
 *  Button structure
 */
typedef struct {
	char text[256];
}button;

/*!
 *  Progressbar structure
 */
typedef struct {
	float progress;
}progressbar;

/*!
 *  Vertical scrollbar structure
 */
typedef struct {
	int pos, pos_inc, bar_len;
}vscrollbar;

/*!
 *  Tabbed window structure
 */
typedef struct {
	Sint8 label[64];
	Uint32 content_id;
	Uint16 tag_x, tag_width;
	float label_r, label_g, label_b;
	char closable;
} tab;

/*!
 *  Tab collection structure
 */
typedef struct {
	int tag_height, tag_space, nr_tabs, max_tabs, cur_tab;
	tab *tabs;
} tab_collection;

Uint32 widget_id = 0x0000FFFF;

/* forward declarations added due to code cleanup */
int ReadXMLWindow(xmlNode * a_node);
int ParseWindow (xmlNode *node);
int ParseWidget (xmlNode *node, int winid);
int ParseTab (xmlNode *node, int winid, int colid);
int GetWidgetType (const char *w);
/* end of added forward declarations */

// Common widget functions
widget_list * widget_find(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w;

	if (window_id < 0 || window_id >= windows_list.num_windows) return NULL;
	if (windows_list.window[window_id].window_id != window_id) return NULL;
	
	w = windows_list.window[window_id].widgetlist;
	while(w != NULL)
	{
		if(w->id == widget_id)
			return w;
		w = w->next;
	}
	
	return NULL;
}

int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnDraw = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnClick = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnDrag = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnMouseover = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnKey ( Uint32 window_id, Uint32 widget_id, int (*handler)() )
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		w->OnKey = handler;
		return 1;
	}
	return 0;
}

int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->pos_x = x;
		w->pos_y = y;
		return 1;
	}
	return 0;
}

int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		w->len_x = x;
		w->len_y = y;
		if (w->OnResize) w->OnResize (w, x, y);
		return 1;
	}
	return 0;
}

int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->Flags = f;
		return 1;
	}
	return 0;
}

int widget_set_size(Uint32 window_id, Uint32 widget_id, float size)
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		w->size = size;
		return 1;
	}
	return 0;
}

int widget_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->r = r;
		w->g = g;
		w->b = b;
		return 1;
	}
	return 0;
}


// Label
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y)
{
	return label_add_extended (window_id, widget_id++, OnInit, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int free_widget_info (widget_list *widget)
{
	free (widget->widget_info);
	return 1;
}

int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	label *T = (label *) malloc(sizeof(label));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(label));

	// Filling the widget info
	W->widget_info = T;
	W->id = wid;
	W->type = LABEL;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	strncpy(T->text,text,255);
	W->len_y = (Uint16)(18 * 1.0);
	W->len_x = (Uint16)(strlen(T->text) * 11 * 1.0);
	W->OnDraw = label_draw;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int label_draw(widget_list *W)
{
	label *l = (label *)W->widget_info;
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);
	draw_string_zoomed(W->pos_x,W->pos_y,(unsigned char *)l->text,1,W->size);
	return 1;
}

int label_set_text(Uint32 window_id, Uint32 widget_id, char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		label *l = (label *) w->widget_info;
		strncpy(l->text,text,255);
		return 1;
	}
	return 0;
}

// Image
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2)
{
	return image_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 1.0, 1.0, 1.0, id, u1, v1, u2, v2); 
}

int image_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int id, float u1, float v1, float u2, float v2)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	image *T = (image *) malloc(sizeof(label));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(image));

	// Filling the widget info
	W->widget_info = T;
	W->id = wid;
	W->type = IMAGE;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	T->u1 = u1;
	T->u2 = u2;
	T->v1 = v1;
	T->v2 = v2;
	T->id = id;
	W->len_y = lx;
	W->len_x = ly;
	W->OnDraw = image_draw;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;

}

int image_draw(widget_list *W)
{
	image *i = (image *)W->widget_info;
	get_and_set_texture_id(i->id);
	glColor3f(W->r, W->g, W->b);
	glBegin(GL_QUADS);
	draw_2d_thing(i->u1, i->v1, i->u2, i->v2, W->pos_x, W->pos_y, W->pos_x + (W->len_x * W->size), W->pos_y + (W->len_y * W->size));
	glEnd();
	return 1;
}

int image_set_id(Uint32 window_id, Uint32 widget_id, int id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		image *l = (image *) w->widget_info;
		l->id = id;
		return 1;
	}
	return 0;
}

int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		image *l = (image *) w->widget_info;
		l->u1 = u1;
		l->u2 = u2;
		l->v1 = v1;
		l->v2 = v2;
		return 1;
	}
	return 0;
}


// Checkbox
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int checked)
{
	return checkbox_add_extended(window_id, widget_id++, NULL, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, checked);
}

int checkbox_add_extended(Uint32 window_id,  Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int checked)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	checkbox *T = (checkbox *) malloc(sizeof(label));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(checkbox));

	// Filling the widget info
	W->widget_info = T;
	W->id = wid;
	W->type = CHECKBOX;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	T->checked = checked;
	W->len_y = lx;
	W->len_x = ly;
	W->OnDraw = checkbox_draw;
	W->OnClick = checkbox_click;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int checkbox_draw(widget_list *W)
{
	checkbox *c = (checkbox *)W->widget_info;
	glDisable(GL_TEXTURE_2D);
	if(W->r!=-1.0)
		glColor3f(W->r, W->g, W->b);
	glBegin(c->checked ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	return 1;
}

int checkbox_click(widget_list *W)
{
	checkbox *c = (checkbox *)W->widget_info;
	c->checked = !c->checked;
	return 1;
}

int checkbox_get_checked(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		return c->checked;
	}
	return -1;
}

int checkbox_set_checked(Uint32 window_id, Uint32 widget_id, int checked)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		c->checked = checked;
		return 1;
	}
	return 0;
}


// Button
int button_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y)
{
	return button_add_extended(window_id, widget_id++, NULL, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	button *T = (button *) malloc(sizeof(button));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(button));

	// Filling the widget info
	W->widget_info = T;
	W->id = wid;
	W->type = BUTTON;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	strncpy(T->text,text,255);
	W->len_y = (Uint16)(18 * 1.0) + 2;
	W->len_x = (Uint16)(strlen(T->text) * 11 * 1.0) + 4;
	W->OnDraw = button_draw;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
	glDisable(GL_TEXTURE_2D);
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);

	glBegin(GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	draw_string_zoomed(W->pos_x + 2, W->pos_y + 2, (unsigned char *)l->text, 1, W->size);
	return 1;
}

int button_set_text(Uint32 window_id, Uint32 widget_id, char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		button *l = (button *) w->widget_info;
		strncpy(l->text,text,255);
		return 1;
	}
	return 0;
}


// Progressbar
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return progressbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0);
}

int progressbar_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	progressbar *T = (progressbar *) malloc(sizeof(progressbar));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(progressbar));

	// Filling the widget info
	W->widget_info = T;
	W->id = wid;
	W->type = PROGRESSBAR;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	W->len_y = ly;
	W->len_x = lx;
	T->progress = progress;
	W->OnDraw = progressbar_draw;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int progressbar_draw(widget_list *W)
{
	progressbar *b = (progressbar *)W->widget_info;
	int pixels = (b->progress/100) * W->len_x;
	glDisable(GL_TEXTURE_2D);
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);

	glBegin(GL_LINES);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.40f,0.40f,1.00f);
	glVertex3i(W->pos_x + 1, W->pos_y + 0, 0);
	glVertex3i(W->pos_x + pixels,W->pos_y + 0,0);
	glColor3f(0.10f,0.10f,0.80f);
	glVertex3i(W->pos_x + pixels, W->pos_y + W->len_y - 1, 0);
	glVertex3i(W->pos_x + 1, W->pos_y + W->len_y - 1, 0);
	glColor3f(0.77f,0.57f,0.39f);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);
	return 0;
}

float progressbar_get_progress(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		progressbar *c = (progressbar *)w->widget_info;
		return c->progress;
	}
	return -1;
}

int progressbar_set_progress(Uint32 window_id, Uint32 widget_id, float progress)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		progressbar *c = (progressbar *)w->widget_info;
		c->progress = progress;
		return 1;
	}
	return 0;
}


// Vertical scrollbar
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return vscrollbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, 1, ly);
}

int vscrollbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc, int bar_len)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	vscrollbar *T = (vscrollbar *) malloc(sizeof(vscrollbar));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(vscrollbar));

	// Filling the widget info
	W->widget_info = T;
	T->pos_inc = pos_inc;
	T->pos = pos;
	W->id = wid;
	W->type = VSCROLLBAR;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	W->len_y = ly;
	W->len_x = lx;
	T->bar_len = bar_len;
	W->OnClick = vscrollbar_click;
	W->OnDraw = vscrollbar_draw;
	W->OnDrag = vscrollbar_drag;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int vscrollbar_draw(widget_list *W)
{
	vscrollbar *c = (vscrollbar *)W->widget_info;
	glDisable(GL_TEXTURE_2D);
	if(W->r!=-1.0)
		glColor3f(W->r, W->g, W->b);
	glBegin(GL_LINES);

	// scrollbar border
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);

	// scrollbar arrows
	glVertex3i(W->pos_x + 5, W->pos_y + 10,0);
	glVertex3i(W->pos_x + 10, W->pos_y + 5,0);
	glVertex3i(W->pos_x + 10, W->pos_y + 5,0);
	glVertex3i(W->pos_x + 15, W->pos_y + 10,0);
	glVertex3i(W->pos_x + 5, W->pos_y + W->len_y - 10,0);
	glVertex3i(W->pos_x + 10, W->pos_y + W->len_y - 5,0);
	glVertex3i(W->pos_x + 10, W->pos_y + W->len_y - 5,0);
	glVertex3i(W->pos_x + 15, W->pos_y + W->len_y - 10,0);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3i(W->pos_x + 7, W->pos_y + 15 + (c->pos*((float)(W->len_y-50)/c->bar_len)), 0);
	glVertex3i(W->pos_x + W->len_x - 7, W->pos_y +  15 + (c->pos*((float)(W->len_y-50)/c->bar_len)), 0);
	glVertex3i(W->pos_x + W->len_x - 7, W->pos_y + 35 + (c->pos*((float)(W->len_y-50)/c->bar_len)), 0);
	glVertex3i(W->pos_x + 7, W->pos_y + 35 + (c->pos*((float)(W->len_y-50)/c->bar_len)), 0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	return 0;
}

int vscrollbar_click(widget_list *W, int x, int y)
{
	vscrollbar *b = (vscrollbar *)W->widget_info;
	if (y<15)
			b->pos -= b->pos_inc;
	else
		if(y>(W->len_y-15))
			b->pos += b->pos_inc;
		else
			b->pos = (y - 25)/((float)(W->len_y-50)/b->bar_len);

	if(b->pos < 0) b->pos = 0;
	if(b->pos > (b->bar_len)) b->pos = b->bar_len;

	return 1;
}

int vscrollbar_set_pos_inc(Uint32 window_id, Uint32 widget_id, int pos_inc)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		c->pos_inc = pos_inc;
		return 1;
	}

	return 0;
}

int vscrollbar_set_pos(Uint32 window_id, Uint32 widget_id, int pos)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		if (pos < 0)
			c->pos = 0;
		else if (pos > c->bar_len)
		 	c->pos = c->bar_len;
		else
			c->pos = pos;
		return 1;
	}

	return 0;
}

int vscrollbar_set_bar_len (Uint32 window_id, Uint32 widget_id, int bar_len)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		c->bar_len = bar_len;
		return 1;
	}

	return 0;
}

int vscrollbar_drag(widget_list *W, int x, int y, int dx, int dy)
{
	vscrollbar_click(W,x,y);
	return 1;
}

int vscrollbar_get_pos(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		return c->pos;
	}
	return -1;
}

// Tab collection
int tab_collection_get_tab (Uint32 window_id, Uint32 widget_id) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
	{
		tab_collection *col = (tab_collection *) w->widget_info;
		return col->cur_tab;
	}
	return -1;
}

int tab_collection_get_tab_id (Uint32 window_id, Uint32 widget_id) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
	{
		tab_collection *col = (tab_collection *) w->widget_info;
		int tab = col->cur_tab;
		if (tab >= 0 && tab < col->nr_tabs)
			return col->tabs[tab].content_id;
	}
	return -1;
}

int tab_set_label_color_by_id (Uint32 window_id, Uint32 col_id, Uint32 tab_id, float r, float g, float b)
{
	widget_list *w = widget_find (window_id, col_id);
	
	if (w) 
	{
		int itab;
		tab_collection *col = (tab_collection *) w->widget_info;
		
		for (itab = 0; itab < col->nr_tabs; itab++)
		{
			if (col->tabs[itab].content_id == tab_id)
			{
				col->tabs[itab].label_r = r;
				col->tabs[itab].label_g = g;
				col->tabs[itab].label_b = b;
				return tab_id;
			}
		}
	}
	return -1;
}

int tab_collection_select_tab (Uint32 window_id, Uint32 widget_id, int tab) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
	{
		tab_collection *col = (tab_collection *) w->widget_info;
		if (tab >= 0 && tab < col->nr_tabs)
		{
			if (tab != col->cur_tab) 
				hide_window (col->tabs[col->cur_tab].content_id);
			col->cur_tab = tab;

			// Don't show the tab, because the parent window might
			// be hidden. The widget drawing code will take care 
			// of it.
			//show_window (col->tabs[tab].content_id);
			//select_window (col->tabs[tab].content_id);
			return tab;
		}
	}
	return -1;
}

int tab_collection_close_tab (Uint32 window_id, Uint32 widget_id, int tab) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
	{
		tab_collection *col = (tab_collection *) w->widget_info;
		if (tab >= 0 && tab < col->nr_tabs)
		{
			int i, w;
			
			w = col->tabs[tab].tag_width + col->tag_space;
			destroy_window (col->tabs[tab].content_id);
			for (i = tab+1; i < col->nr_tabs; i++)
			{
				col->tabs[i].tag_x -= w;
				col->tabs[i-1] = col->tabs[i];
			}

			col->nr_tabs--;
			if (col->cur_tab > tab)
				col->cur_tab--;
			else if (col->cur_tab == tab)
				col->cur_tab = 0;
				
			return col->cur_tab;
		}
	}
	return -1;
}

int free_tab_collection (widget_list *widget)
{
	tab_collection *col = (tab_collection *) widget->widget_info;
	free (col->tabs);
	free (col);
	return 1;
}

int tab_collection_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint16 tag_height, Uint16 tag_space)
{
	return tab_collection_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, tag_height, tag_space);
}

int tab_collection_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int max_tabs, Uint16 tag_height, Uint16 tag_space)
{
	int itab;
	widget_list *W = (widget_list *) malloc (sizeof (widget_list));
	tab_collection *T = (tab_collection *) malloc (sizeof (tab_collection));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(vscrollbar));

	// Filling the widget info
	T->max_tabs =  max_tabs <= 0 ? 2 : max_tabs;
	T->tabs = malloc (T->max_tabs * sizeof (tab));
	memset (T->tabs, 0, T->max_tabs * sizeof (tab));
	// initialize all tabs content ids to -1 (unitialized window
	for (itab = 0; itab < T->max_tabs; itab++)
		T->tabs[itab].content_id = -1;
	
	W->widget_info = T;
	T->nr_tabs = 0;
	T->tag_height = tag_height;
	T->tag_space = tag_space;
	T->cur_tab = 0;
	W->id = wid;
	W->type = TABCOLLECTION;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	W->len_y = ly;
	W->len_x = lx;
	W->OnClick = tab_collection_click;
	W->OnDraw = tab_collection_draw;
	W->OnDestroy = free_tab_collection;
	W->OnInit = OnInit;
	W->OnResize = tab_collection_resize;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int tab_collection_draw (widget_list *w)
{
	tab_collection *col;
	int itab, ytagtop, ytagbot, xstart, xend, space;
	int h;

	if (!w) return 0;
	
	col = (tab_collection *) w->widget_info;
	
	space = col->tag_space;
	h = col->tag_height;
	ytagtop = w->pos_y;
	ytagbot = w->pos_y + h;
	
	glDisable(GL_TEXTURE_2D);

	if (col->nr_tabs > 0) {
		// draw the tags
		for (itab = 0; itab < col->nr_tabs; itab++) 
		{
			if(w->r!=-1.0)
				glColor3f(w->r, w->g, w->b);

			xstart = w->pos_x + col->tabs[itab].tag_x;
			xend = xstart + col->tabs[itab].tag_width;

			glBegin (GL_LINES);

			glVertex3i (xstart, ytagbot, 0);
			glVertex3i (xstart, ytagtop, 0);
			glVertex3i (xstart, ytagtop, 0);
			glVertex3i (xend, ytagtop, 0);
			glVertex3i (xend, ytagtop, 0);
			glVertex3i (xend, ytagbot, 0);
			
			// draw a close box if necessary
			if (col->tabs[itab].closable)
			{
				glVertex3i (xstart+3, ytagbot-3, 0);
				glVertex3i (xstart+3, ytagtop+3, 0);
				glVertex3i (xstart+3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagbot-3, 0);
				glVertex3i (xstart+h-3, ytagbot-3, 0);
				glVertex3i (xstart+3, ytagbot-3, 0);

				glVertex3i (xstart+3, ytagbot-3, 0);
				glVertex3i (xstart+h-3, ytagtop+3, 0);
				glVertex3i (xstart+3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagbot-3, 0);
			}
			glEnd ();

			glEnable(GL_TEXTURE_2D);
			
			if (col->tabs[itab].label_r >= 0.0f)
				glColor3f (col->tabs[itab].label_r, col->tabs[itab].label_g, col->tabs[itab].label_b); 
				
			if (col->tabs[itab].closable)
				draw_string_zoomed (xstart+h, ytagtop+2, (unsigned char *)col->tabs[itab].label, 1, w->size);
			else
				draw_string_zoomed (xstart+2, ytagtop+2, (unsigned char *)col->tabs[itab].label, 1, w->size);
			glDisable(GL_TEXTURE_2D);
		}
	}

	if(w->r!=-1.0)
		glColor3f(w->r, w->g, w->b);

	xstart = w->pos_x + col->tabs[col->cur_tab].tag_x;
	xend = xstart + col->tabs[col->cur_tab].tag_width;
	
	// draw the rest of the frame around the tab
	glBegin (GL_LINES);
	glVertex3i (w->pos_x, ytagbot, 0);
	glVertex3i (xstart, ytagbot, 0);
	glVertex3i (xend, ytagbot, 0);
	glVertex3i (w->pos_x + w->len_x, ytagbot, 0);		
	glVertex3i (w->pos_x + w->len_x, ytagbot, 0);		
	glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, ytagbot, 0);		
	glEnd ();
	glEnable(GL_TEXTURE_2D);
	
	// show the content of the current tab
	if (col->nr_tabs > 0)
		show_window (col->tabs[col->cur_tab].content_id);
	
	return 1;
}

int tab_collection_click (widget_list *W, int x, int y)
{
	tab_collection *col = (tab_collection *) W->widget_info;
	if (y < col->tag_height) 
	{
		int itag, ctag = col->cur_tab;

		// find which tag was clicked
		for (itag = 0; itag < col->nr_tabs; itag++) 
			if (x > col->tabs[itag].tag_x && x < col->tabs[itag].tag_x + col->tabs[itag].tag_width)
				break;
		
		if (itag < col->nr_tabs)
		{
			// check if close box was clicked
			if (col->tabs[itag].closable && x > col->tabs[itag].tag_x + 3 && x < col->tabs[itag].tag_x + col->tag_height - 3 && y > 3 && y < col->tag_height - 3)
			{
				int i, w;

				w = col->tabs[itag].tag_width + col->tag_space;

				destroy_window (col->tabs[itag].content_id);
				col->tabs[itag].content_id = -1;

				for (i = itag+1; i < col->nr_tabs; i++)
				{
					col->tabs[i].tag_x -= w;
					col->tabs[i-1] = col->tabs[i];
				}

				col->nr_tabs--;
				if (ctag > itag)
					col->cur_tab--;
				else if (ctag == itag)
					col->cur_tab = 0;
			}
			// check if a new tab is selected
			else if (itag != ctag)
			{
				col->cur_tab = itag;
				hide_window (col->tabs[ctag].content_id);
				show_window (col->tabs[itag].content_id);
				//select_window (col->tabs[itag].content_id);
			}
			return 1;
		}
	}
	
	return 0;
}

int tab_collection_resize (widget_list *W, Uint32 width, Uint32 height)
{
	Uint16 win_height;
	tab_collection *col;
	int itab;

	if (W == NULL || (col = (tab_collection *) W->widget_info) == NULL)
		return 0;
	
	win_height = height > col->tag_height ? height - col->tag_height : 0;
	for (itab = 0; itab < col->nr_tabs; itab++)
		resize_window (col->tabs[itab].content_id, width, win_height);
	
	return 1;
}

int tab_add (Uint32 window_id, Uint32 col_id, const char *label, Uint16 tag_width, int closable)
{
	widget_list *w = widget_find(window_id, col_id);
	tab_collection *col;
	int nr;
	
	if (w == NULL || (col = (tab_collection *) w->widget_info) == NULL)
		return 0;
	
	nr = col->nr_tabs++;
	if (nr >= col->max_tabs)
	{		
		// shoot, we allocated too few tabs
		int old_max = col->max_tabs, new_max = 2 * old_max;
		int itab;
		
		col->tabs = realloc ( col->tabs, new_max * sizeof (tab) );
		memset ( &(col->tabs[old_max]), 0, (new_max-old_max) * sizeof (tab) );
		for (itab = old_max; itab < new_max; itab++)
			col->tabs[itab].content_id = -1;
		col->max_tabs = new_max;
	}
		
	my_strncp (col->tabs[nr].label, label, sizeof (col->tabs[nr].label));
	col->tabs[nr].content_id = create_window ("", window_id, 0, w->pos_x, w->pos_y + col->tag_height, w->len_x, w->len_y - col->tag_height, ELW_TITLE_NONE);
	col->tabs[nr].closable = closable ? 1 : 0;

	if (nr == 0)
		col->tabs[nr].tag_x = 0;
	else
		col->tabs[nr].tag_x = col->tabs[nr-1].tag_x + col->tabs[nr-1].tag_width + col->tag_space;

	if (tag_width > 0)
	{
		col->tabs[nr].tag_width = tag_width;
	}
	else
	{
		// compute tag width from label width
		col->tabs[nr].tag_width = 4 + (int) (w->size * get_string_width(col->tabs[nr].label));
		if (col->tabs[nr].closable) 
			col->tabs[nr].tag_width += col->tag_height;
	}
	
	// set label color to default values
	col->tabs[nr].label_r = -1.0f;
	col->tabs[nr].label_g = -1.0f;
	col->tabs[nr].label_b = -1.0f;
	
	return col->tabs[nr].content_id;
}

// text field
int text_field_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, text_message *buf, int buf_size, int x_space, int y_space)
{
	return text_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, TEXT_FIELD_BORDER, 1.0, -1.0, -1.0, -1.0, buf, buf_size, CHANNEL_ALL, x_space, y_space, -1.0, -1.0, -1.0);
}

int text_field_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, text_message *buf, int buf_size, int chan_nr, int x_space, int y_space, float text_r, float text_g, float text_b)
{
	widget_list *W = malloc ( sizeof (widget_list) );
	text_field *T = malloc ( sizeof (text_field) );
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset ( W, 0, sizeof (widget_list) );
	memset ( T, 0, sizeof (text_field) );
	
	// Filling the widget info
	W->widget_info = T;
	T->x_space = x_space,
	T->y_space = y_space,
	T->msg = 0;
	T->offset = 0;
	T->buffer = buf;
	T->buf_size = buf_size;
	T->chan_nr = chan_nr;
	T->cursor = (Flags & TEXT_FIELD_EDITABLE) ? 0 : -1;
	T->text_r = text_r;
	T->text_g = text_g;
	T->text_b = text_b;
	W->id = wid;
	W->type = TEXTFIELD;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	W->len_y = ly;
	W->len_x = lx;
	W->OnDraw = text_field_draw;
	W->OnDestroy = free_widget_info;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	if (w == NULL)
	{
		windows_list.window[window_id].widgetlist = W;
	}
	else
	{
		while(w->next != NULL)
			w = w->next;
		w->next = W;
	}

	return W->id;
}

int text_field_draw (widget_list *w)
{
	text_field *tf;
	Uint8 ch;

	if (w == NULL) return 0;
	
	tf = (text_field *) w->widget_info;
	if (w->Flags & TEXT_FIELD_BORDER)
	{
		// draw the frame
		glDisable (GL_TEXTURE_2D);
		if(w->r!=-1.0)
			glColor3f (w->r, w->g, w->b);

		glBegin (GL_LINES);

		glVertex3i (w->pos_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y, 0);

		glVertex3i (w->pos_x + w->len_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);

		glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);
		glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);

		glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);
		glVertex3i (w->pos_x, w->pos_y, 0);

		glEnd ();
		
		glEnable (GL_TEXTURE_2D);
	}
	
	if (tf->text_r >= 0.0f)
	{
		glColor3f (tf->text_r, tf->text_g, tf->text_b);
	}
	else
	{		
		ch = tf->buffer[tf->msg].data[tf->offset];
		if (ch < 127 || ch > 127 + c_grey4)
		{
			// search backwards for the last color
			int ichar;
		
			for (ichar = tf->offset-1; ichar >= 0; ichar--)
			{
				ch = tf->buffer[tf->msg].data[ichar];
				if (ch >= 127 && ch <= 127 + c_grey4)
				{
					float r, g, b;
					ch -= 127;
					r = colors_list[ch].r1 / 255.0f;
					g = colors_list[ch].g1 / 255.0f;
					b = colors_list[ch].b1 / 255.0f;
					glColor3f (r, g, b);
					break;
				}
			}
		}
	}
	
	draw_messages (w->pos_x + tf->x_space, w->pos_y + tf->y_space, tf->buffer, tf->buf_size, tf->chan_nr, tf->msg, tf->offset, tf->cursor, w->len_x - 2 * tf->x_space, w->len_y - 2*tf->y_space, w->size);
	
	return 1;
}

int text_field_set_buf_pos (Uint32 window_id, Uint32 widget_id, int msg, int offset)
{
	text_field *tf;
	widget_list *w = widget_find (window_id, widget_id);
	if (w == NULL) return 0;
	
	tf = (text_field *) w->widget_info;

	if (msg < 0)
	{
		msg = 0;
	}
	else if (msg >= tf->buf_size)
	{
		msg = tf->buf_size - 1;
	}
		
	if (offset < 0) 
	{
		offset = 0;
	}
	else if (offset >= tf->buffer[msg].len)
	{
		offset = tf->buffer[msg].len - 1;
	}
		
	tf->msg = msg;
	tf->offset = offset;

	return  1;
}

// XML Windows

int AddXMLWindow (char *fn)
{
	xmlDocPtr doc = xmlReadFile (fn, NULL, 0);
	int w;

	if (doc == NULL)
		return -1;

	w = ReadXMLWindow( xmlDocGetRootElement (doc) );
	xmlFreeDoc(doc);

	return w;
}

int ReadXMLWindow(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;
	static int win;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next)
	{
		if (cur_node->type == XML_ELEMENT_NODE)
		{
			win = ParseWindow (cur_node);
		}
	}
	
	return win;
}

int ParseWindow (xmlNode *node)
{
	xmlAttr *cur_attr = NULL;
	xmlNode *child = NULL;

	Uint8 name[256] = "Default";
	int pos_x = 0, pos_y = 0, size_x = 320, size_y = 240;
	Uint32 flags = ELW_WIN_DEFAULT;
	int winid;

	for (cur_attr = node->properties; cur_attr; cur_attr = cur_attr->next)
	{
		if (cur_attr->type == XML_ATTRIBUTE_NODE)
		{
			//name=""
			if ( !xmlStrcasecmp (cur_attr->name,"name") )
			{
				char *p = name;
				my_xmlStrncopy (&p, cur_attr->children->content, sizeof (name) );
				continue;
			}
			//pos_x=""
			if ( !xmlStrcasecmp (cur_attr->name, "pos_x") )
			{
				pos_x = atoi (cur_attr->children->content);
				continue;
			}
			//pos_y=""
			if (!xmlStrcasecmp (cur_attr->name, "pos_y"))
			{
				pos_y = atoi (cur_attr->children->content);
				continue;
			}
			//size_x=""
			if ( !xmlStrcasecmp (cur_attr->name, "size_x") )
			{
				size_x = atoi (cur_attr->children->content);
				continue;
			}
			//size_y=""
			if ( !xmlStrcasecmp (cur_attr->name, "size_y") )
			{
				size_y = atoi (cur_attr->children->content);
				continue;
			}
			//flags=""
			if ( !xmlStrcasecmp (cur_attr->name, "flags") )
			{
				flags = atoi (cur_attr->children->content);
				continue;
			}
		}
	}
	
	winid = create_window (name, -1, 0, pos_x, pos_y, size_x, size_y, flags);
	
	for (child = node->children; child; child = child->next)
	{
		if (child->type == XML_ELEMENT_NODE)
			ParseWidget (child, winid);
	}
	
	return winid;
}

int ParseWidget (xmlNode *node, int winid)
{
	xmlAttr *cur_attr = NULL;

	int pos_x = 0, pos_y = 0, len_x = 0, len_y = 0, type = GetWidgetType (node->name), checked = 1, pos = 0, pos_inc = 1;
	Uint32 flags = 0, id = widget_id++, tid = 0, max_tabs = 0, tag_height = 0, tag_space = 0;
	float size = 1.0, r = 1.0, g = 1.0, b = 1.0, u1 = 0.0, u2 = 1.0, v1 = 0.0, v2 = 1.0, progress = 0.0;
	char text[256] = {'\0'};

	for (cur_attr = node->properties; cur_attr; cur_attr = cur_attr->next)
	{
		if (cur_attr->type == XML_ATTRIBUTE_NODE)
		{
			//pos_x=""
			if ( !xmlStrcasecmp (cur_attr->name, "pos_x") )
			{
				pos_x = atoi (cur_attr->children->content);
				continue;
			}
			//pos_y=""
			if ( !xmlStrcasecmp (cur_attr->name, "pos_y") )
			{
				pos_y = atoi (cur_attr->children->content);
				continue;
			}
			//len_x=""
			if ( !xmlStrcasecmp (cur_attr->name, "len_x") )
			{
				len_x = atoi (cur_attr->children->content);
				continue;
			}
			//len_y=""
			if ( !xmlStrcasecmp (cur_attr->name, "len_y") )
			{
				len_y = atoi (cur_attr->children->content);
				continue;
			}
			//flags=""
			if ( !xmlStrcasecmp (cur_attr->name, "flags") )
			{
				flags = atoi (cur_attr->children->content);
				continue;
			}
			//size=""
			if ( !xmlStrcasecmp (cur_attr->name, "size") )
			{
				size = atof (cur_attr->children->content);
				continue;
			}
			//r=""
			if ( !xmlStrcasecmp (cur_attr->name, "r") )
			{
				r = atof (cur_attr->children->content);
				continue;
			}
			//g=""
			if ( !xmlStrcasecmp (cur_attr->name, "g") )
			{
				g = atof (cur_attr->children->content);
				continue;
			}
			//b=""
			if ( !xmlStrcasecmp (cur_attr->name, "b") )
			{
				b = atof (cur_attr->children->content);
				continue;
			}
			//id=""
			if ( !xmlStrcasecmp (cur_attr->name,"id") )
			{
				id = atof (cur_attr->children->content);
				widget_id--;
				continue;
			}

			switch (type)
			{
				case LABEL:
				case BUTTON:
					//text=""
					if ( !xmlStrcasecmp (cur_attr->name, "text") )
					{
						char *p = text;
						my_xmlStrncopy (&p, cur_attr->children->content, 255);
						continue;
					}
					break;
				case IMAGE:
					//u1=""
					if ( !xmlStrcasecmp (cur_attr->name,"u1") )
					{
						u1 = atof (cur_attr->children->content);
						continue;
					}
					//u2=""
					if ( !xmlStrcasecmp (cur_attr->name, "u2") )
					{
						u2 = atof (cur_attr->children->content);
						continue;
					}
					//v1=""
					if ( !xmlStrcasecmp (cur_attr->name, "v1") )
					{
						v1 = atof (cur_attr->children->content);
						continue;
					}
					//v2=""
					if ( !xmlStrcasecmp (cur_attr->name, "v2") )
					{
						v2 = atof (cur_attr->children->content);
						continue;
					}
					//id=""
					if ( !xmlStrcasecmp (cur_attr->name, "tid") )
					{
						tid = atoi (cur_attr->children->content);
						continue;
					}
					break;

				case CHECKBOX:
					//checked=""
					if ( !xmlStrcasecmp (cur_attr->name, "checked") )
					{
						checked = atoi (cur_attr->children->content);
						continue;
					}
					break;

				case PROGRESSBAR:
					//progress=""
					if ( !xmlStrcasecmp (cur_attr->name, "progress") )
					{
						progress = atof (cur_attr->children->content);
						continue;
					}
					break;

				case VSCROLLBAR:
					//pos=""
					if ( !xmlStrcasecmp (cur_attr->name, "pos") )
					{
						pos = atoi (cur_attr->children->content);
						continue;
					}
					//pos_inc=""
					if ( !xmlStrcasecmp (cur_attr->name, "pos_inc") )
					{
						pos_inc = atoi (cur_attr->children->content);
						continue;
					}
					break;
				
				case TABCOLLECTION:
					//max_tabs=""
					if ( !xmlStrcasecmp (cur_attr->name, "max_tabs") )
					{
						max_tabs = atoi (cur_attr->children->content);
						continue;
					}
					//tag_height=""
					if ( !xmlStrcasecmp (cur_attr->name, "tag_height") )
					{
						tag_height = atoi (cur_attr->children->content);
						continue;
					}
					//tag_space=""
					if ( !xmlStrcasecmp (cur_attr->name, "tag_space") )
					{
						tag_space = atoi (cur_attr->children->content);
						continue;
					}
					break;
			}
		}
	}

	switch(type)
	{
		case LABEL:
			return label_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, text);
		case IMAGE:
			return image_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, tid, u1, v1, u2, v2);
		case CHECKBOX:
			return checkbox_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, checked);
		case BUTTON:
			return button_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, text);
		case PROGRESSBAR:
			return progressbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, progress);
		case VSCROLLBAR:
			return vscrollbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, pos, pos_inc, len_y);
		case TABCOLLECTION:
		{
			xmlNode *tab;
			int colid = tab_collection_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, max_tabs, tag_height, tag_space);
			for (tab = node->children; tab; tab = tab->next)
			{
				if (tab->type == XML_ELEMENT_NODE)
					ParseTab (tab, winid, colid);
			}
			
			return colid;
		}
	}

	return -1;
}

int ParseTab (xmlNode *node, int winid, int colid)
{
	int tag_width = 0, closable = 0, tabid;
	char label[64] = {'\0'};
	xmlAttr *cur_attr = NULL;
	xmlNode *child;

	for (cur_attr = node->properties; cur_attr; cur_attr = cur_attr->next)
	{
		if (cur_attr->type == XML_ATTRIBUTE_NODE)
		{
			if ( !xmlStrcasecmp (cur_attr->name, "label") )
			{
				char *p = label;
				my_xmlStrncopy ( &p, cur_attr->children->content, sizeof (label) );
			}
			else if ( !xmlStrcasecmp (cur_attr->name, "tag_width") )
			{
				tag_width = atoi (cur_attr->children->content);
			}
			else if ( !xmlStrcasecmp (cur_attr->name, "closable") )
			{
				closable = atoi (cur_attr->children->content);
			}
		}
	}
	
	tabid = tab_add (winid, colid, label, tag_width, closable);
	
	for (child = node->children; child; child = child->next)
	{
		if (child->type == XML_ELEMENT_NODE)
			ParseWidget (node, tabid);
	}
	
	return tabid;
}

int GetWidgetType (const char *w)
{
	if(!xmlStrcasecmp(w, "LABEL"))
		return LABEL;
	if(!xmlStrcasecmp(w, "IMAGE"))
		return IMAGE;
	if(!xmlStrcasecmp(w, "CHECKBOX"))
		return CHECKBOX;
	if(!xmlStrcasecmp(w, "BUTTON"))
		return BUTTON;
	if(!xmlStrcasecmp(w, "PROGRESSBAR"))
		return PROGRESSBAR;
	if(!xmlStrcasecmp(w, "VSCROLLBAR"))
		return VSCROLLBAR;
	if(!xmlStrcasecmp(w, "TABCOLLECTION"))
		return TABCOLLECTION;

	return 0;
}


