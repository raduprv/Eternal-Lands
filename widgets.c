#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "widgets.h"
#include "elwindows.h"
#include <string.h>

typedef struct {
	char text[256];
}label;

typedef struct {
	float u1,v1,u2,v2;
	int id;
}image;

typedef struct {
	int *checked;
}checkbox;

typedef struct {
	char text[256];
}button;

typedef struct {
	float progress;
	GLfloat colors[12];
}progressbar;

typedef struct {
	int pos, pos_inc, bar_len;
}vscrollbar;

typedef struct {
	char * password;
	int status;
	int max_chars;
}password_entry;

typedef struct {
	char text[256];
	Uint16 x;
	Uint16 y;
	int width;
}multiselect_button;

typedef struct {
	int nr_buttons;
	int selected_button;
	int max_buttons;
	multiselect_button *buttons;
	/* Scrollbar related vars */
	Uint16 max_height;
	Uint16 actual_height;
	Uint32 scrollbar;
	Uint32 win_id;
	float highlighted_red;
	float highlighted_green;
	float highlighted_blue;
}multiselect;

Uint32 widget_id = 0x0000FFFF;

int ReadXMLWindow(xmlNode * a_node);
int ParseWindow (xmlNode *node);
int ParseWidget (xmlNode *node, int winid);
int ParseTab (xmlNode *node, int winid, int colid);
int GetWidgetType (const char *w);

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

