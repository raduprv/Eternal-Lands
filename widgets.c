#include "global.h"
#include "widgets.h"
#include "elwindows.h"
#include <string.h>

Uint32 widget_id = 0x0000FFFF;

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
	return label_add_extended(window_id, widget_id++, OnInit, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	label *T = (label *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
	return image_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 1.0, 1.0, 1.0, id, u1, v1, u2, v2); 
}

int image_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float id, float u1, float v1, float u2, float v2)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	image *T = (image *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
	return checkbox_add_extended(window_id, widget_id++, NULL, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, checked);
}

int checkbox_add_extended(Uint32 window_id,  Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int checked)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	checkbox *T = (checkbox *) malloc(sizeof(label));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
	return button_add_extended(window_id, widget_id++, NULL, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	button *T = (button *) malloc(sizeof(button));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
	return progressbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0);
}

int progressbar_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	progressbar *T = (progressbar *) malloc(sizeof(progressbar));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return vscrollbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, 1);
}

int vscrollbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	vscrollbar *T = (vscrollbar *) malloc(sizeof(vscrollbar));
	widget_list *w = &windows_list.window[window_id].widgetlist;
	
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
	glVertex3i(W->pos_x + 7, W->pos_y + 15 + c->pos, 0);
	glVertex3i(W->pos_x + W->len_x - 7, W->pos_y +  15 + c->pos, 0);
	glVertex3i(W->pos_x + W->len_x - 7, W->pos_y + 35 + c->pos, 0);
	glVertex3i(W->pos_x + 7, W->pos_y + 35 + c->pos, 0);
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
	b->pos += dy;

	if(b->pos < 0) b->pos = 0;
	if(b->pos > (W->len_y -50)) b->pos = W->len_y -50;

	return 1;

}



// XML Windows

int AddXMLWindow(char *fn)
{
	xmlDocPtr doc = xmlReadFile(fn, NULL, 0);
	int w;

    if (doc == NULL)
		return 0;

    w = ReadXMLWindow(xmlDocGetRootElement(doc));
    xmlFreeDoc(doc);
	return w;
}


int ReadXMLWindow(xmlNode * a_node)
{
    xmlNode *cur_node=NULL;
	static int w;
    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type==XML_ELEMENT_NODE){
			if(!xmlStrcasecmp(cur_node->name,"Window")){
				w = ParseWindow(cur_node->properties);
			}else{
				ParseWidget((char *)cur_node->name, w, cur_node->properties);
			}
		}
		ReadXMLWindow(cur_node->children);
    }
	return w;
}

int ParseWindow(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	Uint8 name[256] = "Default";
	int pos_x = 0, pos_y = 0, size_x = 320, size_y = 240;
	Uint32 flags = ELW_WIN_DEFAULT;

    for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
        if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//name=""
			if(!xmlStrcasecmp(cur_attr->name,"name")){
				int l = strlen(cur_attr->children->content);
				UTF8Toisolat1(name, &l, cur_attr->children->content, &l);
				name[l]=0;
				continue;
			}
			//pos_x=""
			if(!xmlStrcasecmp(cur_attr->name,"pos_x")){
				pos_x = atoi(cur_attr->children->content);
				continue;
			}
			//pos_y=""
			if(!xmlStrcasecmp(cur_attr->name,"pos_y")){
				pos_y = atoi(cur_attr->children->content);
				continue;
			}
			//size_x=""
			if(!xmlStrcasecmp(cur_attr->name,"size_x")){
				size_x = atoi(cur_attr->children->content);
				continue;
			}
			//size_y=""
			if(!xmlStrcasecmp(cur_attr->name,"size_y")){
				size_y = atoi(cur_attr->children->content);
				continue;
			}
			//flags=""
			if(!xmlStrcasecmp(cur_attr->name,"flags")){
				flags = atoi(cur_attr->children->content);
				continue;
			}
		}
	}
	return create_window(name, 0, 0, pos_x, pos_y, size_x, size_y, flags);
}


