#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include <SDL_keysym.h>
#include "widgets.h"
#include "asc.h"
#include "chat.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
#include "global.h"
#include "interface.h"
#include "misc.h"
#include "multiplayer.h"
#include "paste.h"
#include "tabs.h"
#include "textures.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "sound.h"

static size_t cm_edit_id = CM_INIT_VALUE;

typedef struct {
	char text[256];
}label;

typedef struct {
	float u1,v1,u2,v2, alpha;
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
	char * password;
	int status;
	int max_chars;
}password_entry;

typedef struct {
	char text[256];
	Uint16 x;
	Uint16 y;
	int width;
	int value;
}multiselect_button;

typedef struct {
	int nr_buttons;
	int selected_button;
	int max_buttons;
	int next_value;
	multiselect_button *buttons;
	/* Scrollbar related vars */
	Uint16 max_height;
	Uint16 actual_height;
	Uint32 scrollbar;
	int win_id;
	float highlighted_red;
	float highlighted_green;
	float highlighted_blue;
}multiselect;

Uint32 widget_id = 0x0000FFFF;
static const int multiselect_button_height = 22;

int ReadXMLWindow(xmlNode * a_node);
int ParseWindow (xmlNode *node);
int ParseWidget (xmlNode *node, int winid);
int ParseTab (xmlNode *node, int winid, int colid);
int GetWidgetType (const char *w);
int disable_double_click = 0;

// <--- Common widget functions ---
widget_list * widget_find(int window_id, Uint32 widget_id)
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

int widget_destroy (int window_id, Uint32 widget_id)
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
		{
			if (n->spec != NULL)
				n->OnDestroy (n, n->spec);
			else
				n->OnDestroy (n);
		}
		if (n->type != NULL)
			if (n->type->destroy != NULL)
				n->type->destroy (n);
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
				// unlink n first, otherwise recursive calls
				// to widget_destroy (e.g. widgets destroying
				// their scrollbar) may set a pointer to the
				// widget being deleted here.
				w->next = n->next;
				
				if (n->OnDestroy != NULL)
				{
					if(n->spec != NULL)
						n->OnDestroy (n, n->spec);
					else
						n->OnDestroy (n);
				}
				if (n->type != NULL)
					if (n->type->destroy != NULL)
						n->type->destroy (n);
				free (n);
				return 1;
			}
		}
	}
	
	// shouldn't get here	
	return 0;
}

int widget_set_OnDraw(int window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnDraw = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnClick(int window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnClick = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnDrag(int window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnDrag = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnMouseover(int window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->OnMouseover = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnKey ( int window_id, Uint32 widget_id, int (*handler)() )
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		w->OnKey = handler;
		return 1;
	}
	return 0;
}

int widget_set_OnDestroy (int window_id, Uint32 widget_id, int (*handler)())
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w)
	{
		w->OnDestroy = handler;
		return 1;
	}
	return 0;
}

int widget_move(int window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->pos_x = x;
		w->pos_y = y;
		return 1;
	}
	return 0;
}

Uint32 widget_move_win(int window_id, Uint32 widget_id, int new_win_id)
{
	widget_list *w;
	widget_list *prev_w = NULL;

	if (window_id < 0 || window_id >= windows_list.num_windows 
		|| new_win_id < 0 || new_win_id >= windows_list.num_windows) {
		return 0;
	}
	if (windows_list.window[window_id].window_id != window_id 
		|| windows_list.window[new_win_id].window_id != new_win_id 
		|| window_id == new_win_id) {
		return 0;
	}

	w = windows_list.window[window_id].widgetlist;
	/* Find the widget */
	while(w != NULL) {
		if(w->id == widget_id) {
			break;
		} else {
			prev_w = w;
			w = w->next;
		}
	}
	if(w != NULL) {
		Uint32 new_id = 0;
		widget_list *target_w = windows_list.window[new_win_id].widgetlist;
		/* Remove the widget from the old list */
		if(prev_w) {
			prev_w->next = w->next;
		} else {
			windows_list.window[window_id].widgetlist = NULL;
		}
		/* Put it in the new list */
		if(target_w == NULL) {
			/* List is empty, start a new one */
			windows_list.window[new_win_id].widgetlist = w;
		} else {
			/* Find the end of the list */
			while(target_w->next != NULL) {
				if(target_w->id > new_id) {
					new_id = target_w->id;
				}
				target_w = target_w->next;
			}
			target_w->next = w;
		}
		w->next = NULL;
		w->window_id = new_win_id;
		if(new_id > 0xFFFF) {
			new_id = widget_id++;
		} else {
			new_id++;
		}
		w->id = new_id;
		/* if removing a (possibly none existant) context menu for the widget works, we need to replace it */
		if (cm_remove_widget(window_id, widget_id))
			cm_add_widget(cm_edit_id, new_win_id, new_id);
		return w->id;
	} else {
		return 0;
	}
}

int widget_move_rel (int window_id, Uint32 widget_id, Sint16 dx, Sint16 dy)
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

int widget_resize(int window_id, Uint32 widget_id, Uint16 x, Uint16 y)
{
	widget_list *w = widget_find (window_id, widget_id);
	int res = 0;
	
	if (w)
	{
		w->len_x = x;
		w->len_y = y;
		if (w->type != NULL)
			if (w->type->resize != NULL) 
				res = w->type->resize (w, x, y);
		if (w->OnResize && res != -1)
		{
			if(w->spec != NULL) res |= w->OnResize (w, x, y, w->spec);
			else res |= w->OnResize (w, x, y);
		}
		
		return res > -1 ? res : 0;
	}
	return 0;
}

int widget_set_flags(int window_id, Uint32 widget_id, Uint32 f)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->Flags = f;
		return 1;
	}
	return 0;
}

int widget_unset_flags (int window_id, Uint32 widget_id, Uint32 f)
{
	widget_list *w = widget_find(window_id, widget_id);
	if (w)
	{
		w->Flags &= ~f;
		return 1;
	}
	return 0;
}

int widget_set_size(int window_id, Uint32 widget_id, float size)
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		w->size = size;
		return 1;
	}
	return 0;
}

int widget_set_color(int window_id, Uint32 widget_id, float r, float g, float b)
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

int widget_get_width (int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if (w != NULL)
	{
		return w->len_x;
	}
	return -1;
}

int widget_get_height (int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if (w != NULL)
	{
		return w->len_y;
	}
	return -1;
}

int free_widget_info (widget_list *widget)
{
	free (widget->widget_info);
	return 1;
}

int widget_set_type (int window_id, Uint32 widget_id, const struct WIDGET_TYPE *type)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->type = type;
		return 1;
	}
	return 0;
}

int widget_set_args (int window_id, Uint32 widget_id, void *spec)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		w->spec = spec;
		return 1;
	}
	return 0;
}

// Create a generic widget
Uint32 widget_add (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly,
	Uint32 Flags, float size, float r, float g, float b, const struct WIDGET_TYPE *type, void *T, void *S)
{
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	widget_list *w = windows_list.window[window_id].widgetlist;
	
	// Clearing everything
	memset(W,0,sizeof(widget_list));

	// Filling the widget info
	W->widget_info = T;
	
	// Copy the information
	W->id = wid;
	W->window_id = window_id;
	W->Flags = Flags;
	W->pos_x = x;
	W->pos_y = y;
	W->size = size;
	W->r = r;
	W->g = g;
	W->b = b;
	W->len_y = ly;
	W->len_x = lx;
	
	W->spec = S;
	
	// Generic Handling Functions
	W->OnInit = OnInit;
	W->type = type;
	
	// Check if we need to initialize it
	if(W->type != NULL)	
		if(W->type->init != NULL)
			W->type->init(W);
	if(W->OnInit != NULL)
	{
		if(W->spec != NULL) W->OnInit (W, W->spec);
		else W->OnInit (W);
	}
	
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


int widget_handle_mouseover (widget_list *widget, int mx, int my)
{
	int res = 0;

	if (widget->type != NULL) {
		if (widget->type->mouseover != NULL) {
			res = widget->type->mouseover (widget, mx, my);
		}
	}

	if (widget->OnMouseover != NULL && res != -1) {
		if(widget->spec != NULL) {
			res |= widget->OnMouseover (widget, mx, my, widget->spec);
		} else {
			res |= widget->OnMouseover (widget, mx, my);
		}
	}

	return res > -1 ? res : 0;
}

int widget_handle_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int res = 0;
	/* widget might get destroyed by handler so check for sound type now */
	int play_sound = (widget->type == &round_button_type) ?1 :0;

	if (widget->type != NULL) {
		if (widget->type->click != NULL) {
			res = widget->type->click (widget, mx, my, flags);
		}
	}

	if (widget->OnClick != NULL && res != -1)
	{
		if(widget->spec != NULL) {
			res |= widget->OnClick (widget, mx, my, flags,  widget->spec);
		} else {
			res |= widget->OnClick (widget, mx, my, flags);
		}
	}

	if (play_sound && res > -1)
		do_click_sound();

	return res > -1 ? res : 0;
}

int widget_handle_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int res = 0;

	if (widget->type != NULL && widget->type->drag != NULL) {
		res = widget->type->drag (widget, mx, my, flags, dx, dy);
	}

	if (widget->OnDrag != NULL && res != -1)
	{
		if(widget->spec != NULL) {
			res |= widget->OnDrag (widget, mx, my, flags, dx, dy, widget->spec);
		} else {
			res |= widget->OnDrag (widget, mx, my, flags, dx, dy);
		}
	}
	return res > -1 ? res : 0;
}

int widget_handle_keypress (widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey)
{
	int res = 0;

	if (widget->type != NULL) {
		if (widget->type->key != NULL) {
			res = widget->type->key (widget, mx, my, key, unikey);
		}
	}

	if (widget->OnKey != NULL && res != -1)
	{
		if(widget->spec != NULL) {
			res |= widget->OnKey (widget, mx, my, key, unikey, widget->spec);
		} else {
			res |= widget->OnKey (widget, mx, my, key, unikey);
		}
	}
	return res > -1 ? res : 0;
}

// --- End Common Widget Functions --->

// Label
const struct WIDGET_TYPE label_type = { NULL, label_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info };

int label_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint32 Flags, float size, float r, float g, float b, const char *text)
{
	Uint16 len_x = (Uint16)(strlen (text) * 11 * 1.0);
	Uint16 len_y = (Uint16)(18 * 1.0);

	label *T = (label *) calloc (1, sizeof(label));
	safe_snprintf (T->text, sizeof(T->text), "%s", text);

	return widget_add (window_id, wid, OnInit, x, y, len_x, len_y, Flags, size, r, g, b, &label_type, (void *)T, NULL);
}

int label_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return label_add_extended (window_id, widget_id++, OnInit, x, y, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int label_draw(widget_list *W)
{
	label *l = (label *)W->widget_info;
	if(W->r != -1.0) {
		glColor3f(W->r,W->g,W->b);
	}
	draw_string_zoomed(W->pos_x,W->pos_y,(unsigned char *)l->text,1,W->size);
	return 1;
}

int label_set_text(int window_id, Uint32 widget_id, const char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		label *l = (label *) w->widget_info;
		safe_snprintf(l->text, sizeof(l->text), "%s", text);
		return 1;
	}
	return 0;
}