int widget_destroy (Uint32 window_id, Uint32 widget_id)
{
	widget_list *w, *n;

	if (window_id < 0 || window_id >= windows_list.num_windows) return 0;
	if (windows_list.window[window_id].window_id != window_id) return 0;
	
	n = windows_list.window[window_id].widgetlist;
	if (n == NULL)
	{
		// shouldn't happen
		return 0;
	}
	
	if (n->id == widget_id)
	{
		if (n->OnDestroy != NULL)
			n->OnDestroy (n);
		windows_list.window[window_id].widgetlist = n->next;
		free (n);
		return 1;
	}
	else
	{
		while (1)
		{
			w = n;
			n = n->next;
			if (n == NULL)
			{
				// shouldn't happen
				return 0;
			}
			if (n->id == widget_id)
			{
				if (n->OnDestroy != NULL)
					n->OnDestroy (n);
				w->next = n->next;
				free (n);
				return 1;
			}
		}
	}
	
	// shouldn't get here	
	return 0;
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

int widget_set_OnDestroy (Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w)
	{
		w->OnDestroy = handler;
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

int widget_move_rel (Uint32 window_id, Uint32 widget_id, Sint16 dx, Sint16 dy)
{
	widget_list *w = widget_find (window_id, widget_id);
	if(w)
	{
		w->pos_x += dx;
		w->pos_y += dy;
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

int widget_get_width (Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if (w != NULL)
	{
		return w->len_x;
	}
	return -1;
}

int free_widget_info (widget_list *widget)
{
	free (widget->widget_info);
	return 1;
}

widget_list *add_new_widget (int window_id, Uint32 wid, int (*OnInit)(), Uint32 type, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b)
{
	widget_list *newW = calloc (1, sizeof (widget_list));
	widget_list *Ws = windows_list.window[window_id].widgetlist;
	
	newW->id = wid;
	newW->window_id = window_id;
	newW->type = type;
	newW->Flags = Flags;
	newW->pos_x = x;
	newW->pos_y = y;
	newW->len_x = lx;
	newW->len_y = ly;
	newW->size = size;
	newW->r = r;
	newW->g = g;
	newW->b = b;
	newW->OnInit = OnInit;
		
	// Adding the widget to the list
	if (Ws == NULL)
	{
		windows_list.window[window_id].widgetlist = newW;
	}
	else
	{
		while (Ws->next != NULL)
			Ws = Ws->next;
		Ws->next = newW;
	}	
	
	return newW;
}

// Label
int label_add(Uint32 window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return label_add_extended (window_id, widget_id++, OnInit, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, const char *text)
{
	Uint16 len_x = (Uint16)(strlen (text) * 11 * 1.0);
	Uint16 len_y = (Uint16)(18 * 1.0);
	widget_list *W = add_new_widget (window_id, wid, OnInit, LABEL, x, y, len_x, len_y, Flags, size, r, g, b);
	
	label *T = (label *) calloc (1, sizeof(label));
	snprintf (T->text, sizeof(T->text), "%s", text);
	
	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = label_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit (W);

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
		snprintf(l->text, sizeof(l->text), "%s", text);
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
	widget_list *W = add_new_widget (window_id, wid, OnInit, IMAGE, x, y, lx, ly, Flags, size, r, g, b);

	image *T = calloc (1, sizeof (image));
	T->u1 = u1;
	T->u2 = u2;
	T->v1 = v1;
	T->v2 = v2;
	T->id = id;

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = image_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

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
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int *checked)
{
	if(checked == NULL)
	{
		checked = calloc(1,sizeof(*checked));
	}
	return checkbox_add_extended(window_id, widget_id++, NULL, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, checked);
}

int checkbox_add_extended(Uint32 window_id,  Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int *checked)
{
	widget_list *W = add_new_widget (window_id, wid, OnInit, CHECKBOX, x, y, lx, ly, Flags, size, r, g, b);

	checkbox *T = calloc (1, sizeof (checkbox));
	T->checked = checked;

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = checkbox_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int checkbox_draw(widget_list *W)
{
	checkbox *c = (checkbox *)W->widget_info;
	glDisable(GL_TEXTURE_2D);
	if(W->r!=-1.0)
		glColor3f(W->r, W->g, W->b);
	glBegin(*c->checked ? GL_QUADS: GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	return 1;
}

int checkbox_click (widget_list *W, Uint32 flags)
{
	checkbox *c = (checkbox *)W->widget_info;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	*c->checked = !*c->checked;
	return 1;
}

int checkbox_get_checked(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		return *c->checked;
	}
	return -1;
}

int checkbox_set_checked(Uint32 window_id, Uint32 widget_id, int checked)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		*c->checked = checked;
		return 1;
	}
	return 0;
}


// Button
int button_add(Uint32 window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return button_add_extended(window_id, widget_id++, NULL, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, const char *text)
{
	Uint16 len_x = lx > 0 ? lx : (Uint16)(strlen(text) * 11 * size) + 30*size;
	Uint16 len_y = ly > 0 ? ly : (Uint16)(18 * size) + 12*size;
	widget_list *W = add_new_widget (window_id, wid, OnInit, BUTTON, x, y, len_x, len_y, Flags, size, r, g, b);

	button *T = calloc (1, sizeof(button));
	snprintf (T->text, sizeof(T->text), "%s", text);

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = button_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
	float extra_space = (W->len_x - get_string_width(l->text)*W->size)/2.0f;
	if(extra_space < 0) {
		extra_space = 0;
	}

	draw_smooth_button((unsigned char *)l->text, W->size, W->pos_x, W->pos_y, W->len_x-20*W->size, 1, W->r, W->g, W->b, 0, 0.0f, 0.0f, 0.0f, 0.0f);
	
	return 1;
}

int square_button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
	float extra_space = (W->len_x - get_string_width(l->text)*W->size*(0.11f/0.12f))/2.0f;
	if(extra_space < 0) {
		extra_space = 0;
	}

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
	draw_string_zoomed(W->pos_x + 2 + extra_space, W->pos_y + 2, (unsigned char *)l->text, 1, W->size);

	return 1;
}

int button_set_text(Uint32 window_id, Uint32 widget_id, char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		button *l = (button *) w->widget_info;
		snprintf(l->text, sizeof(l->text), "%s",  text);
		return 1;
	}
	return 0;
}

// Progressbar
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return progressbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0.0f, NULL);
}

int progressbar_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress, const float * colors)
{
	widget_list *W = add_new_widget (window_id, wid, OnInit, PROGRESSBAR, x, y, lx, ly, Flags, size, r, g, b);

	progressbar *T = calloc (1, sizeof(progressbar));
	T->progress = progress;
	if (colors) {
		memcpy(T->colors, colors, sizeof(T->colors));
	} else {
		T->colors[0] = -1.0f;
	}

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = progressbar_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int progressbar_draw(widget_list *W)
{
	progressbar *b = (progressbar *)W->widget_info;
	int pixels = (b->progress/100) * W->len_x;

	glDisable(GL_TEXTURE_2D);
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);
	else
		glColor3f(0.77f,0.57f,0.39f);

	glBegin(GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd();

	if (pixels > 0) {
		const char have_bar_colors = (b->colors[0] > -0.5f);
		GLfloat right_colors[6];

		if (have_bar_colors) {
			const float progress = b->progress/100.0f, inv_progress = 1.0f - progress;
			int i;

			for (i=0; i<3; i++) {
				right_colors[i+0] = progress * b->colors[i+3] + inv_progress * b->colors[i+0];
				right_colors[i+3] = progress * b->colors[i+6] + inv_progress * b->colors[i+9];
			}
		}

		glBegin(GL_QUADS);
			if (have_bar_colors) glColor3fv(&b->colors[0]);
			glVertex3i(W->pos_x + 1, W->pos_y + 0, 0);
			if (have_bar_colors) glColor3fv(&right_colors[0]);
			glVertex3i(W->pos_x + pixels,W->pos_y + 0,0);
			if (have_bar_colors) glColor3fv(&right_colors[3]);
			glVertex3i(W->pos_x + pixels, W->pos_y + W->len_y - 1, 0);
			if (have_bar_colors) glColor3fv(&b->colors[9]);
			glVertex3i(W->pos_x + 1, W->pos_y + W->len_y - 1, 0);
			glColor3f(0.77f,0.57f,0.39f);
		glEnd();
	}
	
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
	widget_list *W = add_new_widget (window_id, wid, OnInit, VSCROLLBAR, x, y, lx, ly, Flags, size, r, g, b);

	vscrollbar *T = calloc (1, sizeof(vscrollbar));
	T->pos_inc = pos_inc;
	T->pos = pos;
	T->bar_len = bar_len;

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = vscrollbar_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int vscrollbar_draw(widget_list *W)
{
	vscrollbar *c = (vscrollbar *)W->widget_info;
	glDisable(GL_TEXTURE_2D);
	if(W->r!=-1.0)
		glColor3f(W->r, W->g, W->b);

	// scrollbar border
	glBegin(GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd ();

	// scrollbar arrows
	glBegin (GL_LINES);
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

int vscrollbar_click (widget_list *W, int my, Uint32 flags)
{
	vscrollbar *b = (vscrollbar *)W->widget_info;
	if ( my < 15 || (flags & ELW_WHEEL_UP) )
	{
		b->pos -= b->pos_inc;
	}
	else if (my > W->len_y - 15 || (flags & ELW_WHEEL_DOWN) )
	{
		b->pos += b->pos_inc;
	}
	else
	{
		b->pos = (my - 25)/((float)(W->len_y-50)/b->bar_len);
	}

	if (b->pos < 0) b->pos = 0;
	if (b->pos > b->bar_len) b->pos = b->bar_len;
	
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

int vscrollbar_scroll_up(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *scrollbar = w->widget_info;
		return vscrollbar_set_pos(window_id, widget_id, scrollbar->pos - scrollbar->pos_inc);
	}
	return 0;
}

int vscrollbar_scroll_down(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *scrollbar = w->widget_info;
		return vscrollbar_set_pos(window_id, widget_id, scrollbar->pos + scrollbar->pos_inc);
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

int vscrollbar_drag(widget_list *W, int x, int y, Uint32 flags, int dx, int dy)
{
	vscrollbar_click(W,y,flags);
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
		int tab;
		tab_collection *col = (tab_collection *) w->widget_info;
		tab = col->cur_tab;
		if (tab >= 0 && tab < col->nr_tabs)
			return col->tabs[tab].content_id;
	}
	return -1;
}

int tab_collection_get_tab_nr (Uint32 window_id, Uint32 col_id, Uint32 tab_id) 
{
	widget_list *w = widget_find (window_id, col_id);
	if (w != NULL) 
	{
		int tab;
		tab_collection *col = (tab_collection *) w->widget_info;
		for (tab = 0; tab < col->nr_tabs; tab++)
			if (col->tabs[tab].content_id == tab_id)
				return tab;
	}
	return -1;
}

int tab_collection_get_nr_tabs (Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		tab_collection *t = (tab_collection *) w->widget_info;
		return t->nr_tabs;
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
			
			w = col->tabs[tab].tag_width;
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

int tab_collection_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint16 tag_height)
{
	return tab_collection_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, tag_height);
}

int tab_collection_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int max_tabs, Uint16 tag_height)
{
	widget_list *W = add_new_widget (window_id, wid, OnInit, TABCOLLECTION, x, y, lx, ly, Flags, size, r, g, b);

	int itab;
	tab_collection *T = calloc (1, sizeof (tab_collection));
	T->max_tabs =  max_tabs <= 0 ? 2 : max_tabs;
	T->tabs = calloc (T->max_tabs, sizeof (tab));
	// initialize all tabs content ids to -1 (unitialized window_
	for (itab = 0; itab < T->max_tabs; itab++)
		T->tabs[itab].content_id = -1;
	T->nr_tabs = 0;
	T->tag_height = tag_height;
	T->cur_tab = 0;
	
	W->widget_info = T;
	W->OnDraw = tab_collection_draw;
	W->OnDestroy = free_tab_collection;
	W->OnResize = tab_collection_resize;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int tab_collection_draw (widget_list *w)
{
	tab_collection *col;
	int itab, ytagtop, ytagbot, xstart, xend;
	int h;

	if (!w) return 0;
	
	col = (tab_collection *) w->widget_info;
	
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

			if(col->cur_tab == itab){
				glBegin(GL_LINE_STRIP);
					glVertex3i(xstart, ytagbot, 0);
					draw_circle_ext(xstart, ytagtop, DEFAULT_TAB_RADIUS, -10, 180, 90);
					draw_circle_ext(xend-2*DEFAULT_TAB_RADIUS, ytagtop, DEFAULT_TAB_RADIUS, -10, 89, 0);
					glVertex3i(xend, ytagbot, 0);
				glEnd();
			} else if(col->cur_tab>itab){
				glBegin (GL_LINE_STRIP);
					glVertex3i (xstart, ytagbot, 0);
					draw_circle_ext(xstart, ytagtop, DEFAULT_TAB_RADIUS, -10, 180, 90);
					glVertex3i (xend, ytagtop+1, 0);
				glEnd ();
			} else {
				glBegin (GL_LINE_STRIP);
					glVertex3i (xstart, ytagtop+1, 0);
					draw_circle_ext(xend-2*DEFAULT_TAB_RADIUS, ytagtop, DEFAULT_TAB_RADIUS, -10, 89, 0);
					glVertex3i (xend, ytagbot, 0);
				glEnd ();
			}
			if(itab+1<col->nr_tabs){
				glBegin(GL_LINES);
				glVertex2i(xend-DEFAULT_TAB_RADIUS, ytagtop+1);
				glVertex2i(xend, ytagtop+1);
				glEnd();
			}
			if(itab){
				glBegin(GL_LINES);
					glVertex2i(xstart+DEFAULT_TAB_RADIUS, ytagtop+1);
					glVertex2i(xstart, ytagtop+1);
				glEnd();
			}
			
			// draw a close box if necessary
			if (col->tabs[itab].closable)
			{
				glBegin (GL_LINE_LOOP);
				glVertex3i (xstart+3, ytagbot-3, 0);
				glVertex3i (xstart+3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagbot-3, 0);
				glEnd ();

				glBegin (GL_LINES);
				glVertex3i (xstart+3, ytagbot-3, 0);
				glVertex3i (xstart+h-3, ytagtop+3, 0);
				glVertex3i (xstart+3, ytagtop+3, 0);
				glVertex3i (xstart+h-3, ytagbot-3, 0);
				glEnd ();
			}

			glEnable(GL_TEXTURE_2D);
			
			if (col->tabs[itab].label_r >= 0.0f)
				glColor3f (col->tabs[itab].label_r, col->tabs[itab].label_g, col->tabs[itab].label_b); 
				
			if (col->tabs[itab].closable)
				draw_string_zoomed (xstart+h, ytagbot-SMALL_FONT_Y_LEN-2, (unsigned char *)col->tabs[itab].label, 1, w->size);
			else
				draw_string_zoomed (xstart+4, ytagbot-SMALL_FONT_Y_LEN-2, (unsigned char *)col->tabs[itab].label, 1, w->size);
			glDisable(GL_TEXTURE_2D);
		}
	}

	if(w->r!=-1.0)
		glColor3f(w->r, w->g, w->b);

	xstart = w->pos_x + col->tabs[col->cur_tab].tag_x;
	xend = xstart + col->tabs[col->cur_tab].tag_width;
	
	// draw the rest of the frame around the tab
	glBegin (GL_LINE_STRIP);
	glVertex3i (xend, ytagbot, 0);
	glVertex3i (w->pos_x + w->len_x, ytagbot, 0);		
	glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, ytagbot, 0);		
	glVertex3i (xstart, ytagbot, 0);
	glEnd ();

	glEnable(GL_TEXTURE_2D);
	
	// show the content of the current tab
	if (col->nr_tabs > 0)
		show_window (col->tabs[col->cur_tab].content_id);
	
	return 1;
}

