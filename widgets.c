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

int widget_set_size(Uint32 window_id, Uint32 widget_id, float size)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->size = size;
			return 1;
		}
	}
	return 0;
}

int widget_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			w->r = r;
			w->g = g;
			w->b = b;
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
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			label *l = (label *) w->widget_info;
			strncpy(l->text,text,255);
			return 1;
		}
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
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			image *l = (image *) w->widget_info;
			l->id = id;
			return 1;
		}
	}
	return 0;
}

int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2)
{
	widget_list *w = &windows_list.window[window_id].widgetlist;
	while(w->next != NULL){
		w = w->next;
		if(w->id == widget_id){
			image *l = (image *) w->widget_info;
			l->u1 = u1;
			l->u2 = u2;
			l->v1 = v1;
			l->v2 = v2;
			return 1;
		}
	}
	return 0;
}