// Image

const struct WIDGET_TYPE image_type = { NULL, image_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info };

int image_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int id, float u1, float v1, float u2, float v2, float alpha)
{
	image *T = calloc (1, sizeof (image));
	T->u1 = u1;
	T->u2 = u2;
#ifdef	NEW_TEXTURES
	T->v1 = -v1;
	T->v2 = -v2;
#else	/* NEW_TEXTURES */
	T->v1 = v1;
	T->v2 = v2;
#endif	/* NEW_TEXTURES */
	T->id = id;
	T->alpha = alpha;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &image_type, T, NULL);
}

int image_add(int window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2)
{
	return image_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 1.0, 1.0, 1.0, id, u1, v1, u2, v2, -1); 
}

int image_draw(widget_list *W)
{
	image *i = (image *)W->widget_info;
#ifdef	NEW_TEXTURES
	bind_texture(i->id);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(i->id);
#endif	/* NEW_TEXTURES */
	glColor3f(W->r, W->g, W->b);
	if (i->alpha > -1) {
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, i->alpha);
	}
	glBegin(GL_QUADS);
	draw_2d_thing(i->u1, i->v1, i->u2, i->v2, W->pos_x, W->pos_y, W->pos_x + (W->len_x * W->size), W->pos_y + (W->len_y * W->size));
	glEnd();
	if (i->alpha > -1) {
		glDisable(GL_ALPHA_TEST);
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int image_set_id(int window_id, Uint32 widget_id, int id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		image *l = (image *) w->widget_info;
		l->id = id;
		return 1;
	}
	return 0;
}

int image_set_uv(int window_id, Uint32 widget_id, float u1, float v1, float u2, float v2)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		image *l = (image *) w->widget_info;
		l->u1 = u1;
		l->u2 = u2;
#ifdef	NEW_TEXTURES
		l->v1 = -v1;
		l->v2 = -v2;
#else	/* NEW_TEXTURES */
		l->v1 = v1;
		l->v2 = v2;
#endif	/* NEW_TEXTURES */
		return 1;
	}
	return 0;
}


// Checkbox
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int checkbox_click (widget_list *W, int mx, int my, Uint32 flags)
{
	checkbox *c = (checkbox *)W->widget_info;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	*c->checked = !*c->checked;

	do_click_sound();

	return 1;
}

int checkbox_get_checked(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		return *c->checked;
	}
	return -1;
}

int checkbox_set_checked(int window_id, Uint32 widget_id, int checked)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		checkbox *c = (checkbox *)w->widget_info;
		*c->checked = checked;
		return 1;
	}
	return 0;
}

const struct WIDGET_TYPE checkbox_type = { NULL, checkbox_draw, checkbox_click, NULL, NULL, NULL, NULL, free_widget_info };

int checkbox_add_extended(int window_id,  Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int *checked)
{
	checkbox *T = calloc (1, sizeof (checkbox));
	T->checked = checked;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &checkbox_type, T, NULL);
}

int checkbox_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int *checked)
{
	if(checked == NULL)
	{
		checked = calloc(1,sizeof(*checked));
	}
	return checkbox_add_extended(window_id, widget_id++, NULL, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, checked);
}

// Button
const struct WIDGET_TYPE round_button_type = { NULL, button_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info };
const struct WIDGET_TYPE square_button_type = { NULL, square_button_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info };

int safe_button_click(Uint32 *last_click)
{
	int retvalue = 0;
	if (disable_double_click || ((SDL_GetTicks() - *last_click) < 500))
		retvalue = 1;
	*last_click = SDL_GetTicks();
	return retvalue;
}

int button_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, const char *text)
{
	Uint16 len_x = lx > 0 ? lx : (Uint16)(strlen(text) * 11 * size) + 2*BUTTONRADIUS*size;
	Uint16 len_y = ly > 0 ? ly : (Uint16)(18 * size) + 12*size;

	button *T = calloc (1, sizeof(button));
	safe_snprintf (T->text, sizeof(T->text), "%s", text);

	return widget_add (window_id, wid, OnInit, x, y, len_x, len_y, Flags, size, r, g, b, &round_button_type, T, NULL);
}

int button_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return button_add_extended(window_id, widget_id++, NULL, x, y, 0, 0, 0, 1.0, -1.0, -1.0, -1.0, text);
}

int button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
// 0ctane: I suspect the below section was a reminant from the original button drawing routine below
/*	float extra_space = (W->len_x - get_string_width((unsigned char*)l->text)*W->size)/2.0f;
	if(extra_space < 0) {
		extra_space = 0;
	}*/

#ifdef NEW_NEW_CHAR_WINDOW
	draw_smooth_button(l->text, W->size, W->pos_x, W->pos_y, W->len_x-2*BUTTONRADIUS*W->size, 1, W->r, W->g, W->b, W->Flags & BUTTON_ACTIVE, 0.32f, 0.23f, 0.15f, 0.0f);
#else
	draw_smooth_button(l->text, W->size, W->pos_x, W->pos_y, W->len_x-2*BUTTONRADIUS*W->size, 1, W->r, W->g, W->b, 0, 0.0f, 0.0f, 0.0f, 0.0f);
#endif
	
	return 1;
}

int square_button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
	float extra_space = (W->len_x - get_string_width((unsigned char*)l->text)*W->size*(0.11f/0.12f))/2.0f;
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
	draw_string_zoomed(W->pos_x + 2 + extra_space + gx_adjust, W->pos_y + 2 + gy_adjust, (unsigned char *)l->text, 1, W->size);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int button_set_text(int window_id, Uint32 widget_id, char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		button *l = (button *) w->widget_info;
		safe_snprintf(l->text, sizeof(l->text), "%s",  text);
		return 1;
	}
	return 0;
}

// Progressbar
const struct WIDGET_TYPE progressbar_type = { NULL, progressbar_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info };

int progressbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return progressbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0.0f, NULL);
}

int progressbar_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress, const float * colors)
{
	progressbar *T = calloc (1, sizeof(progressbar));
	T->progress = progress;
	if (colors) {
		memcpy(T->colors, colors, sizeof(T->colors));
	} else {
		T->colors[0] = -1.0f;
	}

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &progressbar_type, T, NULL);
}

int progressbar_draw(widget_list *W)
{
	progressbar *b = (progressbar *)W->widget_info;
	int pixels = (b->progress/100) * W->len_x;

	glDisable(GL_TEXTURE_2D);

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
			glVertex3i(W->pos_x, W->pos_y + 0, 0);//LabRat: fix unfilled pixels in progress bar
			if (have_bar_colors) glColor3fv(&right_colors[0]);
			glVertex3i(W->pos_x + pixels,W->pos_y + 0,0);
			if (have_bar_colors) glColor3fv(&right_colors[3]);
			glVertex3i(W->pos_x + pixels, W->pos_y + W->len_y, 0);
			if (have_bar_colors) glColor3fv(&b->colors[9]);
			glVertex3i(W->pos_x, W->pos_y + W->len_y, 0);//LabRat: fix unfilled pixels in progress bar
			glColor3f(0.77f,0.57f,0.39f);
		glEnd();
	}
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);
	else
		glColor3f(0.77f,0.57f,0.39f);

	//LabRat: Draw bounding box after progress bar
	glBegin(GL_LINE_LOOP);
	glVertex3i(W->pos_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y,0);
	glVertex3i(W->pos_x + W->len_x,W->pos_y + W->len_y,0);
	glVertex3i(W->pos_x,W->pos_y + W->len_y,0);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}

float progressbar_get_progress(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		progressbar *c = (progressbar *)w->widget_info;
		return c->progress;
	}
	return -1;
}

int progressbar_set_progress(int window_id, Uint32 widget_id, float progress)
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
int vscrollbar_draw(widget_list *W)
{
	int drawn_bar_len = 0;
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
	glVertex3i(W->pos_x + 5 + gx_adjust, W->pos_y + 10 + gy_adjust,0);
	glVertex3i(W->pos_x + 10 + gx_adjust, W->pos_y + 5 + gy_adjust,0);
	glVertex3i(W->pos_x + 10 + gx_adjust, W->pos_y + 5 + gy_adjust,0);
	glVertex3i(W->pos_x + 15 + gx_adjust, W->pos_y + 10 + gy_adjust,0);
	glVertex3i(W->pos_x + 5 + gx_adjust, W->pos_y + W->len_y - 10 + gy_adjust,0);
	glVertex3i(W->pos_x + 10 + gx_adjust, W->pos_y + W->len_y - 5 + gy_adjust,0);
	glVertex3i(W->pos_x + 10 + gx_adjust, W->pos_y + W->len_y - 5 + gy_adjust,0);
	glVertex3i(W->pos_x + 15 + gx_adjust, W->pos_y + W->len_y - 10 + gy_adjust,0);
	glEnd();

	if (c->bar_len > 0)
		drawn_bar_len = c->bar_len;
	else
	{
		drawn_bar_len = 1;
		if(W->r!=-1.0)
			glColor3f(W->r/3, W->g/3, W->b/3);
	}
	glBegin(GL_QUADS);
	glVertex3i(W->pos_x + 7 + gx_adjust, W->pos_y + 15 + (c->pos*((float)(W->len_y-50)/drawn_bar_len)) + gy_adjust, 0);
	glVertex3i(W->pos_x + W->len_x - 7 + gx_adjust, W->pos_y +  15 + (c->pos*((float)(W->len_y-50)/drawn_bar_len)) + gy_adjust, 0);
	glVertex3i(W->pos_x + W->len_x - 7 + gx_adjust, W->pos_y + 35 + (c->pos*((float)(W->len_y-50)/drawn_bar_len)) + gy_adjust, 0);
	glVertex3i(W->pos_x + 7 + gx_adjust, W->pos_y + 35 + (c->pos*((float)(W->len_y-50)/drawn_bar_len)) + gy_adjust, 0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}

int vscrollbar_click (widget_list *W, int mx, int my, Uint32 flags)
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

int vscrollbar_set_pos_inc(int window_id, Uint32 widget_id, int pos_inc)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		c->pos_inc = pos_inc;
		return 1;
	}

	return 0;
}

int vscrollbar_set_pos(int window_id, Uint32 widget_id, int pos)
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

int vscrollbar_scroll_up(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *scrollbar = w->widget_info;
		return vscrollbar_set_pos(window_id, widget_id, scrollbar->pos - scrollbar->pos_inc);
	}
	return 0;
}

int vscrollbar_scroll_down(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *scrollbar = w->widget_info;
		return vscrollbar_set_pos(window_id, widget_id, scrollbar->pos + scrollbar->pos_inc);
	}
	return 0;
}

int vscrollbar_set_bar_len (int window_id, Uint32 widget_id, int bar_len)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		c->bar_len = bar_len >= 0 ? bar_len : 1;
		if (c->pos > c->bar_len)
			c->pos = c->bar_len;
		return 1;
	}

	return 0;
}