int tab_collection_click (widget_list *W, int x, int y, Uint32 flags)
{
	tab_collection *col = (tab_collection *) W->widget_info;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;
	
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

				w = col->tabs[itag].tag_width;

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

int tab_collection_keypress (widget_list *W, Uint32 key)
{
	int shift_on = key & ELW_SHIFT;
	Uint16 keysym = key & 0xffff;
	tab_collection *col = (tab_collection *) W->widget_info;
	
	if (col->nr_tabs <= 0) return 0;
	
	if (shift_on && keysym == K_ROTATERIGHT)
	{
		if (col->nr_tabs == 1) return 1;
		hide_window (col->tabs[col->cur_tab].content_id);
		if (++col->cur_tab >= col->nr_tabs)
			col->cur_tab = 0;
		return 1;
	}
	else if (shift_on && keysym == K_ROTATELEFT)
	{
		if (col->nr_tabs == 1) return 1;
		hide_window (col->tabs[col->cur_tab].content_id);
		if (--col->cur_tab < 0)
			col->cur_tab = col->nr_tabs - 1;
		return 1;
	}

	return 0;	
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
		col->tabs[nr].tag_x = col->tabs[nr-1].tag_x + col->tabs[nr-1].tag_width;

	if (tag_width > 0)
	{
		col->tabs[nr].tag_width = tag_width;
	}
	else
	{
		// compute tag width from label width
		col->tabs[nr].tag_width = 10 + ((float)w->size * (DEFAULT_FONT_X_LEN * (float)get_string_width(col->tabs[nr].label)/12.0f));
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
int text_field_keypress (widget_list *w, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	Uint8 ch = key_to_char(unikey);
	text_field *tf;
	text_message *msg;
	int alt_on = key & ELW_ALT, ctrl_on = key & ELW_CTRL;
	
	if (w == NULL) return 0;
	if ( !(w->Flags & TEXT_FIELD_EDITABLE) ) return 0;
	if (w->Flags & TEXT_FIELD_NO_KEYPRESS) return 0;
	
	tf = (text_field *) w->widget_info;
	msg = &(tf->buffer[tf->msg]);
	
	if (keysym == K_ROTATELEFT)
	{
		if (tf->cursor > 0) 
		{
			do
			{
				tf->cursor--;
			}
			while (tf->cursor > 0 && msg->data[tf->cursor] == '\r');
		}
			
		return 1;
	}
	else if (keysym == K_ROTATERIGHT)
	{
		if (tf->cursor < msg->len)
		{
			do
			{
				tf->cursor++;
			} while (tf->cursor < msg->len && msg->data[tf->cursor] == '\r');
		}
		return 1;
	}
	else if (keysym == SDLK_HOME)
	{
		tf->cursor = 0;
		return 1;
	}
	else if (keysym == SDLK_END)
	{
		tf->cursor = msg->len;
		return 1;
	}
	else if (ch == SDLK_BACKSPACE && tf->cursor > 0)
	{
		int i = tf->cursor, n = 1;
		
		while (n < i && msg->data[i-n] == '\r') n++;
		
		for ( ; i <= msg->len; i++)
			msg->data[i-n] = msg->data[i];

		tf->cursor -= n;
		msg->len -= n;
		tf->nr_lines = rewrap_message (msg, w->size, w->len_x, &tf->cursor);	
		return 1;
	}
	else if (ch == SDLK_DELETE && tf->cursor < msg->len)
	{
		int i = tf->cursor, n = 1;
		
		while (i+n <= msg->len && msg->data[i+n] == '\r') n++;
		
		for (i += n; i <= msg->len; i++)
			msg->data[i-n] = msg->data[i];

		msg->len -= n;
		tf->nr_lines = rewrap_message (msg, w->size, w->len_x, &tf->cursor);
		return 1;
	}
	else if ( !alt_on && !ctrl_on && ( (ch >= 32 && ch <= 126) || (ch > 127 + c_grey4) || ch == SDLK_RETURN ) && ch != '`' )
	{
		int nr_lines;
	
		if (ch == SDLK_RETURN) ch = '\n';
		
		// keep one position free, so that we can always introduce a
		// soft line break if necessary.
		if (msg->len >= msg->size-2)
		{
			if (w->Flags & TEXT_FIELD_CAN_GROW)
			{
				msg->size *= 2;
				msg->data = realloc (msg->data, msg->size * sizeof (char) );
			}
		}
		tf->cursor += put_char_in_buffer (msg, ch, tf->cursor);
		nr_lines = rewrap_message (msg, w->size, w->len_x, &tf->cursor);
		
		if (nr_lines != tf->nr_lines)
		{
			msg->len += nr_lines - tf->nr_lines;
			tf->nr_lines = nr_lines;
		}

		return 1;
	}
	return 0;
}

// XXX rewrite: there's bound to be a simpler way to do this.
unsigned int get_edit_pos(unsigned short x, unsigned short y, char *str, unsigned int maxchar, float text_zoom)
{
	unsigned short i = 0, c = 0, k = 0;
	unsigned short nnls = 0, ncs = 0;
	float displayed_font_x_size = 11.0 * text_zoom;
	float displayed_font_y_size = 18.0 * text_zoom;

	ncs = x/displayed_font_x_size;
	nnls = y/displayed_font_y_size;

	if (c == nnls)
	{   
		while (k < ncs && str[i+k] != '\0')
		{
			if (str[i+k] == '\r' || str[i+k] == '\n')
			{
				return i+k;
			}
			k++;
		}
		return i+k;
	}
     
	while (i < maxchar && str[i] != '\0')
	{
		if (str[i] == '\n' || str[i] == '\r')
		{
			c++;
			if (c == nnls)
			{
				i++;
				while (k < ncs && str[i+k] != '\0')
				{
					if (str[i+k] == '\r' || str[i+k] == '\n')
					{
						return i+k;
					}
					k++;
				}
				return i+k;
			}
		}
		i++;
	}
	return i+k;
} 

int text_field_click (widget_list *w, int mx, int my, Uint32 flags)
{
	text_field *tf;
	text_message *msg;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;
        
	if ( (w->Flags & TEXT_FIELD_EDITABLE) == 0) return 0;

	tf = (text_field *) w->widget_info;
	msg = &(tf->buffer[tf->msg]);
	tf->cursor = get_edit_pos (mx, my, msg->data, msg->len, 1);
    
	return 1;
}

int text_field_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, text_message *buf, int buf_size, int x_space, int y_space)
{
	return text_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, TEXT_FIELD_BORDER, 1.0, -1.0, -1.0, -1.0, buf, buf_size, FILTER_ALL, x_space, y_space, -1.0, -1.0, -1.0);
}

int text_field_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, text_message *buf, int buf_size, Uint8 chan_filt, int x_space, int y_space, float text_r, float text_g, float text_b)
{
	widget_list *W = add_new_widget (window_id, wid, OnInit, TEXTFIELD, x, y, lx, ly, Flags, size, r, g, b);

	text_field *T = calloc (1, sizeof (text_field));
	T->x_space = x_space,
	T->y_space = y_space,
	T->msg = 0;
	T->offset = 0;
	T->buffer = buf;
	T->buf_size = buf_size;
	T->nr_lines = 0;
	T->chan_nr = chan_filt;
	T->cursor = (Flags & TEXT_FIELD_EDITABLE) ? 0 : -1;
	T->text_r = text_r;
	T->text_g = text_g;
	T->text_b = text_b;
	
	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = text_field_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);

	return W->id;
}

