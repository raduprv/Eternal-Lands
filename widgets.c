#include "global.h"
#include "widgets.h"
#include "elwindows.h"

Uint32 widget_id = 0;

// Common widget functions
int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->OnDraw = handler;
			return 1;
		}
	}
	return 0;
}

int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->OnClick = handler;
			return 1;
		}
	}
	return 0;
}

int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->OnDrag = handler;
			return 1;
		}
	}
	return 0;
}

int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->OnMouseover = handler;
			return 1;
		}
	}
	return 0;
}

int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->pos_x = x;
			w->pos_y = y;
			return 1;
		}
	}
	return 0;
}

int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->len_x = x;
			w->len_y = y;
			return 1;
		}
	}
	return 0;
}

int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->Flags = f;
			return 1;
		}
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
	T->size = 1.0;
	T->r = -1.0;
	T->g = -1.0;
	T->b = -1.0;
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
	if(l->r != -1.0)
		glColor3f(l->r,l->g,l->b);
	draw_string_zoomed(W->pos_x,W->pos_y,(unsigned char *)l->text,1,l->size);
	return 1;
}


int label_set_size(Uint32 window_id, Uint32 widget_id, float size)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			label *l = (label *)w->widget_info;
			w->len_y = (Uint16)(18 * size);
			w->len_x = (Uint16)(strlen(l->text) * 11 * size);
			l->size = size;
			return 1;
		}
	}
	return 0;
}

int label_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			label *l = (label *) w->widget_info;
			l->r = r;
			l->g = g;
			l->b = b;
			return 1;
		}
	}
	return 0;
}