int vscrollbar_drag(widget_list *W, int x, int y, Uint32 flags, int dx, int dy)
{
	window_info *win = (W==NULL) ? NULL : &windows_list.window[W->window_id];
	vscrollbar_click(W, x, y, flags);
	
	/* the drag event can happen multiple times for each redraw as its done in the event loop
	 * so update the scroll bar position each time to avoid positions glitches */
	if(win!=NULL && win->flags&ELW_SCROLLABLE && win->scroll_id==W->id)
	{
		int pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
		int offset = win->scroll_yoffset + ((win->flags&ELW_CLOSE_BOX) ? ELW_BOX_SIZE : 0);
		widget_move(win->window_id, win->scroll_id, win->len_x-ELW_BOX_SIZE, pos+offset);
	}
	
	return 1;
}

int vscrollbar_get_pos(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		vscrollbar *c = (vscrollbar *)w->widget_info;
		return c->pos;
	}
	return -1;
}

const struct WIDGET_TYPE vscrollbar_type = { NULL, vscrollbar_draw, vscrollbar_click, vscrollbar_drag, NULL, NULL, NULL, free_widget_info };

int vscrollbar_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc, int bar_len)
{
	vscrollbar *T = calloc (1, sizeof(vscrollbar));
	T->pos_inc = pos_inc;
	T->pos = pos;
	T->bar_len = bar_len > 0 ? bar_len : 0;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &vscrollbar_type, T, NULL);
}

int vscrollbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return vscrollbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, 1, ly);
}

// Tab collection
int tab_collection_get_tab (int window_id, Uint32 widget_id) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
	{
		tab_collection *col = (tab_collection *) w->widget_info;
		return col->cur_tab;
	}
	return -1;
}

int tab_collection_get_tab_id (int window_id, Uint32 widget_id) 
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

int tab_collection_get_tab_nr (int window_id, Uint32 col_id, int tab_id) 
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

int tab_collection_get_nr_tabs (int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w)
	{
		tab_collection *t = (tab_collection *) w->widget_info;
		return t->nr_tabs;
	}
	return -1;
}

int tab_set_label_color_by_id (int window_id, Uint32 col_id, int tab_id, float r, float g, float b)
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

int tab_collection_select_tab (int window_id, Uint32 widget_id, int tab) 
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

int _tab_collection_close_tab_real (tab_collection* col, int tab)
{
	if (col != NULL && tab >= 0 && tab < col->nr_tabs)
	{
		int i;

		destroy_window (col->tabs[tab].content_id);
		for (i = tab+1; i < col->nr_tabs; i++)
			col->tabs[i-1] = col->tabs[i];

		col->nr_tabs--;
		if (tab < col->cur_tab || (tab == col->cur_tab && tab >= col->nr_tabs))
			col->cur_tab--;
		if (tab < col->tab_offset || (tab == col->tab_offset && tab >= col->nr_tabs))
			col->tab_offset--;
			
		return col->cur_tab;
	}

	return -1;
}

int tab_collection_close_tab (int window_id, Uint32 widget_id, int tab) 
{
	widget_list *w = widget_find (window_id, widget_id);
	if (w) 
		return _tab_collection_close_tab_real (w->widget_info, tab);
	return -1;
}

int free_tab_collection (widget_list *widget)
{
	tab_collection *col = (tab_collection *) widget->widget_info;
	free (col->tabs);
	free (col);
	return 1;
}

int tab_collection_draw (widget_list *w)
{
	tab_collection *col;
	int itab, ytagtop, ytagbot, xstart, xend, xmax;
	int btn_size, arw_width;
	int cur_start, cur_end;
	int h;

	if (!w) return 0;
	
	col = (tab_collection *) w->widget_info;
	
	h = col->tag_height;
	ytagtop = w->pos_y;
	ytagbot = w->pos_y + h;
	xstart = w->pos_x;
	btn_size = col->button_size;
	arw_width = btn_size / 4;
	cur_start = cur_end = xstart;

	glDisable(GL_TEXTURE_2D);

	if (col->tab_offset > 0)
	{
		// draw a "move left" button
		if(w->r!=-1.0)
			glColor3f(w->r, w->g, w->b);

		// button outline
		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart, ytagtop, 0);
			glVertex3i (xstart, ytagtop+btn_size, 0);
			glVertex3i (xstart+btn_size, ytagtop+btn_size, 0);
			glVertex3i (xstart+btn_size, ytagtop, 0);
			glVertex3i (xstart, ytagtop, 0);
		glEnd ();

		// left arrows
		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2-arw_width, 0);
			glVertex3i (xstart+arw_width, ytagtop+btn_size/2, 0);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2+arw_width, 0);
		glEnd ();

		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart+3*arw_width, ytagtop+btn_size/2-arw_width, 0);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2, 0);
			glVertex3i (xstart+3*arw_width, ytagtop+btn_size/2+arw_width, 0);
		glEnd ();

		xstart += h;
	}

	// draw the tags
	for (itab = col->tab_offset; itab < col->nr_tabs; itab++) 
	{
		xend = xstart + col->tabs[itab].tag_width;
		xmax = w->pos_x + w->len_x;
		if (itab < col->nr_tabs - 1)
			xmax -= h;

		// Check if there's still room for this tab, but always 
		// draw at least one tab
		if (itab > col->tab_offset && xend > xmax)
		{
			// this tab doesn't fit. Simply extend the top line to 
			// the end of the available width
			glBegin (GL_LINES);
				glVertex3i (xstart, ytagtop+1, 0);
				glVertex3i (w->pos_x + w->len_x - h, ytagtop+1, 0);
			glEnd ();
			break;
		}

		if (itab == col->cur_tab)
		{
			cur_start = xstart;
			cur_end = xend;
		}

		if(w->r!=-1.0)
			glColor3f(w->r, w->g, w->b);

		if(col->cur_tab == itab){
			glBegin(GL_LINE_STRIP);
				glVertex3i(xstart, ytagbot, 0);
				draw_circle_ext(xstart, ytagtop, DEFAULT_TAB_RADIUS, -10, 180, 90);
				draw_circle_ext(xend-2*DEFAULT_TAB_RADIUS+1, ytagtop, DEFAULT_TAB_RADIUS, -10, 89, 0);
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
				draw_circle_ext(xend-2*DEFAULT_TAB_RADIUS+1, ytagtop, DEFAULT_TAB_RADIUS, -10, 89, 0);
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
			draw_string_zoomed (xstart+h+gx_adjust, ytagbot-SMALL_FONT_Y_LEN-2+gy_adjust, (unsigned char *)col->tabs[itab].label, 1, w->size);
		else
			draw_string_zoomed (xstart+4+gx_adjust, ytagbot-SMALL_FONT_Y_LEN-2+gy_adjust, (unsigned char *)col->tabs[itab].label, 1, w->size);
		glDisable(GL_TEXTURE_2D);

		xstart = xend;
	}

	col->tab_last_visible = itab-1;
	if (itab < col->nr_tabs)
	{
		// draw a "move right" button
		xstart = w->pos_x + w->len_x - btn_size;

		if(w->r!=-1.0)
			glColor3f(w->r, w->g, w->b);

		// button outline
		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart, ytagtop, 0);
			glVertex3i (xstart, ytagtop+btn_size, 0);
			glVertex3i (xstart+btn_size, ytagtop+btn_size, 0);
			glVertex3i (xstart+btn_size, ytagtop, 0);
			glVertex3i (xstart, ytagtop, 0);
		glEnd ();

		// right arrows
		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart+arw_width, ytagtop+btn_size/2-arw_width, 0);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2, 0);
			glVertex3i (xstart+arw_width, ytagtop+btn_size/2+arw_width, 0);
		glEnd ();

		glBegin (GL_LINE_STRIP);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2-arw_width, 0);
			glVertex3i (xstart+3*arw_width, ytagtop+btn_size/2, 0);
			glVertex3i (xstart+2*arw_width, ytagtop+btn_size/2+arw_width, 0);
		glEnd ();
	}

	if(w->r!=-1.0)
		glColor3f(w->r, w->g, w->b);
	
	// draw the rest of the frame around the tab
	glBegin (GL_LINE_STRIP);
	glVertex3i (cur_end, ytagbot, 0);
	glVertex3i (w->pos_x + w->len_x, ytagbot, 0);		
	glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);		
	glVertex3i (w->pos_x, ytagbot, 0);		
	glVertex3i (cur_start, ytagbot, 0);
	glEnd ();

	glEnable(GL_TEXTURE_2D);
	
	// show the content of the current tab
	if (col->nr_tabs > 0)
		show_window (col->tabs[col->cur_tab].content_id);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	
	return 1;
}

int tab_collection_click (widget_list *W, int x, int y, Uint32 flags)
{
	tab_collection *col = (tab_collection *) W->widget_info;
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;
	
	// Check if we clicked a tab scroll button
	if (col->tab_offset > 0 && x >= 0 && x <= col->button_size && y >= 0 && y <= col->button_size)
	{
		col->tab_offset--;
		return 1;
	}
	
	if (col->tab_last_visible < col->nr_tabs-1 && x >= W->len_x - col->button_size && x <= W->len_x && y >= 0 && y <= col->button_size)
	{
		if (col->tab_offset < col->nr_tabs-1)
			col->tab_offset++;
		return 1;
	}

	if (y < col->tag_height) 
	{
		int x_start = col->tab_offset > 0 ? col->tag_height : 0;
		int itag, ctag = col->cur_tab;

		// find which tag was clicked
		for (itag = col->tab_offset; itag <= col->tab_last_visible; itag++)
		{
			int x_end = x_start + col->tabs[itag].tag_width;
			if (x >= x_start && x < x_end)
				break;
			x_start = x_end;
		}

		if (itag <= col->tab_last_visible)
		{
			// check if close box was clicked
			if (col->tabs[itag].closable && x > x_start + 3 && x < x_start + col->tag_height - 3 && y > 3 && y < col->tag_height - 3)
			{
				do_click_sound();
				_tab_collection_close_tab_real (col, itag);
			}
			// check if a new tab is selected
			else if (itag != ctag)
			{
				col->cur_tab = itag;
				hide_window (col->tabs[ctag].content_id);
				show_window (col->tabs[itag].content_id);
				//select_window (col->tabs[itag].content_id);
				do_click_sound();
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

void _tab_collection_make_cur_visible (widget_list *W)
{
	tab_collection *col;

	if (W == NULL)
		return;

	col = W->widget_info;
	if (col == NULL)
		return;

	if (col->cur_tab < col->tab_offset)
	{
		col->tab_offset = col->cur_tab;
	}
	else if (col->cur_tab > col->tab_last_visible)
	{
		// find a tab offset such that the selected tab is the
		// rightmost visible tab
		int max_width = W->len_x - col->tag_height;
		if (col->cur_tab < col->nr_tabs-1)
			max_width -= col->tag_height;
		
		while (col->tab_offset < col->cur_tab)
		{
			int w = 0, i;

			col->tab_offset++;
			for (i = col->tab_offset; i <= col->cur_tab; i++)
				w+= col->tabs[i].tag_width;
			
			if (w < max_width)
				break;
		}
	}
}

int tab_collection_keypress (widget_list *W, int mx, int my, Uint32 key, Uint32 unikey)
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
		_tab_collection_make_cur_visible (W);
		return 1;
	}
	else if (shift_on && keysym == K_ROTATELEFT)
	{
		if (col->nr_tabs == 1) return 1;
		hide_window (col->tabs[col->cur_tab].content_id);
		if (--col->cur_tab < 0)
			col->cur_tab = col->nr_tabs - 1;
		_tab_collection_make_cur_visible (W);
		return 1;
	}

	return 0;	
}

const struct WIDGET_TYPE tab_collection_type = { 
	NULL, 
	tab_collection_draw, 
	tab_collection_click, 
	NULL, 
	NULL, 
	tab_collection_resize, 
	tab_collection_keypress, 
	free_tab_collection
};

int tab_collection_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int max_tabs, Uint16 tag_height)
{
	int itab;
	tab_collection *T = calloc (1, sizeof (tab_collection));
	T->max_tabs =  max_tabs <= 0 ? 2 : max_tabs;
	T->tabs = calloc (T->max_tabs, sizeof (tab));
	// initialize all tabs content ids to -1 (unitialized window_
	for (itab = 0; itab < T->max_tabs; itab++)
		T->tabs[itab].content_id = -1;
	T->nr_tabs = 0;
	T->tag_height = tag_height;
	T->button_size = (9 * tag_height) / 10;
	T->cur_tab = 0;
	T->tab_offset = 0;
	T->tab_last_visible = 0;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &tab_collection_type, T, NULL);
}