int text_field_draw (widget_list *w)
{
	text_field *tf;

	if (w == NULL) return 0;
	
	tf = (text_field *) w->widget_info;
	if (w->Flags & TEXT_FIELD_BORDER)
	{
		// draw the frame
		glDisable (GL_TEXTURE_2D);
		if(w->r!=-1.0)
			glColor3f (w->r, w->g, w->b);

		glBegin (GL_LINE_LOOP);
		glVertex3i (w->pos_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);
		glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);
		glEnd ();
		
		glEnable (GL_TEXTURE_2D);
	}
	
	if (tf->text_r >= 0.0f)
	{
		glColor3f (tf->text_r, tf->text_g, tf->text_b);
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

//password entry field. We act like a restricted text entry with multiple modes
// quite straightforward - we just add or remove from the end
int pword_keypress (widget_list *w, int mx, int my, Uint32 key, Uint32 unikey)
{
       Uint8 ch = key_to_char(unikey);
       password_entry *pword;
       int alt_on = key & ELW_ALT, ctrl_on = key & ELW_CTRL;
	
       if (w == NULL) return 0;
	
       pword = (password_entry *) w->widget_info;

	if (pword->status == P_NONE) return -1;

	if (ch == SDLK_BACKSPACE) {
		int i;
		
		for(i = 0; pword->password[i] != '\0' && i < pword->max_chars; i++) ;
		if(i > 0) pword->password[i-1] = '\0';
		
		return 1;
	} else if ( !alt_on && !ctrl_on && ( (ch >= 32 && ch <= 126) || (ch > 127 + c_grey4) || ch == SDLK_RETURN ) && ch != '`' ) {
		int i;
		
		for(i = 0; pword->password[i] != '\0' && i < pword->max_chars-1; i++);
		if(i >= 0){
				pword->password[i] = ch;
				pword->password[i+1] = '\0';
		}
		
		return 1;
	}

	return 0;
}
		
int pword_field_click(widget_list *w, int mx, int my, Uint32 flags)
{
	password_entry *pword;

	if (w == NULL) return 0;
	pword = (password_entry*) w->widget_info;
	if(pword->status == P_NONE) return -1;
	
	return 1;   // Don't fall through
}

int pword_field_draw (widget_list *w)
{
	password_entry *pword;
	unsigned char *text;
	int difference;
	int i;

	if (w == NULL) return 0;
	pword = (password_entry*) w->widget_info;
	difference = (get_string_width(pword->password)*w->size - w->len_x)/12;

	// draw the frame
	glDisable (GL_TEXTURE_2D);
	glColor3f (w->r, w->g, w->b);
	glBegin (GL_LINE_LOOP);
		glVertex3i (w->pos_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y, 0);
		glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);
		glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);
	if(difference > 0) {
		glVertex3i (w->pos_x + 3, w->pos_y + w->len_y/2, 0);
	}
	glEnd ();
	glEnable (GL_TEXTURE_2D);
	
	if (pword->status == P_NONE) {
		draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, "N/A", 1, w->size);
	} else if(pword->status == P_TEXT) {
		if(difference > 0) {
			/* Only draw the end of the string */
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, pword->password+difference, 1, w->size);
		} else {
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, pword->password, 1, w->size);
		}
	} else if(pword->status == P_NORMAL) {
		text = calloc(1, pword->max_chars);
		for(i = 0; i < pword->max_chars && pword->password[i] != '\0'; i++) {
			text[i] = '*';
		}
		text[i] = '\0';
		if(difference > 0) {
			/* Only draw the end of the string */
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, text+difference, 1, w->size);
		} else {
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, text, 1, w->size);
		}
		free(text);
	}

	return 1;
}

