#include "global.h"
#include "widgets.h"
#include "elwindows.h"

Uint32 widget_id = 0;

int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id)
			w->OnDraw = handler;
	}
	return 1;
}

int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id)
			w->OnClick = handler;
	}
	return 1;
}

int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id)
			w->OnDrag = handler;
	}
	return 1;
}

int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id)
			w->OnMouseover = handler;
	}
	return 1;
}

int add_label(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y, Uint32 flags, float size, float r, float g, float b)
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
	W->Flags = flags;
	W->pos_x = x;
	W->pos_y = y;
	T->size = size;
	T->r = r;
	T->g = g;
	T->b = b;
	strncpy(T->text,text,255);
	W->len_y = 18 * size;
	W->len_x = strlen(T->text) * 11 * size;
	W->OnDraw = draw_label;
	W->OnInit = OnInit;
	if(W->OnInit != NULL)
		W->OnInit(W);

	// Adding the widget to the list
	while(w->next != NULL)
		w = w->next;
	w->next = W;

	return W->id;
}


int draw_label(widget_list *W)
{
	label *l = W->widget_info;
	glColor3f(l->r,l->g,l->b);
	draw_string_zoomed(W->pos_x,W->pos_y,l->text,1,l->size);
	return 1;
}