int tab_collection_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint16 tag_height)
{
	return tab_collection_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, -1.0, -1.0, -1.0, 0, tag_height);
}

int tab_add (int window_id, Uint32 col_id, const char *label, Uint16 tag_width, int closable, Uint32 flags)
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
		
	my_strncp ((char*)col->tabs[nr].label, label, sizeof (col->tabs[nr].label));
	col->tabs[nr].content_id = create_window ("", window_id, 0, w->pos_x, w->pos_y + col->tag_height, w->len_x, w->len_y - col->tag_height, ELW_TITLE_NONE|flags);
	col->tabs[nr].closable = closable ? 1 : 0;

	if (tag_width > 0)
	{
		col->tabs[nr].tag_width = tag_width;
	}
	else
	{
		// compute tag width from label width
		col->tabs[nr].tag_width = 10 + ((float)w->size * (DEFAULT_FONT_X_LEN * (float)get_string_width((unsigned char*)col->tabs[nr].label)/12.0f));
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
void _text_field_set_nr_visible_lines (widget_list *w)
{
	text_field* tf = w->widget_info;

	if (tf != NULL/* && (w->Flags & TEXT_FIELD_EDITABLE)*/)
	{
		float displayed_font_y_size = floor (DEFAULT_FONT_Y_LEN * tf->buffer[tf->msg].wrap_zoom);
		tf->nr_visible_lines = (int) ((w->len_y - 2*tf->y_space) / displayed_font_y_size);
		if (tf->nr_visible_lines < 0)
			tf->nr_visible_lines = 0;
	}
}

void _text_field_set_nr_lines (widget_list *w, int nr_lines)
{
	text_field* tf = w->widget_info;
	if (tf != NULL)
	{
		tf->nr_lines = nr_lines;
		if (tf->scroll_id != -1)
		{
			int bar_len = nr_lines >= tf->nr_visible_lines ? nr_lines - tf->nr_visible_lines : 0;
			vscrollbar_set_bar_len (w->window_id, tf->scroll_id, bar_len);
			tf->update_bar = 0;
		}
	}
}

int skip_message (const text_message *msg, Uint8 filter)
{
	int skip = 0;
	int channel = msg->chan_idx;
	if (filter == FILTER_ALL) return 0;
	if (channel != filter)
	{
		switch (channel)
		{
			case CHAT_LOCAL:    skip = local_chat_separate;    break;
			case CHAT_PERSONAL: skip = personal_chat_separate; break;
			case CHAT_GM:       skip = guild_chat_separate;    break;
			case CHAT_SERVER:   skip = server_chat_separate;   break;
			case CHAT_MOD:      skip = mod_chat_separate;      break;
			case CHAT_MODPM:    skip = 0;                      break;
			default:            skip = 1;
		}
	}
	switch (channel) {
		case CHAT_CHANNEL1:
		case CHAT_CHANNEL2:
		case CHAT_CHANNEL3:
			skip = (msg->channel != active_channels[filter - CHAT_CHANNEL1]);
	}
	return skip;
}

void text_field_find_cursor_line(text_field* tf)
{
	int i, line = 0;
	const text_message* msg = &tf->buffer[tf->msg];
	for (i = 0; i < msg->len; i++)
	{
		if (i == tf->cursor) tf->cursor_line = line;
		if (msg->data[i] == '\n' || msg->data[i] == '\r') line++;
	}
	tf->nr_lines = line + 1; // we'll call _text_field_set_nr_lines later;
	if (tf->cursor >= msg->len) tf->cursor_line = line;
	tf->update_bar = 1;
}

char* text_field_get_selected_text (const text_field* tf)
{
	int sm, sc, em, ec;
	const select_info* select;
	int len, max_len;
	char* text = NULL;

	max_len = 0;
	len = 0;
	select = &tf->select;
	if (TEXT_FIELD_SELECTION_EMPTY(select)) return NULL;

	if ((select->em > select->sm) || ((select->em == select->sm) && (select->ec >= select->sc)))
	{
		sm = select->sm;
		sc = select->sc;
		em = select->em;
		ec = select->ec;
	}
	else
	{
		sm = select->em;
		sc = select->ec;
		em = select->sm;
		ec = select->sc;
	}
	for (; (sm <= em) && (sm < tf->buf_size); sm++)
	{
		if (skip_message(&tf->buffer[sm], tf->chan_nr)) continue;
		while (sc < tf->buffer[sm].len)
		{
			char ch;
			if ((sm == em) && (sc > ec))
			{
				append_char(&text, '\0', &len, &max_len);
				return text;
			}
			ch = tf->buffer[sm].data[sc];
			if (ch == '\0')
			{
				break;
			}
			else if (ch == '\n')
			{
				append_char(&text, '\n', &len, &max_len);
			}
			else if (get_font_char(ch) >= 0)
			{
				append_char(&text, ch, &len, &max_len);
			}
			sc++;
		}
		append_char (&text, '\n', &len, &max_len);
		sc = 0;
	}
	append_char(&text, '\0', &len, &max_len);
	return text;
}

void text_field_remove_selection(text_field* tf)
{
	int sm, sc, em, ec;
	text_message* msg;
	select_info* select;
	
	select = &tf->select;
	if (TEXT_FIELD_SELECTION_EMPTY(select)) return;
	
	if ((select->em > select->sm) || ((select->em == select->sm) && (select->ec >= select->sc)))
	{
		sm = select->sm;
		sc = select->sc;
		em = select->em;
		ec = select->ec;
	}
	else
	{
		sm = select->em;
		sc = select->ec;
		em = select->sm;
		ec = select->sc;
	}

	// XXX Grum: should we remove messages we delete entirely?
	for (; (sm <= em) && (sm < tf->buf_size); sm++)
	{
		if (skip_message(&tf->buffer[sm], tf->chan_nr)) continue;
		msg = &tf->buffer[sm];
		if (em > sm)
		{
			if (sc == 0)
			{
				msg->data[0] = '\0';
				msg->len = 0;
			}
			else
			{
				memmove (msg->data, &(msg->data[sc]), msg->len-sc+1);
				msg->len -= sc;
			}
		}
		else
		{
			if (ec >= msg->len) ec = msg->len - 1;
			memmove(&msg->data[sc], &msg->data[ec + 1], msg->len - ec);
			msg->len -= ec - sc + 1;
			if (tf->cursor > sc) tf->cursor = sc;
		}
		sc = 0;
	}
	text_field_find_cursor_line(tf);
}

void _text_field_scroll_to_cursor (widget_list *w)
{
	text_field *tf = w->widget_info;

	if (tf == NULL || tf->scroll_id == -1)
		return;

	// scroll only if the cursor is currently not visible
	if (tf->cursor_line < tf->line_offset)
		vscrollbar_set_pos (w->window_id, tf->scroll_id, tf->cursor_line);
	else if (tf->cursor_line >= tf->line_offset + tf->nr_visible_lines)
		vscrollbar_set_pos (w->window_id, tf->scroll_id, tf->cursor_line - tf->nr_visible_lines + 1);
}

void _text_field_cursor_left (widget_list *w, int skipword)
{
	text_field *tf = w->widget_info;
	text_message* msg;
	int i;
	char c;

	if (tf == NULL || tf->cursor <= 0)
		return;

	msg = &(tf->buffer[tf->msg]);
	i = tf->cursor;
	do
	{
		c = msg->data[--i];
		if (c == '\r' || c == '\n')
			tf->cursor_line--;
	}
	while (i > 0 && (c == '\r' || (skipword && !isspace (c))));
	tf->cursor = i;

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_right (widget_list *w, int skipword)
{
	text_field *tf = w->widget_info;
	text_message* msg;
	int i;
	char c;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);
	if (tf->cursor >= msg->len)
		return;

	i = tf->cursor;
	do
	{
		c = msg->data[i++];
		if (c == '\r' || c == '\n')
			tf->cursor_line++;
	}
	while (i < msg->len && (c == '\r' || (skipword && !isspace (c))));
	tf->cursor = i;

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_up (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int line_start;      // The beginning of the line we're processing
	int prev_line_start; // Beginning of the line before the line with the cursor
	int cursor_offset;   // Position of the cursor on this line
	int prev_line_length;// Length of the previous line

	if (tf == NULL || tf->cursor_line <= 0)
		return;

	// find where the cursor is on this line
	msg = &(tf->buffer[tf->msg]);
	for (line_start = tf->cursor; line_start > 0; line_start--)
		if (msg->data[line_start-1] == '\r' || msg->data[line_start-1] == '\n')
			break;
	if (line_start == 0)
		// shouldn't happen
		return;
	cursor_offset = tf->cursor - line_start;

	// Now find where the previous line starts
	for (prev_line_start = line_start-1; prev_line_start > 0; prev_line_start--)
		if (msg->data[prev_line_start-1] == '\r' || msg->data[prev_line_start-1] == '\n')
			break;
	
	prev_line_length = line_start - prev_line_start;
	tf->cursor = cursor_offset >= prev_line_length ? line_start - 1 : prev_line_start + cursor_offset;
	tf->cursor_line--;
	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_down (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int line_start;      // The beginning of the line we're processing
	int next_line_start; // Beginning of the line after the line with the cursor
	int next_line_end;   // End of the line after the line with the cursor
	int cursor_offset;   // Position of the cursor on this line
	int next_line_length;// Length of the next line

	if (tf == NULL || tf->cursor_line >= tf->nr_lines-1)
		return;

	// find where the cursor is on this line
	msg = &(tf->buffer[tf->msg]);
	for (line_start = tf->cursor; line_start > 0; line_start--)
		if (msg->data[line_start-1] == '\r' || msg->data[line_start-1] == '\n')
			break;
	cursor_offset = tf->cursor - line_start;

	// Now find where the next line starts
	for (next_line_start = tf->cursor; next_line_start < msg->len; next_line_start++)
		if (msg->data[next_line_start] == '\r' || msg->data[next_line_start] == '\n')
			break;
	if (next_line_start >= msg->len)
		// shouldn't happen
		return;
	// skip newline
	next_line_start++;

	// Find where the next line ends
	for (next_line_end = next_line_start; next_line_end < msg->len; next_line_end++)
		if (msg->data[next_line_end] == '\r' || msg->data[next_line_end] == '\n')
			break;

	next_line_length = next_line_end - next_line_start;
	tf->cursor = cursor_offset >= next_line_length ? next_line_end : next_line_start + cursor_offset;
	tf->cursor_line++;
	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_home (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int i;

	if (tf == NULL)
		return;
		
	msg = &(tf->buffer[tf->msg]);
	for (i = tf->cursor; i > 0; i--)
		if (msg->data[i-1] == '\r' || msg->data[i-1] == '\n')
			break;

	tf->cursor = i;
	// tf->cursor_line doesn't change
}

void _text_field_cursor_end (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int i;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);
	for (i = tf->cursor; i <= msg->len; i++)
		if (msg->data[i] == '\r' || msg->data[i] == '\n' || msg->data[i] == '\0')
			break;

	tf->cursor = i;
	// tf->cursor_line doesn't change
}

void _text_field_cursor_page_up (widget_list *w)
{
	text_field *tf = w->widget_info;

	if (tf == NULL)
		return;

	if (tf->nr_visible_lines > tf->cursor_line)
	{
		tf->cursor = 0;
		tf->cursor_line = 0;
	}
	else
	{
		int i, nr_lines;
		text_message *msg = &(tf->buffer[tf->msg]);
		
		for (i = tf->cursor, nr_lines = tf->nr_visible_lines; i > 0; i--)
		{
			if (msg->data[i-1] == '\n' || msg->data[i-1] == '\r')
				if (--nr_lines == 0) break;
		}
		tf->cursor = i;
		tf->cursor_line -= tf->nr_visible_lines - 1;
	}

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_page_down (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;

	if (tf == NULL)
		return;
	
	msg = &(tf->buffer[tf->msg]);
	if (tf->cursor == msg->len || tf->nr_visible_lines <= 1)
	{
		return;
	}
	if (tf->nr_visible_lines >= tf->nr_lines - tf->cursor_line)
	{
		tf->cursor = msg->len;
		tf->cursor_line = tf->nr_lines - 1;
	}
	else
	{
		int i, nr_lines;
		
		for (i = tf->cursor+1, nr_lines = tf->nr_visible_lines-1; i < msg->len; i++)
		{
			if (msg->data[i-1] == '\n' || msg->data[i-1] == '\r')
				if (--nr_lines == 0) break;
		}
		tf->cursor = i;
		tf->cursor_line += tf->nr_visible_lines - 1;
	}

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_delete_backward (widget_list * w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int i, n = 1, nr_lines, nr_del_lines;
	
	if (tf == NULL)
		return;
	
	msg = &(tf->buffer[tf->msg]);
	i = tf->cursor;
	while (n < i && msg->data[i-n] == '\r')
		n++;
	nr_del_lines = n-1;
	if (msg->data[i-1] == '\n')
		nr_del_lines++;
	
	for ( ; i <= msg->len; i++)
		msg->data[i-n] = msg->data[i];
	msg->len -= n;
	
	// set invalid width to force rewrap
	msg->wrap_width = 0;
	nr_lines = rewrap_message (msg, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width, &tf->cursor);
	_text_field_set_nr_lines (w, nr_lines);

	tf->cursor -= n;
	tf->cursor_line -= nr_del_lines;
	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);

}

void _text_field_delete_forward (widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int i, n = 1, nr_lines;
	
	if (tf == NULL)
		return;
	
	msg = &(tf->buffer[tf->msg]);
	i = tf->cursor;
	while (i+n <= msg->len && msg->data[i+n] == '\r')
		n++;

	for (i += n; i <= msg->len; i++)
		msg->data[i-n] = msg->data[i];

	msg->len -= n;
	// set invalid width to force rewrap
	msg->wrap_width = 0;
	nr_lines = rewrap_message (msg, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width, &tf->cursor);
	_text_field_set_nr_lines (w, nr_lines);
	
	// cursor position doesn't change, so no need to update it here
}

void _text_field_insert_char (widget_list *w, char ch)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int nr_lines, old_cursor;

	if (tf == NULL)
		return;
	
	msg = &(tf->buffer[tf->msg]);
	
	if (ch == SDLK_RETURN)
		ch = '\n';

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
	if (ch == '\n')
		tf->cursor_line++;
	
	// set invalid width to force rewrap
	msg->wrap_width = 0;
	// Save the current character position, and rewrap the message.
	// The difference between the old and the new position should
	// be the number of extra line breaks introduced before the 
	// cursor position
	old_cursor = tf->cursor;
	nr_lines = rewrap_message (msg, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width, &tf->cursor);
	tf->cursor_line += tf->cursor - old_cursor;
	_text_field_set_nr_lines (w, nr_lines);

	// XXX FIXME: Grum: is the following even possible?
	while (msg->data[tf->cursor] == '\r') {
		tf->cursor++;
		tf->cursor_line++;
	}

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_copy_to_clipboard(text_field *tf)
{
	char* text = text_field_get_selected_text(tf);
	if (text != NULL)
	{
		copy_to_clipboard(text);
		free(text);
	}
}

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
void _text_field_copy_to_primary(text_field *tf)
{
	char* text = text_field_get_selected_text(tf);
	if (text)
	{
		copy_to_primary(text);
		free(text);
	}
}
#endif
#endif

static void update_cursor_selection(widget_list* w, int flag);

int text_field_resize (widget_list *w, int width, int height)
{
	text_field *tf = w->widget_info;

	if (tf != NULL)
	{
		if (tf->scroll_id != -1)
		{
			widget_move (w->window_id, tf->scroll_id, w->pos_x + width - tf->scrollbar_width, w->pos_y);
			widget_resize (w->window_id, tf->scroll_id, tf->scrollbar_width, height);
		}
		if (tf->buffer != NULL)
		{
			int i, nr_lines = 0, nr_vis = tf->nr_visible_lines;

			_text_field_set_nr_visible_lines (w);
			if (tf->nr_visible_lines != nr_vis)
				tf->select.lines = realloc (tf->select.lines, tf->nr_visible_lines * sizeof (text_field_line));
			tf->select.sm = tf->select.em = tf->select.sc = tf->select.ec = -1;

			for (i = 0; i < tf->buf_size; i++)
			{
				int *cursor = i == tf->msg ? &(tf->cursor) : NULL;
				nr_lines += rewrap_message (tf->buffer+i, w->size, width - tf->scrollbar_width, cursor);
			}
			_text_field_set_nr_lines (w, nr_lines);

			text_field_find_cursor_line (tf);
		}
	}

	return 1;
}

static int insert_window_id = -1;
static int insert_widget_id = -1;

/* insert the given text string into the text widget */
static void text_widget_insert(const char *thestring)
{
	widget_list* w = widget_find (insert_window_id, insert_widget_id);
	if (w != NULL)
	{
		Uint32 saved_flag = w->Flags & TEXT_FIELD_NO_KEYPRESS;
		if (w->Flags & TEXT_FIELD_MOUSE_EDITABLE)
			w->Flags &= ~TEXT_FIELD_NO_KEYPRESS;
		widget_unset_flags(insert_window_id, insert_widget_id, WIDGET_DISABLED);
		do_paste_to_text_field(w, thestring);
		w->Flags |= saved_flag;
	}
	insert_window_id = insert_widget_id = -1;
}


/* the edit context menu callback */
static int context_edit_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	widget_list* w = NULL;
	Uint32 saved_flag;

	if (win == NULL)
		return 0;
	w = widget_find (win->window_id, widget_id);
	if (w == NULL)
		return 0;

	saved_flag = w->Flags & TEXT_FIELD_NO_KEYPRESS;
	if (w->Flags & TEXT_FIELD_MOUSE_EDITABLE)
		w->Flags &= ~TEXT_FIELD_NO_KEYPRESS;
	switch (option)
	{
		case 0: text_field_keypress(w, 0, 0, K_CUT, 24); break;
		case 1: text_field_keypress(w, 0, 0, K_COPY, 3); break;
		case 2:
			if (!text_field_keypress(w, 0, 0, K_PASTE, 22))
				start_paste(NULL);
			break;
		case 4:
			{
				insert_window_id = win->window_id;
				insert_widget_id = widget_id;
				get_date(text_widget_insert);
			}
			break;
		case 5:
			{
				char str[20];
				safe_snprintf(str, sizeof(str), "%1d:%02d:%02d", real_game_minute/60, real_game_minute%60, real_game_second);
				widget_unset_flags(win->window_id, widget_id, WIDGET_DISABLED);
				do_paste_to_text_field(w, str);
			}
			break;
		case 6:
			{
				actor *me = get_our_actor ();
				if (me != NULL)
				{
					char str[20];
					safe_snprintf(str, sizeof(str), "%d,%d", me->x_tile_pos, me->y_tile_pos);
					widget_unset_flags(win->window_id, widget_id, WIDGET_DISABLED);
					do_paste_to_text_field(w, str);
				}
			}
			break;
	}
	w->Flags |= saved_flag;
	return 1;
}

/* the edit context menu pre show callback */
static void context_edit_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	widget_list* w = NULL;
	text_field *tf;
	int is_grey = 0;

	if (win == NULL)
		return;
	w = widget_find (win->window_id, widget_id);
	if (w == NULL)
		return;

	tf = w->widget_info;
	is_grey = TEXT_FIELD_SELECTION_EMPTY(&tf->select);
	cm_grey_line(cm_edit_id, 1, is_grey);

	is_grey = is_grey || !(w->Flags & TEXT_FIELD_EDITABLE)
		|| ((w->Flags & TEXT_FIELD_NO_KEYPRESS) && !(w->Flags & TEXT_FIELD_MOUSE_EDITABLE));
	cm_grey_line(cm_edit_id, 0, is_grey);

	is_grey = !((w->Flags & TEXT_FIELD_EDITABLE) || (w->Flags & TEXT_FIELD_MOUSE_EDITABLE));
	cm_grey_line(cm_edit_id, 2, is_grey);
	cm_grey_line(cm_edit_id, 4, is_grey);
	cm_grey_line(cm_edit_id, 5, is_grey);
	cm_grey_line(cm_edit_id, 6, is_grey);
}

int text_field_keypress(widget_list *w, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint16 keysym = key & 0xffff;
	Uint8 ch = key_to_char(unikey);
	text_field *tf;
	text_message *msg;
	int alt_on = key & ELW_ALT, ctrl_on = key & ELW_CTRL;
	int shift_on = key & ELW_SHIFT;

	if (w == NULL) return 0;
	tf = w->widget_info;

	if (key == K_COPY || key == K_COPY_ALT)
	{
		_text_field_copy_to_clipboard (tf);
		return 1;
	}

	if ( !(w->Flags & TEXT_FIELD_EDITABLE) ) return 0;
	if (w->Flags & TEXT_FIELD_NO_KEYPRESS) return 0;

	msg = &(tf->buffer[tf->msg]);

	if (is_printable (ch) || keysym == SDLK_UP || keysym == SDLK_DOWN ||
		keysym == SDLK_LEFT || keysym == SDLK_RIGHT || keysym == SDLK_HOME ||
		keysym == SDLK_END || ch == SDLK_BACKSPACE || ch == SDLK_DELETE
#ifdef OSX
		|| keysym == 127
#endif
		)
		{
		/* Stop blinking on input */
		tf->next_blink = cur_time + TF_BLINK_DELAY;
	}

	if ((keysym == SDLK_LEFT) || (keysym == SDLK_RIGHT) || (keysym == SDLK_UP) || (keysym == SDLK_DOWN) ||
		(keysym == SDLK_HOME) || (keysym == SDLK_END))
	{
		if (shift_on)
		{
			if (TEXT_FIELD_SELECTION_EMPTY(&tf->select))
				update_cursor_selection(w, 0);
		}
		else
		{
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		}
	}

	if (keysym == SDLK_LEFT)
	{
		_text_field_cursor_left (w, ctrl_on);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_RIGHT)
	{
		_text_field_cursor_right (w, ctrl_on);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_UP && !ctrl_on && !alt_on && tf->cursor >= 0)
	{
		_text_field_cursor_up (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_DOWN && !ctrl_on && !alt_on && tf->cursor >= 0)
	{
		_text_field_cursor_down (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_HOME)
	{
		if (ctrl_on)
		{
			tf->cursor = 0;
			tf->cursor_line = 0;
			if (tf->scroll_id != -1)
				_text_field_scroll_to_cursor (w);
		}
		else
		{
			_text_field_cursor_home (w);
		}
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_END)
	{
		if (ctrl_on)
		{
			tf->cursor = msg->len;
			tf->cursor_line = tf->nr_lines - 1;
			if (tf->scroll_id != -1)
				_text_field_scroll_to_cursor (w);
		}
		else
		{
			_text_field_cursor_end (w);
		}
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_PAGEUP)
	{
		_text_field_cursor_page_up (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (keysym == SDLK_PAGEDOWN)
	{
		_text_field_cursor_page_down (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if ((ch == SDLK_BACKSPACE || ch == SDLK_DELETE
#ifdef OSX
	          || ch == 127
#endif
	         ) && !TEXT_FIELD_SELECTION_EMPTY(&tf->select))
	{
		text_field_remove_selection(tf);
		TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		return 1;
	}
#ifdef OSX
        else if (ch == SDLK_BACKSPACE || ch == 127)
#else
        else if (ch == SDLK_BACKSPACE)
#endif
	{
		if (tf->cursor > 0)
			_text_field_delete_backward (w);
		return 1;
	}
	else if (ch == SDLK_DELETE)
	{
		if (tf->cursor < msg->len)
			_text_field_delete_forward (w);
		return 1;
	}
	else if (key == K_CUT)
	{
		_text_field_copy_to_clipboard(tf);
		if (!TEXT_FIELD_SELECTION_EMPTY(&tf->select))
		{
			text_field_remove_selection(tf);
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		}
		return 1;
	}
	else if (key == K_PASTE || key == K_PASTE_ALT)
	{
		if (!TEXT_FIELD_SELECTION_EMPTY(&tf->select))
		{
			text_field_remove_selection(tf);
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		}
		start_paste(w);
		return 1;
	}
	else if (!alt_on && !ctrl_on && ( is_printable (ch)
			|| (ch == SDLK_RETURN && !(w->Flags&TEXT_FIELD_IGNORE_RETURN)) ) && ch != '`' )
	{
		if (!TEXT_FIELD_SELECTION_EMPTY(&tf->select))
		{
			text_field_remove_selection(tf);
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);		
		}
		_text_field_insert_char (w, ch);
		return 1;
	}
	return 0;
}

void _set_edit_pos (text_field* tf, int x, int y)
{
	unsigned int i = tf->offset;
	unsigned int nrlines = 0, line = 0;
	int px = 0;
	text_message* msg = &(tf->buffer[tf->msg]);
	float displayed_font_y_size = floor (DEFAULT_FONT_Y_LEN * msg->wrap_zoom);

	if (msg->len == 0)
		return;	// nothing to do, there is no string

	nrlines = (int) (y/displayed_font_y_size);
	for (; line < nrlines && i < msg->len; i++) {
		switch (msg->data[i]) {
			case '\r':
			case '\n':
				++line;
				break;
			case '\0':
				tf->cursor = i;
				tf->cursor_line = tf->line_offset + line;
				return;
		}
	}

	tf->cursor_line = tf->line_offset + nrlines;
	for (; i < msg->len; i++) {
		switch (msg->data[i]) {
			case '\r':
			case '\n':
			case '\0':
				tf->cursor = i;
				return;
			default:
				// lachesis: for formula see draw_char_scaled
				px += (int) (0.5 + get_char_width(msg->data[i]) * msg->wrap_zoom * DEFAULT_FONT_X_LEN / 12.0);
				if (px >= x)
				{
					tf->cursor = i;
					return;
				}
		}
	}
	tf->cursor = msg->len;
}

void update_selection(int x, int y, widget_list* w, int drag)
{
	int line, col;
	int cx = 0;
	float displayed_font_y_size = floorf(DEFAULT_FONT_Y_LEN * w->size);
	text_field* tf;
	text_message* msg;

	tf = w->widget_info;
	if (tf == NULL) return;

	line = y / displayed_font_y_size;
	if (line < 0 || line >= tf->nr_visible_lines || tf->select.lines[line].msg == -1)
	{
		// Invalid position, if we were dragging keep the selection
		// intact, but if this was a click, clear it
		if (!drag)
			tf->select.sm = tf->select.sc = tf->select.em = tf->select.ec = -1;
		return;
	}

	msg = &tf->buffer[tf->select.lines[line].msg];
	for (col = tf->select.lines[line].chr; col < msg->len; col++)
	{
		if (msg->data[col] == '\r' || msg->data[col] == '\n' || msg->data[col] == '\0')
			break;
		cx += (0.5 + get_char_width(msg->data[col]) * w->size * DEFAULT_FONT_X_LEN / 12.0);
		if (cx >= x)
			break;
	}
	if (!drag || tf->select.sm == -1 || tf->select.sc == -1)
	{
		// click (or selection still empty), set the start position
		tf->select.sm = tf->select.lines[line].msg;
		tf->select.sc = col;
		tf->select.em = -1;
		tf->select.ec = -1;
	}
	else
	{
		// drag, set the end position
		tf->select.em = tf->select.lines[line].msg;
		tf->select.ec = col;
#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
		_text_field_copy_to_primary(tf);
#endif
#endif
	}
}

static void update_cursor_selection(widget_list* w, int update_end)
{
	text_field* tf;
	int line;

	tf = w->widget_info;
	if (tf == NULL) return;

	line = tf->cursor_line - tf->line_offset;
	if (line < 0)
		line = 0;
	else if (line >= tf->nr_visible_lines)
		line = tf->nr_visible_lines - 1;

	if (!update_end)
	{
		tf->select.sm = tf->select.lines[line].msg;
		tf->select.sc = tf->cursor;
		tf->select.em = -1;
		tf->select.ec = -1;
	}
	else
	{
		tf->select.em = tf->select.lines[line].msg;
		tf->select.ec = tf->cursor;
#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
		_text_field_copy_to_primary(tf);
#endif
#endif
	}
}

int text_field_click(widget_list *w, int mx, int my, Uint32 flags)
{
	Uint32 buttons;
	text_field *tf;
	int em_before;

	tf = w->widget_info;
	if (tf == NULL)
		return 0;

	// send scroll wheel moves to the scrollbar
	if ((flags & ELW_WHEEL) != 0 && tf->scroll_id != -1)
	{
		widget_list* sbw = widget_find (w->window_id, tf->scroll_id);
		return vscrollbar_click (sbw, mx, my, flags);
	}

	// if no scrollbar, only handle mouse button clicks,
	// not scroll wheels moves
	buttons = flags & ELW_MOUSE_BUTTON;
	if (!buttons)
		return 0;

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
	// Don't handle middle button clicks (paste) if the text field is not editable
	if (buttons == ELW_MID_MOUSE && !(w->Flags & TEXT_FIELD_EDITABLE))
		return 0;
#endif
#endif

	em_before = tf->select.em;
	update_selection(mx, my, w, 0);
	if (em_before != -1 && tf->select.em == -1)
		/* We deselected some text, click was handled */
		return 1;

	if ( (w->Flags & TEXT_FIELD_EDITABLE) == 0)
		return 0;

	_set_edit_pos(tf, mx, my);

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
	if (flags & ELW_MID_MOUSE)
		start_paste_from_primary(w);
#endif // MIDDLE_MOUSE_PASTE
#endif

	return 1;
}

int text_field_drag(widget_list *w, int mx, int my, Uint32 flags, int dx, int dy)
{
	update_selection(mx, my, w, 1);
	return 1;
}

int text_field_destroy (widget_list *w)
{
	text_field *tf = w->widget_info;
	if (tf != NULL)
	{
		/* remove the context menu */
		cm_remove_widget(w->window_id, w->id);
		if (tf->scroll_id != -1)
			widget_destroy (w->window_id, tf->scroll_id);
		if (tf->select.lines != NULL) free(tf->select.lines);
		free (tf);
	}

	return 1;
}

const struct WIDGET_TYPE text_field_type = { 
	NULL, 
	text_field_draw, 
	text_field_click, 
	text_field_drag, 
	NULL, 
	text_field_resize, 
	text_field_keypress, 
	text_field_destroy
};

int text_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, text_message *buf, int buf_size, Uint8 chan_filt, int x_space, int y_space)
{
	int res;

	text_field *T = calloc (1, sizeof (text_field));
	T->x_space = x_space,
	T->y_space = y_space,
	T->msg = 0;
	T->offset = 0;
	T->buffer = buf;
	T->buf_size = buf_size;
	T->nr_lines = 1; //We'll always have one line in the text field.
	T->chan_nr = chan_filt;
	T->cursor = (Flags & TEXT_FIELD_EDITABLE) ? 0 : -1;
	T->cursor_line = T->cursor;
	T->next_blink = TF_BLINK_DELAY;
	if (Flags & TEXT_FIELD_SCROLLBAR)
	{
		T->scrollbar_width = ELW_BOX_SIZE;
		T->scroll_id = vscrollbar_add_extended (window_id, widget_id++, NULL, x + lx-T->scrollbar_width, y, T->scrollbar_width, ly, 0, size, r, g, b, 0, 1, 1);		
	}
	else
	{
		T->scroll_id = -1;
		T->scrollbar_width = 0;
	}

	res = widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, r, g, b, &text_field_type, T, NULL);

	if (buf != NULL)
	{
		// We need to set the correct nr of lines if we want the
		// scrollbar to work, but we couldn't do this earlier, since we
		// need the widget itself which was only just created.
		widget_list *w = widget_find (window_id, wid);
		if (w != NULL)
		{
			int nr_lines = rewrap_message (buf, w->size, lx - 2*x_space - T->scrollbar_width, &T->cursor);
			_text_field_set_nr_visible_lines (w);
			_text_field_set_nr_lines (w, nr_lines);
			T->select.lines = (text_field_line*) calloc(T->nr_visible_lines, sizeof(text_field_line));
			T->select.sm = T->select.em = T->select.sc = T->select.ec = -1;
		}
	}

	/* on the first occurance create the editting context menu */
	/* maintain a activation entry for each widget so they can be removed or modified */
	if (!cm_valid(cm_edit_id))
	{
		cm_edit_id = cm_create(cm_textedit_menu_str, context_edit_handler);
		cm_set_pre_show_handler(cm_edit_id, context_edit_pre_show_handler);
	}
	/* assign to the new widget */
	cm_add_widget(cm_edit_id, window_id, res);

	return res;
}

int text_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, text_message *buf, int buf_size, int x_space, int y_space)
{
	return text_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, TEXT_FIELD_BORDER, 1.0, -1.0, -1.0, -1.0, buf, buf_size, FILTER_ALL, x_space, y_space);
}

int text_field_draw (widget_list *w)
{
	text_field *tf;
	int cursor;
	int mx, my;
	int i;

	if (w == NULL || w->window_id < 0 || w->widget_info == NULL) {
		return 0;
	}

	mx = mouse_x - windows_list.window[w->window_id].pos_x - w->pos_x;
	my = mouse_y - windows_list.window[w->window_id].pos_y - w->pos_y;

	tf = w->widget_info;
	if (tf->scroll_id != -1)
	{
		/* We have a scrollbar, so check its position and update
		 * the text offset if necessary.
		 */
		int pos;

		if (tf->update_bar)
		{
			int nr_lines;
			int old_cursor = tf->cursor;
			tf->buffer[tf->msg].wrap_width = 0;
			nr_lines = rewrap_message (&tf->buffer[tf->msg], w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width, &tf->cursor);
			tf->cursor_line += tf->cursor - old_cursor;
			_text_field_set_nr_lines (w, nr_lines);
			_text_field_scroll_to_cursor(w);
		}
		pos = vscrollbar_get_pos (w->window_id, tf->scroll_id);
		if (pos > tf->line_offset)
		{
			int delta = pos - tf->line_offset, i;
			for (i = tf->offset; i < tf->buffer->len; i++)
			{
				if (tf->buffer->data[i] == '\n' || tf->buffer->data[i] == '\r')
					if (--delta == 0) break;
			}
			tf->offset = i+1;
			tf->line_offset = pos; 
		}
		else if (pos < tf->line_offset)
		{
			int delta = tf->line_offset - pos + 1, i;
			i = tf->offset;
			if (i >= tf->buffer[tf->msg].len)
				i = tf->buffer[tf->msg].len - 1;
			for (; i > 0; i--)
			{
				if (tf->buffer->data[i-1] == '\n' || tf->buffer->data[i-1] == '\r')
					if (--delta == 0) break;
			}
			tf->offset = i;
			tf->line_offset = pos;
		}
	}


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

	if(mx > 0 && mx < w->len_x - tf->scrollbar_width && my > 0 && my < w->len_y && !(w->Flags&WIDGET_CLICK_TRANSPARENT) && w->Flags&TEXT_FIELD_EDITABLE){
		elwin_mouse = CURSOR_TEXT;
	}

	/* Make the cursor blink if the mouse is in the widget */
	if((mx > 0 && mx < w->len_x && my > 0 && my < w->len_y)
		&& cur_time < tf->next_blink - TF_BLINK_DELAY) {
		cursor = -1;
	} else if(cur_time < tf->next_blink) {
		cursor = tf->cursor-tf->offset;
	} else {
		tf->next_blink = cur_time + 2*TF_BLINK_DELAY;
		cursor = tf->cursor-tf->offset;
	}

	glEnable(GL_TEXTURE_2D);
	set_font(chat_font);	// switch to the chat font

	for (i = 0; i < tf->nr_visible_lines; i++)
		tf->select.lines[i].msg = -1;
	draw_messages (w->pos_x + tf->x_space, w->pos_y + tf->y_space, tf->buffer, tf->buf_size, tf->chan_nr, tf->msg, tf->offset, cursor, w->len_x - 2*tf->x_space - tf->scrollbar_width, w->len_y - 2 * tf->y_space, w->size, &tf->select);
	if (tf->nr_visible_lines && tf->select.lines[0].msg == -1)
	{
		tf->select.lines[0].msg = tf->msg;
		tf->select.lines[0].chr = tf->buffer[tf->msg].len;
	}
	for (i = 1; i < tf->nr_visible_lines; i++)
	{
		if (tf->select.lines[i].msg == -1)
		{
			tf->select.lines[i].msg = tf->select.lines[i - 1].msg;
			tf->select.lines[i].chr = tf->buffer[tf->select.lines[i].msg].len;
		}
	}
	set_font (0);	// switch to fixed
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int text_field_set_buf_pos (int window_id, Uint32 widget_id, int msg, int offset)
{
	text_field *tf;
	widget_list *w = widget_find (window_id, widget_id);
	if (w == NULL) {
		return 0;
	}

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
	else if (offset > tf->buffer[msg].len)
	{
		offset = tf->buffer[msg].len;
	}

	tf->msg = msg;
	tf->offset = offset;

	return  1;
}

int text_field_clear (int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find (window_id, widget_id);
	text_field *tf;
	text_message *msg;

	if (w == NULL)
		return 0;

	tf = w->widget_info;
	if (tf == NULL || !(w->Flags & TEXT_FIELD_EDITABLE))
		return 0;

	msg = &(tf->buffer[tf->msg]);
	clear_text_message_data (msg);

	tf->cursor = 0;
	tf->cursor_line = 0;
	tf->offset = 0;
	tf->line_offset = 0;
	if (tf->scroll_id != -1)
	{
		vscrollbar_set_bar_len (window_id, tf->scroll_id, 0);
		vscrollbar_set_pos (window_id, tf->scroll_id, 0);
		tf->update_bar = 1;
	}

	return 1;
}

//password entry field. We act like a restricted text entry with multiple modes
// quite straightforward - we just add or remove from the end
int pword_keypress (widget_list *w, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char(unikey);
	password_entry *pword;
	int alt_on = key & ELW_ALT,
	    ctrl_on = key & ELW_CTRL;

	if (w == NULL) {
		return 0;
	}
	pword = (password_entry *) w->widget_info;

	if (pword->status == P_NONE) {
		return -1;
	}

	if (ch == SDLK_BACKSPACE) {
		int i;

		for(i = 0; pword->password[i] != '\0' && i < pword->max_chars; i++);
		if(i > 0) {
			pword->password[i-1] = '\0';
		}

		return 1;
	} else if (!alt_on && !ctrl_on && is_printable (ch) && ch != '`' ) {
		int i;
		
		for(i = 0; pword->password[i] != '\0' && i < pword->max_chars-1; i++);
		if(i >= 0) {
			pword->password[i] = ch;
			pword->password[i+1] = '\0';
		}
		return 1;
	} else {
		return 0;
	}
}
		
int pword_field_click(widget_list *w, int mx, int my, Uint32 flags)
{
	password_entry *pword;

	if (w == NULL) return 0;
	pword = (password_entry*) w->widget_info;
	if(pword->status == P_NONE) {
		return -1;
	} else {
		return 1;   // Don't fall through
	}
}

int pword_field_draw (widget_list *w)
{
	password_entry *pword;
	unsigned char *text;
	int difference;
	int i;

	if (w == NULL) {
		return 0;
	}
	pword = (password_entry*) w->widget_info;
	difference = (get_string_width((unsigned char*)pword->password)*w->size - w->len_x)/12;

	/*if you want the text cursor, uncomment the following... as clicking goes
	to the end of the line, and you can't jump part way through, using the text
	cursor will not give the user the right idea*/
	/*if(mx > 0 && mx < w->len_x && my > 0 && my < w->len_y && !(w->Flags&WIDGET_CLICK_TRANSPARENT)){
		elwin_mouse = CURSOR_TEXT;
	}*/

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
		draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, (unsigned char*)"N/A", 1, w->size);
	} else if(pword->status == P_TEXT) {
		if(difference > 0) {
			/* Only draw the end of the string */
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, (unsigned char*)pword->password+difference, 1, w->size);
		} else {
			draw_string_zoomed(w->pos_x + 2, w->pos_y + 2, (unsigned char*)pword->password, 1, w->size);
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

void pword_set_status(widget_list *w, Uint8 status)
{
	password_entry *pword;
	if (w == NULL) {
		return;
	}
	pword = (password_entry*) w->widget_info;
	pword->status = status;
}

const struct WIDGET_TYPE pword_field_type = {
 	NULL,
 	pword_field_draw,
 	pword_field_click,
 	NULL,
 	NULL,
 	NULL,
 	pword_keypress,
 	free_widget_info
 	};

int pword_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, float size, float r, float g, float b, unsigned char *buffer, int buffer_size)
{
	password_entry *T = calloc (1, sizeof (password_entry));
	T->status = status;
	T->password = (char*)buffer;
	T->max_chars = buffer_size;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, 0, size, r, g, b, &pword_field_type, T, NULL);
}

int pword_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, unsigned char *buffer, int buffer_size)
{
 	return pword_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, status, 1.0, -1.0, -1.0, -1.0, buffer, buffer_size);
}