void pword_set_status(widget_list *w, Uint8 status)
{
	password_entry *pword;
	if (w == NULL) return;
	pword = (password_entry*) w->widget_info;
	pword->status = status;
}

int pword_field_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, unsigned char *buffer, int buffer_size)
{
	return pword_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, status, 1.0, -1.0, -1.0, -1.0, buffer, buffer_size);
}
		
int pword_field_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, float size, float r, float g, float b, unsigned char *buffer, int buffer_size)
{
	widget_list *W = add_new_widget (window_id, wid, OnInit, PWORDFIELD, x, y, lx, ly, 0, size, r, g, b);

	password_entry *T = calloc (1, sizeof (password_entry));
	T->status = status;
	T->password = buffer;
	T->max_chars = buffer_size;

	// Filling the widget info
	W->widget_info = T;
	W->OnDraw = pword_field_draw;
	W->OnDestroy = free_widget_info;
	if(W->OnInit != NULL)
		W->OnInit(W);
	
	return W->id;
}

int free_multiselect(widget_list *widget)
{
	if(widget != NULL) {
		multiselect *collection = widget->widget_info;
		free(collection->buttons);
		free(collection);
	}
	return 1;
}

int multiselect_get_selected(Uint32 window_id, Uint32 widget_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M = widget->widget_info;
	if(M == NULL) {
		return -1;
	} else {
		return M->selected_button;
	}
}

int multiselect_set_selected(Uint32 window_id, Uint32 widget_id, int button_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M = widget->widget_info;
	if(M == NULL) {
		return -1;
	} else {
		M->selected_button = button_id;
		return button_id;
	}
}

int multiselect_get_height(Uint32 window_id, Uint32 widget_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	return widget->len_y;
}