int ParseWidget(char *wn, int winid, xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	int pos_x = 0, pos_y = 0, len_x = 0, len_y = 0, type = GetWidgetType(wn), checked = 1, pos = 0, pos_inc = 1;
	Uint32 flags = 0, id = widget_id++, tid = 0;
	float size = 1.0, r = 1.0, g = 1.0, b = 1.0, u1 = 0.0, u2 = 1.0, v1 = 0.0, v2 = 1.0, progress = 0;
	char text[256];

    for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
        if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//pos_x=""
			if(!xmlStrcasecmp(cur_attr->name,"pos_x")){
				pos_x = atoi(cur_attr->children->content);
				continue;
			}
			//pos_y=""
			if(!xmlStrcasecmp(cur_attr->name,"pos_y")){
				pos_y = atoi(cur_attr->children->content);
				continue;
			}
			//len_x=""
			if(!xmlStrcasecmp(cur_attr->name,"len_x")){
				len_x = atoi(cur_attr->children->content);
				continue;
			}
			//len_y=""
			if(!xmlStrcasecmp(cur_attr->name,"len_y")){
				len_y = atoi(cur_attr->children->content);
				continue;
			}
			//flags=""
			if(!xmlStrcasecmp(cur_attr->name,"flags")){
				flags = atoi(cur_attr->children->content);
				continue;
			}
			//size=""
			if(!xmlStrcasecmp(cur_attr->name,"size")){
				size = atof(cur_attr->children->content);
				continue;
			}
			//r=""
			if(!xmlStrcasecmp(cur_attr->name,"r")){
				r = atof(cur_attr->children->content);
				continue;
			}
			//g=""
			if(!xmlStrcasecmp(cur_attr->name,"g")){
				g = atof(cur_attr->children->content);
				continue;
			}
			//b=""
			if(!xmlStrcasecmp(cur_attr->name,"b")){
				b = atof(cur_attr->children->content);
				continue;
			}
			//id=""
			if(!xmlStrcasecmp(cur_attr->name,"id")){
				id = atof(cur_attr->children->content);
				widget_id--;
				continue;
			}

			switch(type){
				case LABEL:
				case BUTTON:
					//text=""
					if(!xmlStrcasecmp(cur_attr->name,"text")){
						int l = strlen(cur_attr->children->content);
						UTF8Toisolat1(text, &l, cur_attr->children->content, &l);
						text[l]=0;
						continue;
					}
					break;
				case IMAGE:
					//u1=""
					if(!xmlStrcasecmp(cur_attr->name,"u1")){
						u1 = atof(cur_attr->children->content);
						continue;
					}
					//u2=""
					if(!xmlStrcasecmp(cur_attr->name,"u2")){
						u2 = atof(cur_attr->children->content);
						continue;
					}
					//v1=""
					if(!xmlStrcasecmp(cur_attr->name,"v1")){
						v1 = atof(cur_attr->children->content);
						continue;
					}
					//v2=""
					if(!xmlStrcasecmp(cur_attr->name,"v2")){
						v2 = atof(cur_attr->children->content);
						continue;
					}
					//id=""
					if(!xmlStrcasecmp(cur_attr->name,"tid")){
						tid = atoi(cur_attr->children->content);
						continue;
					}
					break;

				case CHECKBOX:
					//checked=""
					if(!xmlStrcasecmp(cur_attr->name,"checked")){
						checked = atoi(cur_attr->children->content);
						continue;
					}
					break;

				case PROGRESSBAR:
					//progress=""
					if(!xmlStrcasecmp(cur_attr->name,"progress")){
						progress = atof(cur_attr->children->content);
						continue;
					}
					break;

				case VSCROLLBAR:
					//pos=""
					if(!xmlStrcasecmp(cur_attr->name,"pos")){
						pos = atoi(cur_attr->children->content);
						continue;
					}
					//pos_inc=""
					if(!xmlStrcasecmp(cur_attr->name,"pos_inc")){
						pos_inc = atoi(cur_attr->children->content);
						continue;
					}
					break;

			}
		}
	}

	switch(type){
		case LABEL:
			label_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, text);
			break;
		case IMAGE:
			image_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, tid, u1, v1, u2, v2);
			break;
		case CHECKBOX:
			checkbox_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, checked);
			break;
		case BUTTON:
			button_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, text);
			break;
		case PROGRESSBAR:
			progressbar_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, progress);
			break;
		case VSCROLLBAR:
			vscrollbar_add_extended(winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, r, g, b, pos, pos_inc);
			break;

	}
	return 1;
}

int GetWidgetType(char *w)
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

	return 0;
}