// Multiselect
int free_multiselect(widget_list *widget)
{
	if(widget != NULL) {
		multiselect *collection = widget->widget_info;
		free(collection->buttons);
		free(collection);
	}
	return 1;
}

int multiselect_get_selected(int window_id, Uint32 widget_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M = widget->widget_info;
	if(M == NULL) {
		return -1;
	} else {
		if (M->selected_button < M->nr_buttons)
			return M->buttons[M->selected_button].value;
		else
			return -1;
	}
}

int multiselect_set_selected(int window_id, Uint32 widget_id, int button_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M = widget->widget_info;
	if(M == NULL) {
		return -1;
	} else {
		int i;
		for (i=0; i<M->nr_buttons; i++) {
			if (button_id == M->buttons[i].value) {			
				M->selected_button = i;
				return button_id;
			}
		}
		return -1;
	}
}

int multiselect_get_height(int window_id, Uint32 widget_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	return widget->len_y;
}

int multiselect_click(widget_list *widget, int mx, int my, Uint32 flags)
{
	multiselect *M = widget->widget_info;
	int i;
	Uint16 button_y;
	int top_but = 0;
	int start_y = 0;

	if(M->scrollbar != -1)
	{
		vscrollbar *scrollbar = widget_find(M->win_id, M->scrollbar)->widget_info;
		if(flags&ELW_WHEEL_DOWN)
			vscrollbar_set_pos(M->win_id, M->scrollbar, scrollbar->pos+scrollbar->pos_inc);
		else if (flags&ELW_WHEEL_UP)
			vscrollbar_set_pos(M->win_id, M->scrollbar, scrollbar->pos-scrollbar->pos_inc);
		top_but = vscrollbar_get_pos(M->win_id, M->scrollbar);
		start_y = M->buttons[top_but].y;
	}

	for(i = top_but; i < M->nr_buttons; i++)
	{
		button_y = M->buttons[i].y - start_y;
		if (button_y > widget->len_y-multiselect_button_height)
			break;
		if((flags&ELW_LEFT_MOUSE || flags&ELW_RIGHT_MOUSE) && 
			my > button_y && my < button_y+multiselect_button_height && mx > M->buttons[i].x && mx < M->buttons[i].x+M->buttons[i].width) {
				M->selected_button = i;
			do_click_sound();
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
		int top_but = (M->scrollbar != -1) ?vscrollbar_get_pos(M->win_id, M->scrollbar) :0;
		int start_y = M->buttons[top_but].y;

		r = widget->r != -1 ? widget->r : 0.77f;
		g = widget->g != -1 ? widget->g : 0.59f;
		b = widget->b != -1 ? widget->b : 0.39f;
		
		hr = M->highlighted_red != -1 ? M->highlighted_red : 0.32f;
		hg = M->highlighted_green != -1 ? M->highlighted_green : 0.23f;
		hb = M->highlighted_blue != -1 ? M->highlighted_blue : 0.15f;
		
		for(i = top_but; i < M->nr_buttons; i++) {
			button_y = M->buttons[i].y - start_y;
			/* Check if the button can be fully drawn */
			if(button_y > widget->len_y-multiselect_button_height)
				break;
			draw_smooth_button(M->buttons[i].text, widget->size, widget->pos_x+M->buttons[i].x, widget->pos_y+button_y, M->buttons[i].width-22, 1, r, g, b, (i == M->selected_button), hr, hg, hb, 0.5f);
		}
	}
	return 1;
}

int multiselect_button_add(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, const char *text, const char selected)
{
	return multiselect_button_add_extended(window_id, multiselect_id, x, y, 0, text, DEFAULT_SMALL_RATIO, selected);
}

int multiselect_button_add_extended(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, int width, const char *text, float size, const char selected)
{
	widget_list *widget = widget_find(window_id, multiselect_id);
	multiselect *M = widget->widget_info;
	int current_button = M->nr_buttons;
	
	if (text==NULL || strlen(text)==0) {
		M->next_value++;
		return current_button;
	}

	if(y+multiselect_button_height > widget->len_y && (!M->max_height || widget->len_y != M->max_height)) {
		widget->len_y = y+multiselect_button_height;
	}

	widget->size=size;

	if (M->max_height && y+multiselect_button_height > M->actual_height) {
		M->actual_height = y+multiselect_button_height;
	}
	if(M->max_buttons == M->nr_buttons) {
		/*Allocate space for more buttons*/
		M->buttons = realloc(M->buttons, sizeof(*M->buttons) * M->max_buttons * 2);
		M->max_buttons *= 2;
	}
	safe_snprintf(M->buttons[current_button].text, sizeof(M->buttons[current_button].text), "%s", text);
	if(selected) {
		M->selected_button = current_button;
	}
	M->buttons[current_button].value = M->next_value++;
	M->buttons[current_button].x = x;
	M->buttons[current_button].y = y;
	M->buttons[current_button].width = (width == 0) ? widget->len_x : width;
	
	M->nr_buttons++;
	if(M->max_height && M->scrollbar == -1 && M->max_height < y) {
		int i;

		/* Add scrollbar */
		M->scrollbar = vscrollbar_add_extended(window_id, widget_id++, NULL, widget->pos_x+widget->len_x-ELW_BOX_SIZE, widget->pos_y, ELW_BOX_SIZE, M->max_height, 0, 1.0, widget->r, widget->g, widget->b, 0, 1, M->max_height);
		widget->len_x -= ELW_BOX_SIZE;
		widget->len_y = M->max_height;
		/* We don't want things to look ugly. */
		for(i = 0; i < M->nr_buttons; i++) {
			if(M->buttons[i].width > widget->len_x) {
				M->buttons[i].width -= ELW_BOX_SIZE;
			}
		}
	}
	
	if (M->scrollbar != -1)
		vscrollbar_set_bar_len(window_id, M->scrollbar, M->nr_buttons-widget->len_y/multiselect_button_height);
	
	return current_button;
}

const struct WIDGET_TYPE multiselect_type = {
 	NULL,
 	multiselect_draw,
 	multiselect_click,
 	NULL,
 	NULL,
 	NULL,
 	NULL,
 	free_multiselect
	};

int multiselect_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, int width, Uint16 max_height, float size, float r, float g, float b, float hr, float hg, float hb, int max_buttons)
  {
 	multiselect *T = calloc (1, sizeof (multiselect));
  	//Save info
 	T->max_buttons = max_buttons > 0 ? max_buttons : 2;
 	T->selected_button = 0;
	T->next_value = 0;
 	T->nr_buttons = 0;
 	T->buttons = malloc(sizeof(*T->buttons) * T->max_buttons);
	T->max_height = max_height;
 	T->scrollbar = -1;
 	T->win_id = window_id;
 	T->highlighted_red = hr;
 	T->highlighted_green = hg;
	T->highlighted_blue = hb;
 
 	return widget_add (window_id, wid, OnInit, x, y, width, 0, 0, size, r, g, b, &multiselect_type, T, NULL);
}
  