int multiselect_click(widget_list *widget, Uint16 mx, Uint16 my, Uint32 flags)
{
	multiselect *M = widget->widget_info;
	int i;
	Uint16 button_y;

	for(i = 0; i < M->nr_buttons; i++) {
		if(M->scrollbar == -1) {
			button_y = M->buttons[i].y;
		} else {
			vscrollbar *scrollbar = widget_find(M->win_id, M->scrollbar)->widget_info;
			if(flags&ELW_WHEEL_DOWN) {
				vscrollbar_set_pos(M->win_id, M->scrollbar, scrollbar->pos+scrollbar->pos_inc);
			} else if (flags&ELW_WHEEL_UP) {
				vscrollbar_set_pos(M->win_id, M->scrollbar, scrollbar->pos-scrollbar->pos_inc);
			}
			button_y = M->buttons[i].y - ceilf((vscrollbar_get_pos(M->win_id, M->scrollbar)/(float)M->max_height)*M->actual_height);
		}
		if((flags&ELW_LEFT_MOUSE || flags&ELW_RIGHT_MOUSE) && 
			my > button_y && my < button_y+22 && mx > M->buttons[i].x && mx < M->buttons[i].x+M->buttons[i].width) {
				M->selected_button = i;
			return 1;
		}
	}
	return 0;
}

int multiselect_draw(widget_list *widget)
{
	if (widget == NULL) {
		return 0;
	} else {
		int i;
		float r, g, b;
		float hr, hg, hb;
		Uint16 button_y;
		multiselect *M = widget->widget_info;

		r = widget->r != -1 ? widget->r : 0.77f;
		g = widget->g != -1 ? widget->g : 0.59f;
		b = widget->b != -1 ? widget->b : 0.39f;
		
		hr = M->highlighted_red != -1 ? M->highlighted_red : 0.32f;
		hg = M->highlighted_green != -1 ? M->highlighted_green : 0.23f;
		hb = M->highlighted_blue != -1 ? M->highlighted_blue : 0.15f;
		
		for(i = 0; i < M->nr_buttons; i++) {
			if(M->buttons[i].width <= 0) {
				continue;
			}
			if(M->scrollbar != -1) {
				int scrollbar_y = ceilf((vscrollbar_get_pos(M->win_id, M->scrollbar)/(float)M->max_height)*M->actual_height);
				/* Check if the button should be drawn and set the y position */
				if(M->buttons[i].y+22 > scrollbar_y+widget->len_y || M->buttons[i].y < scrollbar_y) {
					continue;
				} else {
					button_y = M->buttons[i].y-scrollbar_y;
				}
			} else {
				button_y = M->buttons[i].y;
			}
			draw_smooth_button(M->buttons[i].text, widget->size, widget->pos_x+M->buttons[i].x, widget->pos_y+button_y, M->buttons[i].width-22, 1, r, g, b, (i == M->selected_button), hr, hg, hb, 0.5f);
		}
	}
	return 1;
}

int multiselect_button_add(Uint32 window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, const char *text, const char selected)
{
	return multiselect_button_add_extended(window_id, multiselect_id, x, y, 0, text, DEFAULT_SMALL_RATIO, selected);
}

int multiselect_button_add_extended(Uint32 window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, int width, const char *text, float size, const char selected)
{
	widget_list *widget = widget_find(window_id, multiselect_id);
	multiselect *M = widget->widget_info;
	int current_button = M->nr_buttons;

	if(y+22 > widget->len_y && (!M->max_height || widget->len_y != M->max_height)) {
		widget->len_y = y+22; //22 = button height
	}

	widget->size=size;
	
	if (M->max_height && y+22 > M->actual_height) {
		M->actual_height = y+22;
	}
	if(M->max_buttons == M->nr_buttons) {
		/*Allocate space for more buttons*/
		M->buttons = realloc(M->buttons, sizeof(*M->buttons) * M->max_buttons * 2);
		M->max_buttons *= 2;
	}
	snprintf(M->buttons[current_button].text, sizeof(M->buttons[current_button].text), "%s", text);
	if(selected) {
		M->selected_button = current_button;
	}
	M->buttons[current_button].x = x;
	M->buttons[current_button].y = y;
	M->buttons[current_button].width = (width == 0) ? widget->len_x : width;
	
	M->nr_buttons++;
	if(M->max_height && M->scrollbar == -1 && M->max_height < y) {
		int i;

		/* Add scrollbar */
		M->scrollbar = vscrollbar_add_extended(window_id, widget_id++, NULL, widget->pos_x+widget->len_x-20, widget->pos_y, 20, M->max_height, 0, 1.0, widget->r, widget->g, widget->b, 0, 1, M->max_height);
		widget->len_x -= 20;
		widget->len_y = M->max_height;
		/* We don't want things to look ugly. */
		for(i = 0; i < M->nr_buttons; i++) {
			if(M->buttons[i].width > widget->len_x) {
				M->buttons[i].width -= 20;
			}
		}
	}
	return current_button;
}

int multiselect_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, int width)
{
	return multiselect_add_extended(window_id, widget_id++, OnInit, x, y, width, 0, 1.0f, -1, -1, -1, -1, -1, -1, 0);
}

int multiselect_add_extended(Uint32 window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, int width, Uint16 max_height, float size, float r, float g, float b, float hr, float hg, float hb, int max_buttons)
{
	widget_list *Widget = add_new_widget (window_id, widget_id, OnInit, MULTISELECT, x, y, width, 0, 0, size, r, g, b);

	multiselect *M = calloc (1, sizeof (multiselect));
	//Save info
	M->max_buttons = max_buttons > 0 ? max_buttons : 2;
	M->selected_button = 0;
	M->nr_buttons = 0;
	M->buttons = malloc(sizeof(*M->buttons) * M->max_buttons);
	M->max_height = max_height;
	M->scrollbar = -1;
	M->win_id = window_id;
	M->highlighted_red = hr;
	M->highlighted_green = hg;
	M->highlighted_blue = hb;

	Widget->widget_info = M;
	Widget->OnDraw = multiselect_draw;
	Widget->OnDestroy = free_multiselect;
	if(Widget->OnInit != NULL)
		Widget->OnInit(Widget);

	return Widget->id;
}

