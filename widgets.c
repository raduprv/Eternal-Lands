#include "global.h"
#include "widgets.h"
#include "elwindows.h"
#include <string.h>

Uint32 widget_id = 0;

// Common widget functions
widget_list * widget_find(Uint32 window_id, Uint32 widget_id)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id)
			return w;
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
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->len_x = x;
		w->len_y = y;
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
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
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
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	label *T = (label *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(label));

	// Filling the widget info
	W->widget_info = T;
	W->id=widget_id++;
	W->type = LABEL;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = -1.0;
	W->g = -1.0;
	W->b = -1.0;
	strncpy(T->text,text,255);
	W->len_y = (Uint16)(18 * 1.0);
	W->len_x = (Uint16)(strlen(T->text) * 11 * 1.0);
	W->OnDraw = label_draw;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	image *T = (image *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(image));

	// Filling the widget info
	W->widget_info = T;
	W->id=widget_id++;
	W->type = IMAGE;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = 1.0;
	W->g = 1.0;
	W->b = 1.0;
	T->u1 = u1;
	T->u2 = u2;
	T->v1 = v1;
	T->v2 = v2;
	T->id = id;
	W->len_y = lx;
	W->len_x = ly;
	W->OnDraw = image_draw;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	checkbox *T = (checkbox *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(checkbox));

	// Filling the widget info
	W->widget_info = T;
	W->id=widget_id++;
	W->type = CHECKBOX;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = -1.0;
	W->g = -1.0;
	W->b = -1.0;
	T->checked = checked;
	W->len_y = lx;
	W->len_x = ly;
	W->OnDraw = checkbox_draw;
	W->OnClick = checkbox_click;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	button *T = (button *) malloc(sizeof(button));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(button));

	// Filling the widget info
	W->widget_info = T;
	W->id=widget_id++;
	W->type = BUTTON;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = -1.0;
	W->g = -1.0;
	W->b = -1.0;
	strncpy(T->text,text,255);
	W->len_y = (Uint16)(18 * 1.0) + 2;
	W->len_x = (Uint16)(strlen(T->text) * 11 * 1.0) + 4;
	W->OnDraw = button_draw;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	progressbar *T = (progressbar *) malloc(sizeof(progressbar));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(progressbar));

	// Filling the widget info
	W->widget_info = T;
	W->id=widget_id++;
	W->type = PROGRESSBAR;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = -1.0;
	W->g = -1.0;
	W->b = -1.0;
	W->len_y = ly;
	W->len_x = lx;
	W->OnDraw = progressbar_draw;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int lenght, int vlenght)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	vscrollbar *T = (vscrollbar *) malloc(sizeof(vscrollbar));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));
	memset(T,0,sizeof(vscrollbar));

	// Filling the widget info
	W->widget_info = T;
	T->pos_inc = 1;
	W->id=widget_id++;
	W->type = VSCROLLBAR;
	W->Flags = 0;
	W->pos_x = x;
	W->pos_y = y;
	W->size = 1.0;
	W->r = -1.0;
	W->g = -1.0;
	W->b = -1.0;
	W->len_y = ly;
	W->len_x = lx;
	W->OnClick = vscrollbar_click;
	W->OnDraw = vscrollbar_draw;
	W->OnDrag = vscrollbar_drag;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

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
	glVertex3i(W->pos_x + 7, 25+ c->pos,0);
	glVertex3i(W->pos_x + W->len_x - 7, 25+c->pos,0);
	glVertex3i(W->pos_x + W->len_x - 7, 45+c->pos,0);
	glVertex3i(W->pos_x + 7, 45+c->pos,0);
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
			b->pos = y - 20;

	if(b->pos < 0) b->pos = 0;
	if(b->pos > (W->len_y -50)) b->pos = W->len_y -50;

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

int vscrollbar_drag(widget_list *W, int dx, int dy)
{
	vscrollbar *b = (vscrollbar *)W->widget_info;
	b->pos+=dy;

	if(b->pos < 0) b->pos = 0;
	if(b->pos > (W->len_y -50)) b->pos = W->len_y -50;

	return 1;

}