int multiselect_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, int width)
{
 	return multiselect_add_extended(window_id, widget_id++, OnInit, x, y, width, 0, 1.0f, -1, -1, -1, -1, -1, -1, 0);
}

// Spinbutton
int spinbutton_keypress(widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey)
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
						safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%i", (int)button->max);
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
							safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%.2f", button->max);
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

// Note: Discards dx and dy when used for drag. Must be altered if the drag
//		handler changes.
int spinbutton_click(widget_list *widget, int mx, int my, Uint32 flags)
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
			do_click_sound();
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
					safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%i", *(int *)button->data);
				break;
				case SPIN_FLOAT:
					switch (action) {
						case 'i':
							//if(*(float *)button->data + button->interval <= button->max+0.000001) { //+0.000001 to avoid issues with floating point values
							if (*(float *)button->data + button->interval <= button->max) { // NOTE: Can't do that, values > max may cause crashes. Change the max value intead.
								*(float *)button->data += button->interval;
							} else {
								*(float *)button->data = button->max;
							}
						break;
						case 'd':
							if(*(float *)button->data - button->interval >= button->min) {
								*(float *)button->data -= button->interval;
							} else {
								*(float *)button->data = button->min;
							}								
						break;
					}
					safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%.2f", *(float *)button->data);
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
				safe_snprintf(str, sizeof (str), "%s", button->input_buffer);
			} else {
				*(int *)button->data = atoi(button->input_buffer);
				safe_snprintf(str, sizeof (str), "%i", *(int *)button->data);
			}
		break;
		case SPIN_FLOAT:
			if(atof(button->input_buffer) < button->min) {
				glColor3f(1, 0, 0);
				safe_snprintf(str, sizeof (str), "%s", button->input_buffer);
			} else {
				char *pointer = strchr(button->input_buffer, '.');
				int accuracy;
				char format[10];

				if(pointer == NULL) 
					pointer = strchr(button->input_buffer, ',');

				accuracy = (pointer == NULL) ? 0 : strlen(pointer+1);
				if(accuracy > 3) {
					accuracy = 3;
				}
				safe_snprintf(format, sizeof (format), "%%.%if", accuracy);
				safe_snprintf(str, sizeof (str), format, *(float *)button->data);
				if(accuracy == 0 && pointer != NULL) {
					/* We have a . at the end of the input buffer, but 
					 * safe_snprintf() doesn't write it, so we have to do it manually. */
					safe_strcat (str, ".", sizeof (str));
				}
			}
		break;
	}
	/* Numbers */
	glColor3f(widget->r, widget->g, widget->b);
	draw_string_zoomed(widget->pos_x + 2 + gx_adjust, widget->pos_y + 2 + gy_adjust, (unsigned char*)str, 1, widget->size);
	glDisable(GL_TEXTURE_2D);
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