int spinbutton_keypress(widget_list *widget, Uint32 key, Uint32 unikey)
{
	spinbutton *button;
	int i;
	int i_tmp;

	if(widget != NULL && (button = widget->widget_info) != NULL &&  !(key&ELW_ALT) && !(key&ELW_CTRL)) {
		char ch = key_to_char(unikey);

		switch(button->type) {
			case SPIN_INT:
				i_tmp = ch-'0'; //Convert char to int
				if(ch >= '0' && ch <= '9') {
					if(*(int *)button->data*10 + i_tmp > button->max) {
						/* Make sure we don't exceed any limits */
						*(int *)button->data = button->max;
						snprintf(button->input_buffer, 255, "%i", (int)button->max);
					} else {
						if(atoi(button->input_buffer) >= button->min) {
							*(int *)button->data = *(int *)button->data * 10 + i_tmp;
						}
						if(button->input_buffer[0] != '0') {
							/* Find end of string */
							for(i = 0; button->input_buffer[i] != '\0' && i < sizeof (button->input_buffer); i++);
							/* Append to the end */
							if (i+1 < sizeof (button->input_buffer))
							{
								button->input_buffer[i] = ch;
								button->input_buffer[i+1] = '\0';
							}
						}
					}
					return 1;
				} else if (ch == SDLK_BACKSPACE) {
					if(strlen(button->input_buffer) > 0) {
						button->input_buffer[strlen(button->input_buffer)-1] = '\0';
					}
					if(*(int *)button->data/10 >= button->min) {
						*(int *)button->data /= 10;
					}
					return 1;
				}
			break;
			case SPIN_FLOAT:
				if(ch == ',') {
					ch = '.';
				}
				if((ch >= '0' && ch <= '9') || (ch == '.' && strstr(button->input_buffer, ".") == NULL)) {
					/* Make sure we don't insert illegal characters here */
					if(button->input_buffer[0] != '0' || ch == '.' || strstr(button->input_buffer, ".") != NULL) {
						/* Find end of string */
						for(i = 0; button->input_buffer[i] != '\0' && i < sizeof (button->input_buffer); i++);
						/* Append to the end */
						if (i+1 < sizeof (button->input_buffer))
						{
							button->input_buffer[i] = ch;
							button->input_buffer[i+1] = '\0';
						}
						if(atof(button->input_buffer) > button->max) {
							snprintf(button->input_buffer, 255, "%.2f", button->max);
						}
					}
					if(atof(button->input_buffer) >= button->min && atof(button->input_buffer) <= button->max) {
						*(float *)button->data = atof(button->input_buffer);
					}
					return 1;
				} else if (ch == SDLK_BACKSPACE) {
					if(strlen(button->input_buffer) > 0) {
						button->input_buffer[strlen(button->input_buffer)-1] = '\0';
					}
					if(atof(button->input_buffer) >= button->min) {
						*(float *)button->data = atof(button->input_buffer);
					}
					return 1;
				}
			break;
		}
	}
	return 0;
}

int spinbutton_click(widget_list *widget, Uint16 mx, Uint16 my, Uint32 flags)
{
	if(widget != NULL && widget->widget_info != NULL) {
		spinbutton *button = widget->widget_info;
		Uint8 action = 0;

		if(flags&ELW_WHEEL_UP) {
			action = 'i'; //i for increase
		} else if (flags&ELW_WHEEL_DOWN) {
			action = 'd'; //d for decrease
		} else if(mx > widget->len_x-20) {
			/* Click on one of the arrows */
			if(my < widget->len_y/2) {
				action = 'i'; //i for increase
			} else {
				action = 'd'; //d for decrease
			}
		} else {
			action = 0;
		}
		if(action) {
			switch (button->type) {
				case SPIN_INT:
					switch (action) {
						case 'i':
							if(*(int *)button->data + button->interval <= button->max) {
								*(int *)button->data += button->interval;
							}
						break;
						case 'd':
							if(*(int *)button->data - button->interval >= button->min) {
								*(int *)button->data -= button->interval;
							}
						break;
					}
					snprintf(button->input_buffer, 255, "%i", *(int *)button->data);
				break;
				case SPIN_FLOAT:
					switch (action) {
						case 'i':
							//if(*(float *)button->data + button->interval <= button->max+0.000001) { //+0.000001 to avoid issues with floating point values
							if(*(float *)button->data + button->interval <= button->max) { // NOTE: Can't do that, values > max may cause crashes. Change the max value intead.
								*(float *)button->data += button->interval;
							}
						break;
						case 'd':
							if(*(float *)button->data - button->interval >= button->min) {
								*(float *)button->data -= button->interval;
							}
						break;
					}
					snprintf(button->input_buffer, 255, "%.2f", *(float *)button->data);
				break;
			}
			return 1;
		}
	}
	return 0;
}