const struct WIDGET_TYPE spinbutton_type = { NULL,
	spinbutton_draw,
	spinbutton_click,
	spinbutton_click,
	NULL,
	NULL,
	spinbutton_keypress,
	free_multiselect
	};
int spinbutton_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval, float size, float r, float g, float b)
{
	float wr = r >= 0 ? r : 0.77, wg = g >= 0 ? g : 0.59, wb = b >= 0 ? b : 0.39;

	spinbutton *T = calloc (1, sizeof (spinbutton));
	// Filling the widget info
	T->data = data;
	T->max = max;
	T->min = min;
	T->type = data_type;
	T->interval = interval;
	switch(data_type)
	{
		case SPIN_FLOAT:
			safe_snprintf(T->input_buffer, sizeof(T->input_buffer), "%.2f", *(float *)T->data);
		break;
		case SPIN_INT:
			T->interval = (int)T->interval;
			safe_snprintf(T->input_buffer, sizeof(T->input_buffer), "%i", *(int *)T->data);
		break;
	}

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, 0, size, wr, wg, wb, &spinbutton_type, T, NULL);
}

int spinbutton_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval)
{
	return spinbutton_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, data_type, data, min, max, interval, 1, -1, -1, -1);
}


// XML Windows
/* 
/ This section contains the following (unused) functions:
/	int AddXMLWindow(char *fn);
/	int ReadXMLWindow(xmlNode *a_node);
/	int ParseWindow(xmlNode *node);
/	int ParseWidget(xmlNode *node, int winid);
/	int ParseTab(xmlNode *node, int winid, int colid);
/	int GetWidgetType(const char *w);
/ As well as definitions for symbolic constants for all of the widget types.
*/
/*

#define LABEL		1
#define IMAGE		2
#define CHECKBOX	3
#define BUTTON		4
#define PROGRESSBAR	5
#define VSCROLLBAR	6
#define TABCOLLECTION	7
#define TEXTFIELD	8
#define PWORDFIELD	9
#define MULTISELECT	10
#define SPINBUTTON	11

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
			return label_add_extended (winid, id, NULL, pos_x, pos_y, flags, size, r, g, b, text);
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
*/