int spinbutton_draw(widget_list *widget)
{
	spinbutton *button;
	char str[255];

	if(widget == NULL || (button = widget->widget_info) == NULL) {
		return 0;
	}
	switch(button->type) {
		case SPIN_INT:
			if(atoi(button->input_buffer) < button->min) {
				/* The input buffer has a value less than minimum. 
				 * Don't change the data variable and mark the text in red */
				glColor3f(1, 0, 0);
				snprintf(str, sizeof (str), "%s", button->input_buffer);
			} else {
				*(int *)button->data = atoi(button->input_buffer);
				snprintf(str, sizeof (str), "%i", *(int *)button->data);
			}
		break;
		case SPIN_FLOAT:
			if(atof(button->input_buffer) < button->min) {
				glColor3f(1, 0, 0);
				snprintf(str, sizeof (str), "%s", button->input_buffer);
			} else {
				char *pointer = strstr(button->input_buffer, ".");
				int accuracy = (pointer == NULL) ? 0 : strlen(pointer+1);
				char format[10];
				if(accuracy > 3) {
					accuracy = 3;
				}
				snprintf(format, sizeof (format), "%%.%if", accuracy);
				snprintf(str, sizeof (str), format, *(float *)button->data);
				if(accuracy == 0 && pointer != NULL) {
					/* We have a . at the end of the input buffer, but 
					 * snprintf() doesn't write it, so we have to do it manually. */
					strncat(str, ".", sizeof(str)-1);
				}
			}
		break;
	}
	/* Numbers */
	draw_string_zoomed(widget->pos_x + 2, widget->pos_y + 2, str, 1, widget->size);
	glDisable(GL_TEXTURE_2D);
	glColor3f(widget->r, widget->g, widget->b);
	/* Border */
	glBegin(GL_LINE_LOOP);
		glVertex3i (widget->pos_x, widget->pos_y, 0);
		glVertex3i (widget->pos_x + widget->len_x, widget->pos_y, 0);
		glVertex3i (widget->pos_x + widget->len_x, widget->pos_y + widget->len_y, 0);
		glVertex3i (widget->pos_x, widget->pos_y + widget->len_y, 0);
	glEnd ();
	/* Line between buttons and input */
	glBegin(GL_LINES);
		glVertex3i(widget->pos_x+widget->len_x-20, widget->pos_y,0);
		glVertex3i(widget->pos_x+widget->len_x-20, widget->pos_y+widget->len_y,0);
	glEnd();
	/* Up arrow */
	glBegin(GL_QUADS);
		glVertex3i(widget->pos_x+widget->len_x-20 + 5, widget->pos_y + widget->len_y/4+2, 0); //Left corner
		glVertex3i(widget->pos_x+widget->len_x-20 + 10, widget->pos_y + 2, 0); //Top
		glVertex3i(widget->pos_x+widget->len_x-20 + 15, widget->pos_y + widget->len_y/4+2, 0); //Right corner
		glVertex3i(widget->pos_x+widget->len_x-20 + 5, widget->pos_y + widget->len_y/4+2, 0); //Back to the beginning
	glEnd();
	/* Button separator */
	glBegin(GL_LINES);
		glVertex3i(widget->pos_x+widget->len_x-20, widget->pos_y+widget->len_y/2,0);
		glVertex3i(widget->pos_x+widget->len_x, widget->pos_y+widget->len_y/2,0);
	glEnd();
	/* Down arrow */
	glBegin(GL_QUADS);
		glVertex3i(widget->pos_x+widget->len_x-20 + 5, widget->pos_y + widget->len_y - widget->len_y/4-2, 0); //Left corner
		glVertex3i(widget->pos_x+widget->len_x-20 + 10, widget->pos_y + widget->len_y - 2, 0); //Bottom
		glVertex3i(widget->pos_x+widget->len_x-20 + 15, widget->pos_y + widget->len_y - widget->len_y/4-2, 0); //Right corner
		glVertex3i(widget->pos_x+widget->len_x-20 + 5, widget->pos_y + widget->len_y - widget->len_y/4-2, 0); //Back to the beginning
	glEnd();
	glEnable(GL_TEXTURE_2D);
	return 1;
}

int spinbutton_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval)
{
	return spinbutton_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, data_type, data, min, max, interval, 1, -1, -1, -1);
}

int spinbutton_add_extended(Uint32 window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval, float size, float r, float g, float b)
{
	float wr = r >= 0 ? r : 0.77, wg = g >= 0 ? g : 0.59, wb = b >= 0 ? b : 0.39;
	widget_list *widget = add_new_widget (window_id, widget_id, OnInit, SPINBUTTON, x, y, lx, ly, 0, size, wr, wg, wb);

	spinbutton *button = calloc (1, sizeof (spinbutton));
	// Filling the widget info
	button->data = data;
	button->max = max;
	button->min = min;
	button->type = data_type;
	button->interval = interval;
	switch(data_type)
	{
		case SPIN_FLOAT:
			snprintf(button->input_buffer, 255, "%.2f", *(float *)button->data);
		break;
		case SPIN_INT:
			button->interval = (int)button->interval;
			snprintf(button->input_buffer, 255, "%i", *(int *)button->data);
		break;
	}

	widget->widget_info = button;
	widget->OnDraw = spinbutton_draw;
	widget->OnDestroy = free_widget_info;
	if(widget->OnInit != NULL) 
		widget->OnInit(widget);

	return widget->id;
}

int widget_handle_mouseover (widget_list *widget, int mx, int my)
{
	if (widget->OnMouseover != NULL)
		return widget->OnMouseover (widget, mx, my);
	
	return 0;
}

int widget_handle_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int res = 0;

	switch (widget->type)
	{
		case CHECKBOX:
			res = checkbox_click (widget, flags);
			break;
		case VSCROLLBAR:
			res = vscrollbar_click (widget, my, flags);
			break;
		case TABCOLLECTION:
			res = tab_collection_click (widget, mx, my, flags);
			break;
		case TEXTFIELD:
			res = text_field_click (widget, mx, my, flags);
			break;
		case PWORDFIELD:
			res = pword_field_click (widget, mx, my, flags);
			if ( res == -1 ) return 0;  // Not really there
			break;
		case MULTISELECT:
			res = multiselect_click (widget, mx, my, flags);
			break;
		case SPINBUTTON:
			res = spinbutton_click (widget, mx, my, flags);
			break;
	}

	if (widget->OnClick != NULL)
		res |= widget->OnClick (widget, mx, my, flags);
	
	return res;
}

int widget_handle_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int res = 0;

	switch (widget->type)
	{
		case VSCROLLBAR:
			res = vscrollbar_drag (widget, mx, my, flags, dx, dy);
			break;
		case SPINBUTTON:
			res = spinbutton_click (widget, mx, my, flags);
			break;
	}

	if (widget->OnDrag != NULL)
		res |= widget->OnDrag (widget, mx, my);
	
	return res;
}

int widget_handle_keypress (widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey)
{
	int res = 0;

	switch (widget->type)
	{
		case TABCOLLECTION:
			res = tab_collection_keypress (widget, key);
			break;
		case TEXTFIELD:
			res = text_field_keypress (widget, mx, my, key, unikey);
			break;
		case PWORDFIELD:
			res = pword_keypress (widget, mx, my, key, unikey);
			if ( res == -1 ) return 0;  // Not really there
			break;
		case SPINBUTTON:
			res = spinbutton_keypress (widget, key, unikey);
			break;
	}

	if (widget->OnKey != NULL)
		res |= widget->OnKey (widget, mx, my, key, unikey);
	
	return res;
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
	Uint32 flags = 0, id = widget_id++, tid = 0, max_tabs = 0, tag_height = 0;
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
		{
			int *checked_ptr = calloc(1,sizeof(int));
			*checked_ptr = checked;
			return checkbox_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, checked_ptr);
		}
		case BUTTON:
			return button_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, text);
		case PROGRESSBAR:
			return progressbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, progress, NULL);
		case VSCROLLBAR:
			return vscrollbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, pos, pos_inc, len_y);
		case TABCOLLECTION:
		{
			xmlNode *tab;
			int colid = tab_collection_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, max_tabs, tag_height);
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
