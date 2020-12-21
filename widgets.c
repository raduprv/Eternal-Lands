#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include <SDL_keycode.h>
#include "widgets.h"
#include "asc.h"
#include "chat.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "gamewin.h"
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
	unsigned char text[256];
	Uint16 fixed_width;
	Uint16 fixed_height;
	int center_offset;
}button;

typedef struct {
	float progress;
	GLfloat colors[12];
}progressbar;

typedef struct {
	unsigned char* password;
	int status;
	int max_chars;
	int cursor_pos;
	int draw_begin, draw_end;
	int sel_begin, sel_end;
	int drag_begin;
	int mouseover;
	float shadow_r, shadow_g, shadow_b;
	Uint16 fixed_height;
} password_entry;

typedef struct {
	unsigned char text[256];
	Uint16 x;
	Uint16 y;
	int width;
	int height;
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
	int scrollbar_width;
	int win_id;
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
int disable_double_click = 0;

// Forward declarations for widget types;
static int label_resize (widget_list *w, int width, int height);
static int free_widget_info (widget_list *widget);
static int checkbox_click(widget_list *W, int mx, int my, Uint32 flags);
static int button_draw(widget_list *w);
static int square_button_draw(widget_list *w);
static int button_change_font(widget_list *W, font_cat cat);
static int vscrollbar_click(widget_list *W, int mx, int my, Uint32 flags);
static int vscrollbar_drag(widget_list *W, int x, int y, Uint32 flags, int dx, int dy);
static int tab_collection_click(widget_list *W, int x, int y, Uint32 flags);
static int tab_collection_keypress(widget_list *W, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
static int free_tab_collection(widget_list *widget);
static int tab_collection_change_font(widget_list *w, font_cat font);
static int text_field_click(widget_list *w, int mx, int my, Uint32 flags);
static int text_field_drag(widget_list *w, int mx, int my, Uint32 flags, int dx, int dy);
static int text_field_resize (widget_list *w, int width, int height);
static int text_field_destroy(widget_list *w);
static int text_field_move(widget_list *w, int pos_x, int pos_y);
static int text_field_change_font(widget_list *w, font_cat cat);
static int text_field_paste(widget_list *w, const char* text);
static int text_field_set_color(widget_list *widget, float r, float g, float b);
static int pword_field_mouseover(widget_list *w, int mx, int my);
static int pword_field_click(widget_list *w, int mx, int my, Uint32 flags);
static int pword_field_drag(widget_list *w, int mx, int my, Uint32 flags, int dx, int dy);
static int pword_field_keypress(widget_list *w, int mx, int my, SDL_Keycode key_code,
	Uint32 key_unicode, Uint16 key_mod);
static int pword_field_draw(widget_list *w);
static int pword_field_paste(widget_list *w, const char* text);
static int multiselect_draw(widget_list *widget);
static int multiselect_click(widget_list *widget, int mx, int my, Uint32 flags);
static int multiselect_set_color(widget_list *widget, float r, float g, float b);
static int free_multiselect(widget_list *widget);
static int spinbutton_draw(widget_list *widget);
static int spinbutton_click(widget_list *widget, int mx, int my, Uint32 flags);
static int spinbutton_keypress(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

static const struct WIDGET_TYPE label_type = { NULL, label_draw, NULL, NULL, NULL, label_resize, NULL, free_widget_info, NULL, NULL, NULL, NULL };
static const struct WIDGET_TYPE image_type = { NULL, image_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info, NULL, NULL, NULL, NULL };
static const struct WIDGET_TYPE checkbox_type = { NULL, checkbox_draw, checkbox_click, NULL, NULL, NULL, NULL, free_widget_info, NULL, NULL, NULL, NULL };
static const struct WIDGET_TYPE round_button_type = { NULL, button_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info, NULL, button_change_font, NULL, NULL };
static const struct WIDGET_TYPE square_button_type = { NULL, square_button_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info, NULL, button_change_font, NULL, NULL };
static const struct WIDGET_TYPE progressbar_type = { NULL, progressbar_draw, NULL, NULL, NULL, NULL, NULL, free_widget_info, NULL, NULL, NULL, NULL };
static const struct WIDGET_TYPE vscrollbar_type = { NULL, vscrollbar_draw, vscrollbar_click, vscrollbar_drag, NULL, NULL, NULL, free_widget_info, NULL, NULL, NULL, NULL };
static const struct WIDGET_TYPE tab_collection_type = { NULL, tab_collection_draw, tab_collection_click, NULL, NULL, tab_collection_resize, (int (*)())tab_collection_keypress, free_tab_collection, NULL, tab_collection_change_font, NULL, NULL };
static const struct WIDGET_TYPE text_field_type = { NULL, text_field_draw, text_field_click, text_field_drag, NULL, text_field_resize, (int (*)())text_field_keypress, text_field_destroy, text_field_move, text_field_change_font, text_field_paste, text_field_set_color };
static const struct WIDGET_TYPE pword_field_type = { NULL, pword_field_draw, pword_field_click, pword_field_drag, pword_field_mouseover, NULL, (int (*)())pword_field_keypress, free_widget_info, NULL, NULL, pword_field_paste, NULL };
static const struct WIDGET_TYPE multiselect_type = { NULL, multiselect_draw, multiselect_click, NULL, NULL, NULL, NULL, free_multiselect, NULL, NULL, NULL, multiselect_set_color };
static const struct WIDGET_TYPE spinbutton_type = { NULL, spinbutton_draw, spinbutton_click, spinbutton_click, NULL, NULL, (int (*)())spinbutton_keypress, free_multiselect, NULL, NULL, NULL, NULL };

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
		int res = 1;
		w->pos_x = x;
		w->pos_y = y;
		if (w->type != NULL)
			if (w->type->move != NULL)
				res = w->type->move (w, x, y);
		return res;
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
	if(!w)
		return 0;

	w->r = r;
	w->g = g;
	w->b = b;
	if (w->type->color_change)
		w->type->color_change(w, r, g, b);
	return 1;
}

int widget_set_font_cat(int window_id, int widget_id, font_cat cat)
{
	widget_list *w = widget_find(window_id, widget_id);
	if (!w)
		return 0;

	w->fcat = cat;
	widget_handle_font_change(w, cat);

	return 1;
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

static int free_widget_info (widget_list *widget)
{
	free (widget->widget_info);
	return 1;
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
	Uint32 Flags, float size, const struct WIDGET_TYPE *type, void *T, void *S)
{
	window_info *win = &windows_list.window[window_id];
	widget_list *W = (widget_list *) malloc(sizeof(widget_list));
	widget_list *w = win->widgetlist;

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
	W->r = gui_color[0];
	W->g = gui_color[1];
	W->b = gui_color[2];
	W->fcat = win->font_category;
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
		win->widgetlist = W;
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

int widget_handle_keypress (widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	int res = 0;

	if (widget->type != NULL) {
		if (widget->type->key != NULL) {
			res = widget->type->key (widget, mx, my, key_code, key_unicode, key_mod);
		}
	}

	if (widget->OnKey != NULL && res != -1)
	{
		if(widget->spec != NULL) {
			res |= widget->OnKey (widget, mx, my, key_code, key_unicode, key_mod, widget->spec);
		} else {
			res |= widget->OnKey (widget, mx, my, key_code, key_unicode, key_mod);
		}
	}
	return res > -1 ? res : 0;
}

int widget_handle_font_change(widget_list *widget, font_cat cat)
{
	int res = 0;

	if (cat != widget->fcat)
		return 0;

	if (widget->type && widget->type->font_change)
	{
		res = widget->type->font_change(widget, cat);
	}
	if (widget->OnFontChange)
	{
		if (widget->spec)
		{
			res |= widget->OnFontChange(widget, cat, widget->spec);
		}
		else
		{
			res |= widget->OnFontChange(widget, cat);
		}
	}

	return res;
}

int widget_handle_paste(widget_list *widget, const char* text)
{
	int res = 0;
	if (widget->type && widget->type->paste)
	{
		res = widget->type->paste(widget, text);
	}

	// MAYBE FIXME? Add object-specific paste handlers?

	return res;
}

// --- End Common Widget Functions --->

// Label
int label_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint32 Flags, float size, const char *text)
{
	window_info *win = &windows_list.window[window_id];

	Uint16 len_x = get_string_width_zoom((const unsigned char*)text, win->font_category, size);
	Uint16 len_y = get_line_height(win->font_category, size);

	label *T = (label *) calloc (1, sizeof(label));
	safe_strncpy(T->text, text, sizeof(T->text));

	return widget_add (window_id, wid, OnInit, x, y, len_x, len_y, Flags, size, &label_type, (void *)T, NULL);
}

int label_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return label_add_extended (window_id, widget_id++, OnInit, x, y, 0, 1.0, text);
}

int label_draw(widget_list *W)
{
	label *l = (label *)W->widget_info;
	if(W->r != -1.0) {
		glColor3f(W->r,W->g,W->b);
	}
	draw_string_zoomed_width_font(W->pos_x, W->pos_y, (const unsigned char *)l->text,
		window_width, 1, W->fcat, W->size);
	return 1;
}

static int label_resize (widget_list *w, int width, int height)
{
	label *l;
	if (!w || !(l = (label*)w->widget_info))
		return 0;

	w->len_x = (width > 0) ? width : get_string_width_zoom((const unsigned char*)l->text,
		w->fcat, w->size);
	w->len_y = (height > 0) ? height : get_line_height(w->fcat, w->size);
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

int image_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int id, float u1, float v1, float u2, float v2, float alpha)
{
	image *T = calloc (1, sizeof (image));
	T->u1 = u1;
	T->u2 = u2;
	T->v1 = -v1;
	T->v2 = -v2;
	T->id = id;
	T->alpha = alpha;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &image_type, T, NULL);
}

int image_add(int window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2)
{
	return image_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, id, u1, v1, u2, v2, -1);
}

int image_draw(widget_list *W)
{
	image *i = (image *)W->widget_info;
	bind_texture(i->id);
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
		l->v1 = -v1;
		l->v2 = -v2;
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

static int checkbox_click(widget_list *W, int mx, int my, Uint32 flags)
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

int checkbox_add_extended(int window_id,  Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int *checked)
{
	checkbox *T = calloc (1, sizeof (checkbox));
	T->checked = checked;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &checkbox_type, T, NULL);
}

int checkbox_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int *checked)
{
	if(checked == NULL)
	{
		checked = calloc(1,sizeof(*checked));
	}
	return checkbox_add_extended(window_id, widget_id++, NULL, x, y, lx, ly, 0, 1.0, checked);
}

// Button
int safe_button_click(Uint32 *last_click)
{
	int retvalue = 0;
	if (disable_double_click || ((SDL_GetTicks() - *last_click) < 500))
		retvalue = 1;
	*last_click = SDL_GetTicks();
	return retvalue;
}

int calc_button_width(const unsigned char* label, font_cat cat, float size)
{
	return get_string_width_zoom(label, cat, size) + (int)(0.5 + 2*size*BUTTONRADIUS);
}

static int button_change_font(widget_list *W, font_cat cat)
{
	button *T;
	Uint16 len_x, len_y;

	if (!W || !(T = W->widget_info))
		return 0;

	len_x = T->fixed_width ? T->fixed_width : calc_button_width(T->text, W->fcat, W->size);
	if (T->fixed_height)
	{
		len_y = T->fixed_height;
	}
	else
	{
		// FIXME? Even if the font height changes, the button frame that is drawn only depends on
		// the size. So at least for now, stick to the button frame for the height.
// 		int min_len_y = (int)(2 * BUTTONRADIUS * W->size + 0.5);
// 		len_y = get_line_height(W->fcat, W->size) + (int)(12 * W->size + 0.5);
// 		if (len_y < min_len_y)
// 			len_y = min_len_y;
		len_y = (int)(2 * BUTTONRADIUS * W->size + 0.5);
	}

	if (W->Flags & BUTTON_VCENTER_CONTENT)
		T->center_offset = get_center_offset(T->text, strlen((const char*)T->text), W->fcat, W->size);

	return widget_resize(W->window_id, W->id, len_x, len_y);
}

int button_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, const char *text)
{
	window_info *win = &windows_list.window[window_id];

	Uint16 len_x, len_y;
	const struct WIDGET_TYPE *type = (Flags & BUTTON_SQUARE) ? &square_button_type : &round_button_type;

	button *T = calloc (1, sizeof(button));
	safe_strncpy((char*)T->text, text, sizeof(T->text));
	T->fixed_width = lx;
	T->fixed_height = ly;

	if (Flags & BUTTON_VCENTER_CONTENT)
	{
		T->center_offset = get_center_offset((const unsigned char*)text, strlen(text),
			win->font_category, size);
	}

	len_x = lx ? lx : calc_button_width(T->text, win->font_category, size);
	len_y = ly ? ly : get_line_height(win->font_category, size) + (int)(12 * size + 0.5);

	return widget_add (window_id, wid, OnInit, x, y, len_x, len_y, Flags, size, type, T, NULL);
}

int button_resize(int window_id, Uint32 wid, Uint16 lx, Uint16 ly, float size)
{
	widget_list *w = widget_find(window_id, wid);
	button *T;
	if (!w || !(T = w->widget_info))
		return 0;

	if (lx) T->fixed_width = lx;
	if (ly) T->fixed_height = ly;
	w->size = size;

	return button_change_font(w, w->fcat);
}

int button_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y)
{
	return button_add_extended(window_id, widget_id++, NULL, x, y, 0, 0, 0, 1.0, text);
}

static int button_draw(widget_list *W)
{
	button *l = (button *)W->widget_info;
	int text_width = W->len_x - 2*BUTTONRADIUS*W->size;
	draw_smooth_button(NULL, W->fcat, W->size, W->pos_x, W->pos_y, text_width,
		1, W->r, W->g, W->b, W->Flags & BUTTON_ACTIVE, gui_invert_color[0], gui_invert_color[1], gui_invert_color[2], 0.0f);
	draw_text(W->pos_x + W->len_x/2, W->pos_y + W->len_y/2 - l->center_offset, l->text,
		strlen((const char*)l->text), W->fcat, TDO_MAX_WIDTH, text_width, TDO_ALIGNMENT, CENTER,
		TDO_VERTICAL_ALIGNMENT, CENTER_LINE, TDO_ZOOM, W->size, TDO_SHRINK_TO_FIT, 1, TDO_END);
	return 1;
}

static int square_button_draw(widget_list *W)
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
	draw_text(W->pos_x + W->len_x/2, W->pos_y + W->len_y/2 - l->center_offset, l->text,
		strlen((const char*)l->text), W->fcat, TDO_ALIGNMENT, CENTER, TDO_VERTICAL_ALIGNMENT, CENTER_LINE,
		TDO_ZOOM, W->size, TDO_END);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

void draw_smooth_button(const unsigned char* str, font_cat fcat, float size,
	int x, int y, int w, int lines, float r, float g, float b,
	int highlight, float hr, float hg, float hb, float ha)
{
	int radius=lines*BUTTONRADIUS*size;

	glDisable(GL_TEXTURE_2D);

	if(r>=0.0f)
		glColor3f(r, g, b);

#ifdef OSX
	if (square_buttons) {
		glBegin(GL_LINE_LOOP);
		glVertex3i(x,y,0);
		glVertex3i(x + w + radius*2,y,0);
		glVertex3i(x + w + radius*2,y + radius*2,0);
		glVertex3i(x,y + radius*2,0);
		glEnd();

		if(highlight) {
			if(hr>=0.0f)
				glColor4f(hr,hg,hb,ha);
			glBegin(GL_POLYGON);
			glVertex3i(x+1,y+1,0);
			glVertex3i(x + w + radius*2 -1,y+1,0);
			glVertex3i(x + w + radius*2 -1,y + radius*2 -1,0);
			glVertex3i(x+1,y + radius*2 -1,0);
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
	} else {
#endif
	glBegin(GL_LINE_LOOP);
		draw_circle_ext(x, y, radius, 10, 90, 270);
		draw_circle_ext(x+w, y, radius, 10, -90, 90);
	glEnd();
	if(highlight) {
		if(hr>=0.0f)
			glColor4f(hr,hg,hb,ha);
		glBegin(GL_POLYGON);
			draw_circle_ext(x+1, y+1, radius-1, 10, 90, 270);
			draw_circle_ext(x+w+1, y+1, radius-1, 10, -90, 90);
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);

#ifdef OSX
	}	// to close off square_buttons conditional
#endif

	if(highlight) {
		glColor3f(r, g, b);
	}

	if (str)
	{
		draw_text(x + radius + w/2, y + radius, str, strlen((const char*)str), fcat,
			TDO_MAX_WIDTH, w, TDO_ALIGNMENT, CENTER, TDO_VERTICAL_ALIGNMENT, CENTER_LINE,
			TDO_ZOOM, size, TDO_SHRINK_TO_FIT, 1, TDO_END);
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


int button_set_text(int window_id, Uint32 widget_id, const char *text)
{
	widget_list *w = widget_find(window_id, widget_id);
	if(w){
		button *l = (button *) w->widget_info;
		safe_strncpy((char*)l->text, text, sizeof(l->text));
		return 1;
	}
	return 0;
}

// Progressbar
int progressbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return progressbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 0.0f, NULL);
}

int progressbar_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float progress, const float * colors)
{
	progressbar *T = calloc (1, sizeof(progressbar));
	T->progress = progress;
	if (colors) {
		memcpy(T->colors, colors, sizeof(T->colors));
	} else {
		T->colors[0] = -1.0f;
	}

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &progressbar_type, T, NULL);
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
			glColor3fv(gui_color);
		glEnd();
	}
	if(W->r != -1.0)
		glColor3f(W->r,W->g,W->b);
	else
		glColor3fv(gui_color);

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
	int arrow_size = (int)(0.5 + (float)(W->len_x) / 4.0f);

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
	glVertex3i(W->pos_x + arrow_size, W->pos_y + 2 * arrow_size,0);
	glVertex3i(W->pos_x + 2 * arrow_size, W->pos_y + arrow_size,0);
	glVertex3i(W->pos_x + 2 * arrow_size, W->pos_y + arrow_size,0);
	glVertex3i(W->pos_x + 3 * arrow_size, W->pos_y + 2 * arrow_size,0);
	glVertex3i(W->pos_x + arrow_size, W->pos_y + W->len_y - 2 * arrow_size,0);
	glVertex3i(W->pos_x + 2 * arrow_size, W->pos_y + W->len_y - arrow_size,0);
	glVertex3i(W->pos_x + 2 * arrow_size, W->pos_y + W->len_y - arrow_size,0);
	glVertex3i(W->pos_x + 3 * arrow_size, W->pos_y + W->len_y - 2 * arrow_size,0);
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
	glVertex3i(W->pos_x + 2 * arrow_size - (int)(0.5 + (float)arrow_size / 1.5f), W->pos_y + 3 * arrow_size + (c->pos * ((float)(W->len_y -11 * arrow_size) / drawn_bar_len)), 0);
	glVertex3i(W->pos_x + 2 * arrow_size + (int)(0.5 + (float)arrow_size / 1.5f), W->pos_y + 3 * arrow_size + (c->pos * ((float)(W->len_y -11 * arrow_size) / drawn_bar_len)), 0);
	glVertex3i(W->pos_x + 2 * arrow_size + (int)(0.5 + (float)arrow_size / 1.5f), W->pos_y + 8 * arrow_size + (c->pos * ((float)(W->len_y -11 * arrow_size) / drawn_bar_len)), 0);
	glVertex3i(W->pos_x + 2 * arrow_size - (int)(0.5 + (float)arrow_size / 1.5f), W->pos_y + 8 * arrow_size + (c->pos * ((float)(W->len_y -11 * arrow_size) / drawn_bar_len)), 0);
	glEnd();

	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}

int vscrollbar_click(widget_list *W, int mx, int my, Uint32 flags)
{
	int arrow_size = (int)(0.5 + (float)(W->len_x) / 4.0f);
	vscrollbar *b = (vscrollbar *)W->widget_info;
	if ( my < 3*arrow_size || (flags & ELW_WHEEL_UP) )
	{
		b->pos -= b->pos_inc;
	}
	else if (my > W->len_y - 3*arrow_size || (flags & ELW_WHEEL_DOWN) )
	{
		b->pos += b->pos_inc;
	}
	else
	{
		b->pos = (my - 5*arrow_size)/((float)(W->len_y-11*arrow_size)/b->bar_len);
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

static int vscrollbar_drag(widget_list *W, int x, int y, Uint32 flags, int dx, int dy)
{
	window_info *win;

	if (!W)
		return 0;

	vscrollbar_click(W, x, y, flags);

	/* the drag event can happen multiple times for each redraw as its done in the event loop
	 * so update the scroll bar position each time to avoid positions glitches */
	win = &windows_list.window[W->window_id];
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

int vscrollbar_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int pos, int pos_inc, int bar_len)
{
	vscrollbar *T = calloc (1, sizeof(vscrollbar));
	T->pos_inc = pos_inc;
	T->pos = pos;
	T->bar_len = bar_len > 0 ? bar_len : 0;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &vscrollbar_type, T, NULL);
}

int vscrollbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return vscrollbar_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 0, 1, ly);
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

int tab_collection_calc_tab_height(font_cat cat, float size)
{
	return 2 * get_line_height(cat, size);
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

static int free_tab_collection(widget_list *widget)
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
	int xtxt, ytxt;
	float tab_corner;

	if (!w) return 0;

	col = (tab_collection *) w->widget_info;

	h = col->tag_height;
	ytagtop = w->pos_y;
	ytagbot = w->pos_y + h;
	xstart = w->pos_x;
	btn_size = col->button_size;
	arw_width = btn_size / 4;
	cur_start = cur_end = xstart;
	tab_corner = h / 5.0f;

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
				glVertex3f (xstart - 2 * tab_corner, ytagtop, 0);
				glVertex3f (w->pos_x + w->len_x - h, ytagtop, 0);
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
			// current
			glBegin(GL_LINE_STRIP);
				glVertex3f(xstart, ytagbot, 0);
				glVertex3f(xstart, ytagtop + tab_corner, 0);
				glVertex3f(xstart + tab_corner, ytagtop, 0);
				glVertex3f(xend - tab_corner, ytagtop, 0);
				glVertex3f(xend, ytagtop + tab_corner, 0);
				glVertex3f(xend, ytagbot, 0);
			glEnd();
		} else if(col->cur_tab>itab){
			// left of current
			glBegin (GL_LINE_STRIP);
				glVertex3f(xstart, ytagbot, 0);
				glVertex3f(xstart, ytagtop + tab_corner, 0);
				glVertex3f(xstart + tab_corner, ytagtop, 0);
				glVertex3f(xend + tab_corner, ytagtop, 0);
			glEnd ();
		} else {
			// right of current
			glBegin (GL_LINE_STRIP);
				glVertex3f(xend, ytagbot, 0);
				glVertex3f(xend, ytagtop + tab_corner, 0);
				glVertex3f(xend - tab_corner, ytagtop, 0);
				glVertex3f (xstart - tab_corner, ytagtop, 0);
			glEnd ();
		}

		// draw a close box if necessary
		if (col->tabs[itab].closable)
		{
			glBegin (GL_LINE_LOOP);
			glVertex3i (xstart + tab_corner, ytagbot - tab_corner, 0);
			glVertex3i (xstart + tab_corner, ytagtop + tab_corner, 0);
			glVertex3i (xstart + h - tab_corner, ytagtop + tab_corner, 0);
			glVertex3i (xstart + h - tab_corner, ytagbot - tab_corner, 0);
			glEnd ();

			glBegin (GL_LINES);
			glVertex3i (xstart + tab_corner, ytagbot - tab_corner, 0);
			glVertex3i (xstart + h - tab_corner, ytagtop + tab_corner, 0);
			glVertex3i (xstart + tab_corner, ytagtop + tab_corner, 0);
			glVertex3i (xstart + h - tab_corner, ytagbot - tab_corner, 0);
			glEnd ();
		}

		glEnable(GL_TEXTURE_2D);

		if (col->tabs[itab].label_r >= 0.0f)
			glColor3f (col->tabs[itab].label_r, col->tabs[itab].label_g, col->tabs[itab].label_b);

		xtxt = xstart + (w->size * DEFAULT_FIXED_FONT_WIDTH) / 2;
		ytxt = ytagtop + h/2;
		if (col->tabs[itab].closable)
			xtxt += h;
		draw_text(xtxt, ytxt, col->tabs[itab].label, strlen((const char*)col->tabs[itab].label),
			w->fcat, TDO_ZOOM, w->size, TDO_VERTICAL_ALIGNMENT, CENTER_LINE, TDO_END);

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

static int tab_collection_click(widget_list *W, int x, int y, Uint32 flags)
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
			int tab_corner = col->tag_height / 5;

			// check if close box was clicked
			if (col->tabs[itag].closable && x > x_start + tab_corner &&
				x < x_start + col->tag_height - tab_corner &&
				y > tab_corner && y < col->tag_height - tab_corner)
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

static int calc_tag_width(const unsigned char* label, font_cat fcat, float size, int close_width)
{
	return size * DEFAULT_FIXED_FONT_WIDTH
			+ get_string_width_zoom(label, fcat, size)
			+ close_width;
}

static int tab_collection_change_font(widget_list *w, font_cat font)
{
	tab_collection *col;
	int itab;

	if (!w || !(col = (tab_collection *)w->widget_info))
		return 0;

	col->tag_height = tab_collection_calc_tab_height(w->fcat, w->size);
	col->button_size = (9 * col->tag_height) / 10;

	for (itab = 0; itab < col->nr_tabs; ++itab)
	{
		Uint16 width = calc_tag_width(col->tabs[itab].label, w->fcat, w->size,
			col->tabs[itab].closable ? col->tag_height : 0);
		if (width > col->tabs[itab].min_tag_width)
			col->tabs[itab].tag_width = width;
	}

	return 1;
}

int tab_collection_resize (widget_list *w, Uint32 width, Uint32 height)
{
	tab_collection *col;
	int itab;

	if (w == NULL || (col = (tab_collection *) w->widget_info) == NULL)
		return 0;

	tab_collection_change_font(w, w->fcat);

	for (itab = 0; itab < col->nr_tabs; itab++)
		resize_window (col->tabs[itab].content_id, width, height);

	return 1;
}

int tab_collection_move (widget_list *W, Uint32 pos_x, Uint32 pos_y)
{
	tab_collection *col;
	int itab;

	if (W == NULL || (col = (tab_collection *) W->widget_info) == NULL)
		return 0;

	for (itab = 0; itab < col->nr_tabs; itab++)
		move_window(col->tabs[itab].content_id, W->window_id, 0, pos_x, pos_y);

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

static int tab_collection_keypress(widget_list *W, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	int shift_on = key_mod & KMOD_SHIFT;
	tab_collection *col = (tab_collection *) W->widget_info;

	if (col->nr_tabs <= 0) return 0;

	if (shift_on && KEY_DEF_CMP(K_ROTATERIGHT, key_code, key_mod))
	{
		if (col->nr_tabs == 1) return 1;
		hide_window (col->tabs[col->cur_tab].content_id);
		if (++col->cur_tab >= col->nr_tabs)
			col->cur_tab = 0;
		_tab_collection_make_cur_visible (W);
		return 1;
	}
	else if (shift_on && KEY_DEF_CMP(K_ROTATELEFT, key_code, key_mod))
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

int tab_collection_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int max_tabs)
{
	int itab;
	window_info *win = &windows_list.window[window_id];
	tab_collection *T;

	T = calloc (1, sizeof (tab_collection));
	T->max_tabs =  max_tabs <= 0 ? 2 : max_tabs;
	T->tabs = calloc (T->max_tabs, sizeof (tab));
	// initialize all tabs content ids to -1 (unitialized window_
	for (itab = 0; itab < T->max_tabs; itab++)
		T->tabs[itab].content_id = -1;
	T->nr_tabs = 0;
	T->tag_height = tab_collection_calc_tab_height(win->font_category, size);
	T->button_size = (9 * T->tag_height) / 10;
	T->cur_tab = 0;
	T->tab_offset = 0;
	T->tab_last_visible = 0;

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &tab_collection_type, T, NULL);
}

int tab_collection_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly)
{
	return tab_collection_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, 0, 1.0, 0);
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

	safe_strncpy((char*)col->tabs[nr].label, label, sizeof (col->tabs[nr].label));
	col->tabs[nr].content_id = create_window ("", window_id, 0, w->pos_x, w->pos_y + col->tag_height, w->len_x, w->len_y - col->tag_height, ELW_TITLE_NONE|flags);
	col->tabs[nr].closable = closable ? 1 : 0;

	if (tag_width > 0)
	{
		col->tabs[nr].tag_width = col->tabs[nr].min_tag_width = tag_width;
	}
	else
	{
		// compute tag width from label width
		col->tabs[nr].min_tag_width = 0;
		col->tabs[nr].tag_width = calc_tag_width(col->tabs[nr].label, w->fcat, w->size,
			col->tabs[nr].closable ? col->tag_height : 0);
	}

	// set label color to default values
	col->tabs[nr].label_r = -1.0f;
	col->tabs[nr].label_g = -1.0f;
	col->tabs[nr].label_b = -1.0f;

	return col->tabs[nr].content_id;
}

// text field
static void _text_field_set_nr_visible_lines (widget_list *w)
{
	text_field* tf = w->widget_info;

	if (tf != NULL/* && (w->Flags & TEXT_FIELD_EDITABLE)*/)
	{
		tf->nr_visible_lines = get_max_nr_lines(w->len_y - 2*tf->y_space, w->fcat,
			tf->buffer[tf->msg].wrap_zoom);
		if (tf->nr_visible_lines < 0)
			tf->nr_visible_lines = 0;
	}
}

static void _text_field_set_nr_lines (widget_list *w, int nr_lines)
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

static void _text_field_find_cursor_line(text_field* tf)
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

static void _text_field_set_cursor_line_only(text_field* tf)
{
	int i;
	const text_message* msg = &tf->buffer[tf->msg];
	tf->cursor_line = 0;
	for (i = 0; i < tf->cursor; ++i)
	{
		if (msg->data[i] == '\n' || msg->data[i] == '\r')
			++tf->cursor_line;
	}
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
			else if (is_printable(ch))
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
	_text_field_find_cursor_line(tf);
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

void _text_field_cursor_left(widget_list *w, int skipword)
{
	text_field *tf = w->widget_info;
	text_message* msg;
	int i;

	if (tf == NULL || tf->cursor <= 0)
		return;

	msg = &(tf->buffer[tf->msg]);
	i = tf->cursor - 1;
	if (i > 0 && msg->data[i] == '\r')
		--i;
	if (skipword)
	{
		while (i > 0 && isspace(msg->data[i]))
			--i;
		while (i > 0 && !isspace(msg->data[i-1]))
			--i;
	}
	tf->cursor = i;
	_text_field_set_cursor_line_only(tf);

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);
}

void _text_field_cursor_right(widget_list *w, int skipword)
{
	text_field *tf = w->widget_info;
	text_message* msg;
	int i;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);
	if (tf->cursor >= msg->len)
		return;

	i = tf->cursor + 1;
	if (i < msg->len && msg->data[i] == '\r')
		++i;
	if (skipword)
	{
		while (i < msg->len && !isspace(msg->data[i]))
			++i;
		while (i < msg->len && isspace(msg->data[i]))
			++i;
	}
	tf->cursor = i;
	_text_field_set_cursor_line_only(tf);

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
	int prev_line_end;   // 1 past last character of previous line

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
	prev_line_end = msg->data[line_start-1] == '\r' ? line_start - 1 : line_start;

	// Now find where the previous line starts
	for (prev_line_start = line_start-1; prev_line_start > 0; prev_line_start--)
		if (msg->data[prev_line_start-1] == '\r' || msg->data[prev_line_start-1] == '\n')
			break;

	tf->cursor = min2i(prev_line_start + cursor_offset, prev_line_end - 1);
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
	int next_line_end;   // Index of last character on line after the line with the cursor
	int cursor_offset;   // Position of the cursor on this line

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
	++next_line_start;
	// Find where the next line ends
	for (next_line_end = next_line_start; next_line_end < msg->len; next_line_end++)
	{
		if (msg->data[next_line_end] == '\r')
		{
			--next_line_end;
			break;
		}
		if (msg->data[next_line_end] == '\n')
			break;
	}

	tf->cursor = min2i(next_line_start + cursor_offset, next_line_end);
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
	for (i = tf->cursor; i < msg->len; ++i)
	{
		if (msg->data[i] == '\n' || (i+1 < msg->len && msg->data[i+1] == '\r'))
			break;
	}

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

void _text_field_delete_backward(widget_list * w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int ni, nr_lines;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);
	ni = tf->cursor - 1;
	while (ni > 0 && msg->data[ni] == '\r')
		--ni;

	memmove(msg->data + ni, msg->data + tf->cursor, msg->len - tf->cursor + 1);
	msg->len -= tf->cursor - ni;
	tf->cursor = ni;

	// set invalid width to force rewrap
	msg->wrap_width = 0;
	nr_lines = rewrap_message(msg, w->fcat, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width,
		&tf->cursor);
	_text_field_set_cursor_line_only(tf);
	_text_field_set_nr_lines(w, nr_lines);

	if (tf->scroll_id != -1)
		_text_field_scroll_to_cursor (w);

}

void _text_field_delete_forward(widget_list *w)
{
	text_field *tf = w->widget_info;
	text_message *msg;
	int ni, nr_lines;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);
	ni = tf->cursor + 1;
	while (ni < msg->len && msg->data[ni] == '\r')
		++ni;

	memmove(msg->data + tf->cursor, msg->data + ni, msg->len - ni + 1);
	msg->len -= ni - tf->cursor;

	// set invalid width to force rewrap
	msg->wrap_width = 0;
	nr_lines = rewrap_message(msg, w->fcat, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width,
		&tf->cursor);
	_text_field_set_cursor_line_only(tf);
	_text_field_set_nr_lines (w, nr_lines);
}

void _text_field_insert_char (widget_list *w, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint8 ch = key_to_char (key_unicode);
	text_field *tf = w->widget_info;
	text_message *msg;
	int nr_lines;

	if (tf == NULL)
		return;

	msg = &(tf->buffer[tf->msg]);

	if (key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER)
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

	// set invalid width to force rewrap
	msg->wrap_width = 0;
	nr_lines = rewrap_message(msg, w->fcat, w->size, w->len_x - 2*tf->x_space - tf->scrollbar_width,
		&tf->cursor);
	_text_field_set_cursor_line_only(tf);
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

static int text_field_resize (widget_list *w, int width, int height)
{
	text_field *tf = w->widget_info;

	if (tf != NULL)
	{
		if (tf->scroll_id != -1)
		{
			tf->scrollbar_width = w->size * ELW_BOX_SIZE;
			widget_resize (w->window_id, tf->scroll_id, tf->scrollbar_width, height);
			widget_move (w->window_id, tf->scroll_id, w->pos_x + width - tf->scrollbar_width, w->pos_y);
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
				nr_lines += rewrap_message(tf->buffer+i, w->fcat, w->size,
					width - tf->scrollbar_width, cursor);
			}
			_text_field_set_nr_lines (w, nr_lines);

			_text_field_find_cursor_line (tf);
		}
	}

	return 1;
}

static int text_field_move(widget_list *w, int pos_x, int pos_y)
{
	if (w)
	{
		text_field *tf = w->widget_info;
		if (tf && tf->scroll_id != -1)
			widget_move (w->window_id, tf->scroll_id, w->pos_x + w->len_x - tf->scrollbar_width, w->pos_y);
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
		text_field_paste(w, thestring);
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
		case 0: text_field_keypress(w, 0, 0, K_CUT.key_code, 0, K_CUT.key_mod); break;
		case 1: text_field_keypress(w, 0, 0, K_COPY.key_code, 0, K_COPY.key_mod); break;
		case 2:
			if (!text_field_keypress(w, 0, 0, K_PASTE.key_code, 0, K_PASTE.key_mod))
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
				text_field_paste(w, str);
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
					text_field_paste(w, str);
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

int text_field_keypress(widget_list *w, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint8 ch = key_to_char(key_unicode);
	text_field *tf;
	text_message *msg;
	int alt_on = key_mod & KMOD_ALT, ctrl_on = key_mod & KMOD_CTRL;
	int shift_on = key_mod & KMOD_SHIFT;

	if (w == NULL) return 0;
	tf = w->widget_info;

	if (KEY_DEF_CMP(K_COPY, key_code, key_mod) || KEY_DEF_CMP(K_COPY_ALT, key_code, key_mod))
	{
		_text_field_copy_to_clipboard (tf);
		return 1;
	}

	if ( !(w->Flags & TEXT_FIELD_EDITABLE) ) return 0;
	if (w->Flags & TEXT_FIELD_NO_KEYPRESS) return 0;

	msg = &(tf->buffer[tf->msg]);

	if (is_printable (ch) || key_code == SDLK_UP || key_code == SDLK_DOWN ||
		key_code == SDLK_LEFT || key_code == SDLK_RIGHT || key_code == SDLK_HOME ||
		key_code == SDLK_END || key_code == SDLK_BACKSPACE || key_code == SDLK_DELETE)
	{
		/* Stop blinking on input */
		tf->next_blink = cur_time + TF_BLINK_DELAY;
	}

	if ((key_code == SDLK_LEFT) || (key_code == SDLK_RIGHT) || (key_code == SDLK_UP) || (key_code == SDLK_DOWN) ||
		(key_code == SDLK_HOME) || (key_code == SDLK_END))
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

	if (key_code == SDLK_LEFT)
	{
		_text_field_cursor_left (w, ctrl_on);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (key_code == SDLK_RIGHT)
	{
		_text_field_cursor_right (w, ctrl_on);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (key_code == SDLK_UP && !ctrl_on && !alt_on && tf->cursor >= 0)
	{
		_text_field_cursor_up (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (key_code == SDLK_DOWN && !ctrl_on && !alt_on && tf->cursor >= 0)
	{
		_text_field_cursor_down (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (key_code == SDLK_HOME)
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
	else if (key_code == SDLK_END)
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
	else if (key_code == SDLK_PAGEUP)
	{
		_text_field_cursor_page_up (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if (key_code == SDLK_PAGEDOWN)
	{
		_text_field_cursor_page_down (w);
		if (shift_on) update_cursor_selection(w, 1);
		return 1;
	}
	else if ((key_code == SDLK_BACKSPACE || key_code == SDLK_DELETE
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
        else if (key_code == SDLK_BACKSPACE || ch == 127)
#else
        else if (key_code == SDLK_BACKSPACE)
#endif
	{
		if (tf->cursor > 0)
			_text_field_delete_backward (w);
		return 1;
	}
	else if (key_code == SDLK_DELETE)
	{
		if (tf->cursor < msg->len)
			_text_field_delete_forward (w);
		return 1;
	}
	else if (KEY_DEF_CMP(K_CUT, key_code, key_mod))
	{
		_text_field_copy_to_clipboard(tf);
		if (!TEXT_FIELD_SELECTION_EMPTY(&tf->select))
		{
			text_field_remove_selection(tf);
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		}
		return 1;
	}
	else if (KEY_DEF_CMP(K_PASTE, key_code, key_mod) || KEY_DEF_CMP(K_PASTE_ALT, key_code, key_mod))
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
			|| ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && !(w->Flags&TEXT_FIELD_IGNORE_RETURN)) ) && ch != '`' )
	{
		if (!TEXT_FIELD_SELECTION_EMPTY(&tf->select))
		{
			text_field_remove_selection(tf);
			TEXT_FIELD_CLEAR_SELECTION(&tf->select);
		}
		_text_field_insert_char (w, key_code, key_unicode, key_mod);
		return 1;
	}
	return 0;
}

void _set_edit_pos (text_field* tf, int x, int y, font_cat fcat)
{
	unsigned int i = tf->offset;
	unsigned int nrlines = 0, line = 0;
	int px = 0;
	text_message* msg = &(tf->buffer[tf->msg]);
	int line_skip = get_line_skip(fcat, msg->wrap_zoom);

	if (msg->len == 0)
		return;	// nothing to do, there is no string

	nrlines = y / line_skip;
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
				tf->cursor = i-1;
				return;
			case '\n':
			case '\0':
				tf->cursor = i;
				return;
			default:
				px += get_char_width_zoom(msg->data[i], fcat, msg->wrap_zoom);
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
	text_field* tf;
	text_message* msg;

	tf = w->widget_info;
	if (tf == NULL) return;

	line = y / get_line_skip(w->fcat, w->size);
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
		cx += get_char_width_zoom(msg->data[col], w->fcat, w->size);
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

static int text_field_click(widget_list *w, int mx, int my, Uint32 flags)
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

	_set_edit_pos(tf, mx, my, w->fcat);

#if !defined OSX && !defined WINDOWS
#ifdef MIDDLE_MOUSE_PASTE
	if (flags & ELW_MID_MOUSE)
		start_paste_from_primary(w);
#endif // MIDDLE_MOUSE_PASTE
#endif

	return 1;
}

static int text_field_drag(widget_list *w, int mx, int my, Uint32 flags, int dx, int dy)
{
	update_selection(mx, my, w, 1);
	return 1;
}

static int text_field_destroy(widget_list *w)
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

static int text_field_set_color(widget_list *widget, float r, float g, float b)
{
	text_field *tf = widget->widget_info;
	if (!tf)
		return 0;

	if (tf->scroll_id != -1)
		widget_set_color(widget->window_id, tf->scroll_id, r, g, b);
	return 1;
}

int text_field_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y,
	Uint16 lx, Uint16 ly, Uint32 Flags, font_cat fcat, float size, text_message *buf, int buf_size,
	Uint8 chan_filt, int x_space, int y_space)
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
		T->scrollbar_width = size * ELW_BOX_SIZE;
		T->scroll_id = vscrollbar_add_extended (window_id, widget_id++, NULL, x + lx-T->scrollbar_width, y, T->scrollbar_width, ly, 0, size, 0, 1, 1);
	}
	else
	{
		T->scroll_id = -1;
		T->scrollbar_width = 0;
	}

	res = widget_add (window_id, wid, OnInit, x, y, lx, ly, Flags, size, &text_field_type, T, NULL);
	// In addition to setting the font category, the call to widget_set_font_cat() also sets
	// the number of visible lines, and allocates T->select.lines with the correct number of lines.
	widget_set_font_cat(window_id, wid, fcat);
	T->select.sm = T->select.em = T->select.sc = T->select.ec = -1;

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
	return text_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly,
		TEXT_FIELD_BORDER, 0, 1.0, buf, buf_size, FILTER_ALL, x_space, y_space);
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
			nr_lines = rewrap_message (&tf->buffer[tf->msg], w->fcat, w->size,
				w->len_x - 2*tf->x_space - tf->scrollbar_width, &tf->cursor);
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

	for (i = 0; i < tf->nr_visible_lines; i++)
		tf->select.lines[i].msg = -1;
	draw_messages(w->pos_x + tf->x_space, w->pos_y + tf->y_space,
		tf->buffer, tf->buf_size, tf->chan_nr, tf->msg, tf->offset, cursor,
		w->len_x - 2*tf->x_space - tf->scrollbar_width, w->len_y - 2 * tf->y_space,
		w->fcat, w->size, &tf->select);
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

int text_field_clear(int window_id, Uint32 widget_id)
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

static int text_field_change_font(widget_list *w, font_cat cat)
{
	text_field *tf;
	int i, nr_lines = 0, nr_vis;

	if (!w || !(tf = w->widget_info))
		return 0;

	for (i = 0; i < tf->buf_size; i++)
	{
		int *cursor = i == tf->msg ? &(tf->cursor) : NULL;
		tf->buffer[i].wrap_width = 0;
		nr_lines += rewrap_message(tf->buffer+i, w->fcat, w->size,
			w->len_x - 2*tf->x_space - tf->scrollbar_width, cursor);
	}

	nr_vis = tf->nr_visible_lines;
	_text_field_set_nr_visible_lines(w);
	_text_field_set_nr_lines(w, nr_lines);
	if (tf->nr_visible_lines != nr_vis)
		tf->select.lines = realloc(tf->select.lines, tf->nr_visible_lines * sizeof(text_field_line));

	return 1;
}

static int text_field_paste(widget_list *w, const char* text)
{
	text_field *tf;
	int bytes = strlen(text);
	text_message* msg;
	int p;

	if (!w || !(tf = w->widget_info))
		return 0;

	// if not editable, don't allow paste
	if (!(w->Flags & TEXT_FIELD_EDITABLE))
		return 0;

	msg = &tf->buffer[tf->msg];

	// if can't grow and would over fill, just use what we can
	if ((msg->len + bytes >= msg->size) && !(w->Flags & TEXT_FIELD_CAN_GROW))
		bytes = msg->size - msg->len - 1;

	resize_text_message_data (msg, msg->len + bytes);

	p = tf->cursor;
	memmove (&msg->data[p + bytes], &msg->data[p], msg->len - p + 1);
	memcpy (&msg->data[p], text, bytes);
	msg->len += bytes;
	tf->cursor += bytes;
	_text_field_find_cursor_line (tf);

	return 1;
}

//password entry field. We act like a restricted text entry with multiple modes

static void pword_update_draw_range_right(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	const unsigned char* pw = entry->password;
	int len = strlen((const char*)pw);
	int str_width;

	if (entry->status == P_TEXT)
	{
		str_width = get_buf_width_zoom(pw + entry->draw_begin,
			entry->draw_end - entry->draw_begin, cat, size);
		if (str_width > max_width)
		{
			while (entry->draw_end > entry->draw_begin && str_width > max_width)
				str_width -= get_char_width_zoom(pw[--entry->draw_end], cat, size);
		}
		else
		{
			while (entry->draw_end < len)
			{
				int char_width = get_char_width_zoom(pw[entry->draw_end], cat, size);
				if (str_width + char_width > max_width)
					break;
				str_width += char_width;
				++entry->draw_end;
			}
		}
	}
	else if (entry->status == P_NORMAL)
	{
		int sw = get_char_width_zoom('*', cat, size);
		str_width = (entry->draw_end - entry->draw_begin) * sw;
		if (str_width > max_width)
		{
			while (entry->draw_end > entry->draw_begin && str_width > max_width)
			{
				--entry->draw_end;
				str_width -= sw;
			}
		}
		else
		{
			while (entry->draw_end < len && str_width + sw <= max_width)
			{
				++entry->draw_end;
				str_width += sw;
			}
		}
	}
}

static void pword_check_after_left_move(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	if (entry->status == P_NONE || entry->cursor_pos >= entry->draw_begin)
		return;

	entry->draw_begin = entry->cursor_pos;
	pword_update_draw_range_right(entry, cat, size, max_width);
}

static void pword_update_draw_range_left(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	const unsigned char* pw = entry->password;
	int len = strlen((const char*)pw);
	int str_width;

	if (entry->status == P_TEXT)
	{
		str_width = get_buf_width_zoom(pw + entry->draw_begin,
			entry->draw_end - entry->draw_begin, cat, size);
		if (entry->cursor_pos == len)
			str_width += get_char_width_zoom('_', cat, size);
		if (str_width > max_width)
		{
			while (entry->draw_end > entry->draw_begin && str_width > max_width)
				str_width -= get_char_width_zoom(pw[entry->draw_begin++], cat, size);
		}
		else
		{
			while (entry->draw_begin > 0)
			{
				int char_width = get_char_width_zoom(pw[entry->draw_begin-1], cat, size);
				if (str_width + char_width > max_width)
					break;
				str_width += char_width;
				--entry->draw_begin;
			}
		}
	}
	else if (entry->status == P_NORMAL)
	{
		int sw = get_char_width_zoom('*', cat, size);
		str_width = (entry->sel_end - entry->sel_begin) * sw;
		if (entry->cursor_pos == len)
			str_width += get_char_width_zoom('_', cat, size);
		if (str_width > max_width)
		{
			while (entry->draw_end > entry->draw_begin && str_width > max_width)
			{
				++entry->draw_begin;
				str_width -= sw;
			}
		}
		else
		{
			while (entry->draw_begin > 0 && str_width + sw <= max_width)
			{
				--entry->draw_begin;
				str_width += sw;
			}
		}
	}
}

static void pword_check_after_right_move(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	int len;

	if (entry->status == P_NONE || entry->cursor_pos < entry->draw_end)
		return;

	len = strlen((const char*)entry->password);
	entry->draw_end = min2i(len, entry->cursor_pos+1);
	pword_update_draw_range_left(entry, cat, size, max_width);
}

static void pword_update_after_delete(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	int len = strlen((const char*)entry->password);
	entry->draw_begin = min2i(entry->draw_begin, entry->cursor_pos);
	entry->draw_end = min2i(len, entry->cursor_pos+1);

	pword_update_draw_range_right(entry, cat, size, max_width);
	if (entry->draw_end == len)
		pword_update_draw_range_left(entry, cat, size, max_width);
}

static void pword_update_draw_range_after_insert(password_entry *entry, font_cat cat,
	float size, int max_width)
{
	int len = strlen((const char*)entry->password);
	if (entry->draw_end > len)
		entry->draw_end = len;
	pword_update_draw_range_right(entry, cat, size, max_width);
	if (entry->cursor_pos >= entry->draw_end)
	{
		entry->draw_end = min2i(len, entry->cursor_pos+1);
		pword_update_draw_range_left(entry, cat, size, max_width);
	}
}

static char* pword_get_selected_text(const password_entry *entry)
{
	int size, idst, isrc;
	char *res;

	if (entry->sel_begin < 0 || entry->sel_end <= entry->sel_begin)
		return NULL;

	size = entry->sel_end - entry->sel_begin + 1;
	res = calloc(size, 1);
	if (!res)
		return NULL;

	for (idst = 0, isrc = entry->sel_begin; isrc < entry->sel_end; ++isrc)
	{
		unsigned char ch = entry->password[isrc];
		if (is_printable(ch) || ch == '\n')
			res[idst++] = ch;
	}

	return res;
}

static void pword_delete(password_entry *entry, int begin, int end, font_cat cat,
	float size, int max_width)
{
	unsigned char* pw = entry->password;
	int len = strlen((const char*)pw);
	memmove(pw + begin, pw + end, len - end + 1);
	entry->cursor_pos = begin;
	pword_update_after_delete(entry, cat, size, max_width);
}

static void pword_insert(password_entry *entry, int pos, const unsigned char* text, int len,
	font_cat cat, float size, int max_width)
{
	unsigned char* pw = entry->password;
	if (pos + len >= entry->max_chars)
	{
		memcpy(pw + pos, text, entry->max_chars - pos);
	}
	else
	{
		memmove(pw + pos + len, pw + pos, entry->max_chars - pos - len);
		memcpy(pw + pos, text, len);
	}
	pw[entry->max_chars-1] = '\0';
	entry->cursor_pos = min2i(pos + len, entry->max_chars);
	pword_update_draw_range_after_insert(entry, cat, size, max_width);
}

static int pword_field_keypress(widget_list *w, int mx, int my, SDL_Keycode key_code,
	Uint32 key_unicode, Uint16 key_mod)
{
	unsigned char ch = key_to_char(key_unicode);
	password_entry *entry;
	int shift_on = key_mod & KMOD_SHIFT,
		alt_on = key_mod & KMOD_ALT,
	    ctrl_on = key_mod & KMOD_CTRL;
	unsigned char* pw;
	int len, cpos;
	int max_width;

	if (!w || !(entry = w->widget_info))
		return 0;

	if (entry->status == P_NONE)
		return -1;

	if (w->Flags & PWORD_FIELD_NO_KEYPRESS)
		return 0;

	max_width = (int)(0.5 + w->len_x - 4*w->size);
	pw = entry->password;
	len = strlen((const char*)pw);
	cpos = entry->cursor_pos;

	if (entry->status == P_TEXT)
	{
		// Selection support only for regular text input
		if (!alt_on && !ctrl_on && shift_on)
		{
			switch (key_code)
			{
				case SDLK_LEFT:
					if (cpos > 0)
					{
						if (entry->sel_begin < 0)
						{
							entry->sel_end = cpos;
							entry->sel_begin = cpos - 1;
						}
						else if (cpos == entry->sel_begin)
						{
							--entry->sel_begin;
						}
						else if (cpos == entry->sel_end)
						{
							if (--entry->sel_end < entry->sel_begin)
								SWAP(entry->sel_begin, entry->sel_end);
						}
						--entry->cursor_pos;
						pword_check_after_left_move(entry, w->fcat, w->size, max_width);
					}
					return 1;
				case SDLK_RIGHT:
					if (cpos < len)
					{
						if (entry->sel_begin < 0)
						{
							entry->sel_end = cpos + 1;
							entry->sel_begin = cpos;
						}
						else if (cpos == entry->sel_begin)
						{
							if (++entry->sel_begin > entry->sel_end)
								SWAP(entry->sel_begin, entry->sel_end);
						}
						else if (cpos == entry->sel_end)
						{
							++entry->sel_end;
						}
						++entry->cursor_pos;
						pword_check_after_right_move(entry, w->fcat, w->size, max_width);
					}
					return 1;
				case SDLK_HOME:
					if (entry->sel_begin < 0)
					{
						entry->sel_end = cpos;
					}
					else if (cpos == entry->sel_end)
					{
						entry->sel_end = entry->sel_begin;
					}
					entry->sel_begin = 0;
					entry->cursor_pos = 0;
					pword_check_after_left_move(entry, w->fcat, w->size, max_width);
					return 1;
				case SDLK_END:
					if (entry->sel_begin < 0)
					{
						entry->sel_begin = cpos;
					}
					else if (cpos == entry->sel_begin)
					{
						entry->sel_begin = entry->sel_end;
					}
					entry->sel_end = len;
					entry->cursor_pos = len;
					pword_check_after_right_move(entry, w->fcat, w->size, max_width);
					return 1;
			}
		}

		// Allow paste into password, but copy or cut only for regular text
		if (KEY_DEF_CMP(K_COPY, key_code, key_mod) || KEY_DEF_CMP(K_COPY_ALT, key_code, key_mod))
		{
			if (entry->sel_begin >= 0 && entry->sel_end > entry->sel_begin)
			{
				char* sel_text = pword_get_selected_text(entry);
				if (sel_text)
				{
					copy_to_clipboard(sel_text);
					free(sel_text);
				}
			}
			return 1;
		}

		if (KEY_DEF_CMP(K_CUT, key_code, key_mod))
		{
			char* sel_text = pword_get_selected_text(entry);
			if (sel_text)
			{
				copy_to_clipboard(sel_text);
				free(sel_text);

				pword_delete(entry, entry->sel_begin, entry->sel_end, w->fcat,
					w->size, max_width);
			}
			entry->sel_begin = entry->sel_end = -1;
			return 1;
		}
	}

	// Allow paste into password, but copy or cut only for regular text
	if (KEY_DEF_CMP(K_PASTE, key_code, key_mod) || KEY_DEF_CMP(K_PASTE_ALT, key_code, key_mod))
	{
		if (entry->sel_begin >= 0 && entry->sel_end > entry->sel_begin)
		{
			pword_delete(entry, entry->sel_begin, entry->sel_end, w->fcat,
					w->size, max_width);
			entry->sel_begin = entry->sel_end = -1;
		}
		start_paste(w);
		return 1;
	}

	if (!alt_on && !ctrl_on)
	{
		switch (key_code)
		{
			case SDLK_LEFT:
				if (cpos > 0)
				{
					--entry->cursor_pos;
					pword_check_after_left_move(entry, w->fcat, w->size, max_width);
				}
				break;
			case SDLK_RIGHT:
				if (cpos < len)
				{
					++entry->cursor_pos;
					pword_check_after_right_move(entry, w->fcat, w->size, max_width);
				}
				break;
			case SDLK_HOME:
				entry->cursor_pos = 0;
				pword_check_after_left_move(entry, w->fcat, w->size, max_width);
				break;
			case SDLK_END:
				entry->cursor_pos = len;
				pword_check_after_right_move(entry, w->fcat, w->size, max_width);
				break;
			case SDLK_DELETE:
				if (entry->sel_begin >= 0 && entry->sel_end > entry->sel_begin)
				{
					pword_delete(entry, entry->sel_begin, entry->sel_end, w->fcat,
						w->size, max_width);
				}
				else if (cpos < len)
				{
					pword_delete(entry, cpos, cpos+1, w->fcat, w->size, max_width);
				}
				break;
			case SDLK_BACKSPACE:

				if (entry->sel_begin >= 0 && entry->sel_end > entry->sel_begin)
				{
					pword_delete(entry, entry->sel_begin, entry->sel_end, w->fcat,
						w->size, max_width);
				}
				else if (cpos > 0)
				{
					pword_delete(entry, cpos-1, cpos, w->fcat, w->size, max_width);
				}
				break;
			default:
				if (is_printable(ch))
				{
					if (entry->sel_begin >= 0 && entry->sel_end > entry->sel_begin)
					{
						pword_delete(entry, entry->sel_begin, entry->sel_end,
							w->fcat, w->size, max_width);
						len = strlen((const char*)pw);
					}
					if (len+1 < entry->max_chars)
						pword_insert(entry, entry->cursor_pos, &ch, 1, w->fcat, w->size, max_width);
				}
				else
				{
					// Probably key-down event, which does not send unicode. Ignore.
					return 0;
				}
				break;
		}

		entry->sel_begin = entry->sel_end = -1;
		return 1;
	}

	// No idea how to handle this input
	return 0;
}

static int pword_pos_under_mouse(password_entry* entry, int mx, int space,
	font_cat cat, float size)
{
	const unsigned char* pw = entry->password;
	int i, cw, str_width;

	switch (entry->status)
	{
		case P_NONE:
			return -1;
		case P_TEXT:
			if (mx < space)
				// Avoid click before the first character setting the cursor position to -1
				return entry->draw_begin;
			for (i = entry->draw_begin, str_width = space; pw[i] && str_width <= mx; ++i)
				str_width += get_char_width_zoom(pw[i], cat, size);
			return (str_width <= mx) ? i : i-1;
		case P_NORMAL:
		default:
			if (mx < space)
				// Avoid click before the first character setting the cursor position to -1
				return entry->draw_begin;
			cw = get_char_width_zoom('*', cat, size);
			return min2i(entry->draw_begin + (mx - space + cw - 1) / cw,
				strlen((const char*)pw));
	}
}

static int pword_field_click(widget_list *w, int mx, int my, Uint32 flags)
{
	password_entry *entry;
	int space;

	if (!w || !(entry = w->widget_info))
		return 0;

	space = (int)(0.5 + 2*w->size);
	entry->cursor_pos = pword_pos_under_mouse(entry, mx, space, w->fcat, w->size);
	entry->sel_begin = entry->sel_end = -1;

	return 1;
}

static int pword_field_drag(widget_list *w, int mx, int my, Uint32 flags, int dx, int dy)
{
	password_entry *entry;
	int space, pos, len;

	if (!w || !(entry = w->widget_info) || entry->status != P_TEXT)
		return 0;

	space = (int)(0.5 + 2*w->size);
	len = strlen((const char*)entry->password);
	pos = pword_pos_under_mouse(entry, mx, space, w->fcat, w->size);
	if (entry->sel_begin < 0 && pos < len)
	{
		entry->sel_begin = entry->drag_begin = max2i(0, pos);
		entry->sel_end = pos + 1;
	}
	else if (pos <= entry->drag_begin)
	{
		entry->sel_begin = max2i(0, pos);
		entry->sel_end = min2i(len, entry->drag_begin + 1);
	}
	else
	{
		entry->sel_begin = entry->drag_begin + 1;
		entry->sel_end = min2i(len, pos + 1);
	}

	return 1;
}

static int pword_field_mouseover(widget_list *w, int mx, int my)
{
	password_entry *entry;
	if (!w || !(entry = (password_entry*)w->widget_info))
		return 0;

	entry->mouseover = 1;
	return 1;
}

static int pword_field_draw(widget_list *w)
{
	password_entry *entry;
	unsigned char* start;
	size_t len;
	int max_width, x_left, x_cursor;
	int space = (int)(0.5 + 2*w->size);
	int sel_begin, sel_end;
	ver_alignment valign;
	int draw_cursor, draw_shadow;

	if (!w || !(entry = (password_entry*)w->widget_info))
		return 0;

	switch (entry->status)
	{
		case P_NONE:
			start = (unsigned char*)"N/A";
			len = 3;
			break;
		case P_TEXT:
			start = entry->password + entry->draw_begin;
			len = entry->draw_end - entry->draw_begin;
			break;
		case P_NORMAL:
		default:
		{
			len = entry->draw_end - entry->draw_begin;
			start = malloc(len);
			memset(start, '*', len);
		}
	}

	if (!(w->Flags & PWORD_FIELD_NO_BORDER))
	{
		// draw the frame
		glDisable (GL_TEXTURE_2D);
		glColor3f (w->r, w->g, w->b);
		glBegin (GL_LINE_LOOP);
			glVertex3i (w->pos_x, w->pos_y, 0);
			glVertex3i (w->pos_x + w->len_x, w->pos_y, 0);
			glVertex3i (w->pos_x + w->len_x, w->pos_y + w->len_y, 0);
			glVertex3i (w->pos_x, w->pos_y + w->len_y, 0);
		if (entry->draw_begin > 0)
			glVertex3i ((int)(w->pos_x + 3*w->size + 0.5), w->pos_y + w->len_y/2, 0);
		glEnd ();
		glEnable (GL_TEXTURE_2D);
	}

	x_left = (int)(w->pos_x + 2*w->size + 0.5);
	x_cursor = x_left + get_buf_width_zoom(start, entry->cursor_pos - entry->draw_begin,
		w->fcat, w->size);
	max_width = w->len_x - 2*space;

	sel_begin = max2i(entry->sel_begin - entry->draw_begin, 0);
	sel_end = max2i(entry->sel_end - entry->draw_begin, 0);
	valign = entry->status == P_NORMAL ? CENTER_PASSWORD : CENTER_LINE;
	draw_shadow = (entry->shadow_r >= 0.0f);
	draw_text(x_left, w->pos_y + w->len_y/2, start, len, w->fcat, TDO_MAX_WIDTH, max_width,
		TDO_SHADOW, draw_shadow, TDO_FOREGROUND, w->r, w->g, w->b,
		TDO_BACKGROUND, entry->shadow_r, entry->shadow_g, entry->shadow_b,
		TDO_ZOOM, w->size, TDO_SEL_BEGIN, sel_begin, TDO_SEL_END, sel_end, TDO_VERTICAL_ALIGNMENT, valign,
		TDO_END);
	draw_cursor = !(w->Flags & PWORD_FIELD_NO_CURSOR)
		&& (entry->mouseover || (w->Flags & PWORD_FIELD_DRAW_CURSOR));
	if (draw_cursor && cur_time % (2*TF_BLINK_DELAY) < TF_BLINK_DELAY)
	{
		draw_text(x_cursor, w->pos_y + w->len_y/2, (const unsigned char*)"_", 1, w->fcat,
			TDO_SHADOW, draw_shadow, TDO_FOREGROUND, w->r, w->g, w->b,
			TDO_BACKGROUND, entry->shadow_r, entry->shadow_g, entry->shadow_b,
			TDO_ZOOM, w->size, TDO_VERTICAL_ALIGNMENT, CENTER_LINE, TDO_END);
	}

	if (entry->status == P_NORMAL)
		free(start);

	entry->mouseover = 0;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static int pword_field_paste(widget_list *w, const char* text)
{
	int max_width, space;
	password_entry *entry;
	if (!w || !(entry = w->widget_info))
		return 0;
	space = (int)(0.5 + 2*w->size);
	max_width = w->len_x - 2*space;
	pword_insert(entry, entry->cursor_pos, (const unsigned char*)text, strlen(text),
		w->fcat, w->size, max_width);
	return 1;
}

void pword_set_status(widget_list *w, Uint8 status)
{
	password_entry *pword;
	if (w && (pword = w->widget_info))
		pword->status = status;
}

int pword_clear(int window_id, Uint32 widget_id)
{
	widget_list *w = widget_find (window_id, widget_id);
	password_entry *entry;
	if (!w || !(entry = w->widget_info))
		return 0;
	if (entry->max_chars)
		entry->password[0] = '\0';
	entry->cursor_pos = 0;
	entry->drag_begin = 0;
	entry->draw_end = 0;
	entry->sel_begin = 0;
	entry->sel_end = 0;
	entry->drag_begin = 0;
	entry->mouseover = 0;
	return 1;
}

int pword_field_set_content(int window_id, Uint32 widget_id, const unsigned char* buf, size_t len)
{
	widget_list *w = widget_find(window_id, widget_id);
	password_entry *entry;
	int str_width = 0, space, max_width;

	if (!w || !(entry = w->widget_info))
		return 0;

	space = (int)(0.5 + 2*w->size);
	max_width = w->len_x - 2*space;

	safe_strncpy2((char*)entry->password, (const char*)buf, entry->max_chars, len);
	entry->sel_begin = entry->sel_end = -1;
	entry->cursor_pos = entry->draw_begin = entry->draw_end = 0;
	while (entry->password[entry->draw_end])
	{
		int chr_width = get_char_width_zoom(entry->password[entry->draw_end], w->fcat, w->size);
		if (str_width + chr_width > max_width)
			break;
		str_width += chr_width;
		++entry->draw_end;
	}

	return 1;
}

int pword_field_set_shadow_color(int window_id, Uint32 widget_id, float r, float g, float b)
{
	widget_list *w = widget_find(window_id, widget_id);
	password_entry *entry;

	if (!w || !(entry = w->widget_info))
		return 0;

	entry->shadow_r = r;
	entry->shadow_g = g;
	entry->shadow_b = b;
	return 1;
}

int pword_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, float size, unsigned char *buffer, int buffer_size)
{
	int space = (int)(0.5 + 2*size), max_width = lx - 2*space;
	int widget_id;
	int str_width = 0;
	widget_list *widget;
	password_entry *T = calloc(1, sizeof (password_entry));

	T->status = status;
	T->password = buffer;
	T->max_chars = buffer_size;
	T->sel_begin = T->sel_end = -1;
	T->shadow_r = T->shadow_g = T->shadow_b = -1.0f;

	if (ly != 0)
	{
		T->fixed_height = ly;
	}
	else
	{
		window_info *win = &windows_list.window[window_id];
		ly = get_line_height(win->font_category, size) + 2*space;
	}

	widget_id = widget_add (window_id, wid, OnInit, x, y, lx, ly, 0, size, &pword_field_type, T, NULL);

	widget = widget_find(window_id, widget_id);
	T->draw_end = 0;
	while (T->password[T->draw_end])
	{
		int chr_width = get_char_width_zoom(T->password[T->draw_end], widget->fcat, size);
		if (str_width + chr_width > max_width)
			break;
		str_width += chr_width;
		++T->draw_end;
	}

	return widget_id;
}

int pword_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, unsigned char *buffer, int buffer_size)
{
	return pword_field_add_extended (window_id, widget_id++, OnInit, x, y, lx, ly, status, 1.0, buffer, buffer_size);
}

// Multiselect
static int free_multiselect(widget_list *widget)
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

static int _multiselect_selected_is_visible(int window_id, Uint32 widget_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M;
	int i, but_start, start_y;

	if (!widget || !(M = widget->widget_info))
		return 0;

	if (M->scrollbar == -1)
		// No scrollbar, even if the options is not visible there is nothing we can do about it
		return 1;

	but_start = vscrollbar_get_pos(M->win_id, M->scrollbar);
	if (M->selected_button < but_start)
		return 0;
	if (M->selected_button == but_start)
		return 1;

	start_y = M->buttons[but_start].y;
	for (i = but_start+1; i < M->nr_buttons; i++)
	{
		int button_y = M->buttons[i].y - start_y;
		if(button_y + M->buttons[i].height > widget->len_y)
			// Button won't be shown, and hence neither will the selected button
			return 0;
		if (M->selected_button == i)
			return 1;
	}

	return 0;
}

int multiselect_set_selected(int window_id, Uint32 widget_id, int button_id)
{
	widget_list *widget = widget_find(window_id, widget_id);
	multiselect *M = widget->widget_info;
	if(M == NULL) {
		return -1;
	} else {
		int i;
		for (i=0; i<M->nr_buttons; i++)
		{
			if (button_id == M->buttons[i].value)
			{
				M->selected_button = i;
				if (M->scrollbar != -1 && !_multiselect_selected_is_visible(window_id, widget_id))
					vscrollbar_set_pos(M->win_id, M->scrollbar, i);

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

static int multiselect_click(widget_list *widget, int mx, int my, Uint32 flags)
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
		if (button_y > widget->len_y - M->buttons[i].height)
			break;
		if((flags&ELW_LEFT_MOUSE || flags&ELW_RIGHT_MOUSE) &&
			my > button_y && my < button_y + M->buttons[i].height && mx > M->buttons[i].x && mx < M->buttons[i].x+M->buttons[i].width) {
				M->selected_button = i;
			do_click_sound();
			return 1;
		}
	}
	return 0;
}

static int multiselect_draw(widget_list *widget)
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

		r = widget->r != -1 ? widget->r : gui_color[0];
		g = widget->g != -1 ? widget->g : gui_color[1];
		b = widget->b != -1 ? widget->b : gui_color[2];

		hr = M->highlighted_red != -1 ? M->highlighted_red : gui_invert_color[0];
		hg = M->highlighted_green != -1 ? M->highlighted_green : gui_invert_color[1];
		hb = M->highlighted_blue != -1 ? M->highlighted_blue : gui_invert_color[2];

		for(i = top_but; i < M->nr_buttons; i++) {
			button_y = M->buttons[i].y - start_y;
			/* Check if the button can be fully drawn */
			if(button_y > widget->len_y - M->buttons[i].height)
				break;
			draw_smooth_button(M->buttons[i].text, widget->fcat, widget->size, widget->pos_x+M->buttons[i].x, widget->pos_y+button_y, M->buttons[i].width-2*BUTTONRADIUS*widget->size, 1, r, g, b, (i == M->selected_button), hr, hg, hb, 0.5f);
		}
	}
	return 1;
}

static int multiselect_set_color(widget_list *widget, float r, float g, float b)
{
	multiselect *M = widget->widget_info;
	if (!M)
		return 0;

	if (M->scrollbar != -1)
		widget_set_color(M->win_id, M->scrollbar, r, g, b);
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
	int button_height = (int)(0.5 + 2.0 * BUTTONRADIUS * size);

	if (text==NULL || strlen(text)==0) {
		M->next_value++;
		return current_button;
	}

	if(y+button_height > widget->len_y && (!M->max_height || widget->len_y != M->max_height)) {
		widget->len_y = y+button_height;
	}

	widget->size=size;

	if (M->max_height && y+button_height > M->actual_height) {
		M->actual_height = y+button_height;
	}
	if(M->max_buttons == M->nr_buttons) {
		/*Allocate space for more buttons*/
		M->buttons = realloc(M->buttons, sizeof(*M->buttons) * M->max_buttons * 2);
		M->max_buttons *= 2;
	}
	safe_strncpy((char*)M->buttons[current_button].text, text, sizeof(M->buttons[current_button].text));
	if(selected) {
		M->selected_button = current_button;
	}
	M->buttons[current_button].value = M->next_value++;
	M->buttons[current_button].x = x;
	M->buttons[current_button].y = y;
	M->buttons[current_button].width = (width == 0) ? widget->len_x : width;
	M->buttons[current_button].height = button_height;

	M->nr_buttons++;
	if(M->max_height && M->scrollbar == -1 && M->max_height < y) {
		int i;

		/* Add scrollbar */
		M->scrollbar = vscrollbar_add_extended(window_id, widget_id++, NULL, widget->pos_x+widget->len_x-M->scrollbar_width,
			widget->pos_y, M->scrollbar_width, M->max_height, 0, 1.0, 0, 1, M->max_height);
		widget_set_color(window_id, M->scrollbar, widget->r, widget->g, widget->b);
		widget->len_x -= M->scrollbar_width + 2;
		widget->len_y = M->max_height;
		/* We don't want things to look ugly. */
		for(i = 0; i < M->nr_buttons; i++) {
			if(M->buttons[i].width > widget->len_x) {
				M->buttons[i].width -= M->scrollbar_width + 2;
			}
		}
	}

	if (M->scrollbar != -1)
		vscrollbar_set_bar_len(window_id, M->scrollbar, M->nr_buttons-widget->len_y/button_height);

	return current_button;
}

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
	T->scrollbar_width = ELW_BOX_SIZE * size;
	T->win_id = window_id;
	T->highlighted_red = hr;
	T->highlighted_green = hg;
	T->highlighted_blue = hb;

	return widget_add (window_id, wid, OnInit, x, y, width, 0, 0, size, &multiselect_type, T, NULL);
}

int multiselect_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, int width)
{
	return multiselect_add_extended(window_id, widget_id++, OnInit, x, y, width, 0, 1.0f, -1, -1, -1, -1, -1, -1, 0);
}

int multiselect_clear(int window_id, Uint32 widget_id)
{
	widget_list *widget;
	multiselect *M;

	widget = widget_find(window_id, widget_id);
	if (!widget || !(M = widget->widget_info))
		return 0;

	M->nr_buttons = 0;
	M->next_value = 0;
	if (M->scrollbar != -1)
	{
		widget_destroy(window_id, M->scrollbar);
		M->scrollbar = -1;
		widget->len_x += M->scrollbar_width + 2;
	}

	return 1;
}

// Spinbutton
static int spinbutton_keypress(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	spinbutton *button;
	if(widget != NULL && (button = widget->widget_info) != NULL &&  !(key_mod & KMOD_ALT) && !(key_mod & KMOD_CTRL)) {
		int len = strlen(button->input_buffer);
		char ch = key_to_char(key_unicode);

		switch(button->type) {
			case SPIN_INT:
			{
				int value;

				if (ch >= '0' && ch <= '9')
				{
					if (len+1 < sizeof(button->input_buffer))
					{
						button->input_buffer[len] = ch;
						button->input_buffer[len+1] = '\0';
					}
				}
				else if (key_code == SDLK_BACKSPACE)
				{
					if (len > 0)
						button->input_buffer[len-1] = '\0';
				}
				else
				{
					return 0;
				}

				value = atoi(button->input_buffer);
				if (value > button->max)
				{
					safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%d",
						(int)button->max);
					*(int *)button->data = button->max;
				}
				else if (value >= button->min)
				{
					*(int *)button->data = value;
				}

				return 1;
			}
			case SPIN_FLOAT:
			{
				float value;

				if (ch == ',')
					ch = '.';
				if ((ch >= '0' && ch <= '9') || (ch == '.' && strchr(button->input_buffer, '.') == NULL))
				{
					/* Make sure we don't insert illegal characters here */
					if (button->input_buffer[0] != '0' || ch == '.' || strchr(button->input_buffer, '.') != NULL)
					{
						/* Append to the end */
						if (len+1 < sizeof(button->input_buffer))
						{
							button->input_buffer[len] = ch;
							button->input_buffer[len+1] = '\0';
						}
					}
				}
				else if (key_code == SDLK_BACKSPACE)
				{
					if (len > 0)
						button->input_buffer[len-1] = '\0';
				}
				else
				{
					return 0;
				}

				value = atof(button->input_buffer);
				if (value > button->max)
				{
					safe_snprintf(button->input_buffer, sizeof(button->input_buffer), "%.2f",
						button->max);
					*(float*)button->data = button->max;
				}
				else if (value >= button->min)
				{
					*(float*)button->data = value;
				}
				return 1;
			}
		}
	}
	return 0;
}

// Note: Discards dx and dy when used for drag. Must be altered if the drag
//		handler changes.
static int spinbutton_click(widget_list *widget, int mx, int my, Uint32 flags)
{
	if(widget != NULL && widget->widget_info != NULL) {
		spinbutton *button = widget->widget_info;
		int arrow_size = 4 * (int)(0.5 + 5 * widget->size);
		Uint8 action = 0;

		if(flags&ELW_WHEEL_UP) {
			action = 'i'; //i for increase
		} else if (flags&ELW_WHEEL_DOWN) {
			action = 'd'; //d for decrease
		} else if(mx > widget->len_x-arrow_size) {
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

static int spinbutton_draw(widget_list *widget)
{
	spinbutton *button;
	int arrow_size = 0;
	char str[255];
	int x_space = (int)(0.5 + widget->size*2);
	int out_of_range = 0;

	if(widget == NULL || (button = widget->widget_info) == NULL) {
		return 0;
	}

	arrow_size = 4 * (int)(0.5 + 5 * widget->size);

	switch(button->type) {
		case SPIN_INT:
			if(atoi(button->input_buffer) < button->min) {
				/* The input buffer has a value less than minimum.
				 * Don't change the data variable and mark the text in red */
				out_of_range = 1;
				safe_strncpy(str, button->input_buffer, sizeof(str));
			} else {
				*(int *)button->data = atoi(button->input_buffer);
				safe_snprintf(str, sizeof (str), "%i", *(int *)button->data);
			}
		break;
		case SPIN_FLOAT:
			if(atof(button->input_buffer) < button->min) {
				out_of_range = 1;
				safe_strncpy(str, button->input_buffer, sizeof(str));
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
	if (out_of_range)
		glColor3f(1, 0, 0);
	else
		glColor3f(widget->r, widget->g, widget->b);
	draw_text(widget->pos_x + x_space, widget->pos_y + widget->len_y/2, (const unsigned char*)str,
		strlen(str), widget->fcat, TDO_ZOOM, widget->size, TDO_VERTICAL_ALIGNMENT, CENTER_DIGITS,
		TDO_END);
	glDisable(GL_TEXTURE_2D);

	/* Border */
	glColor3f(widget->r, widget->g, widget->b);
	glBegin(GL_LINE_LOOP);
		glVertex3i (widget->pos_x, widget->pos_y, 0);
		glVertex3i (widget->pos_x + widget->len_x, widget->pos_y, 0);
		glVertex3i (widget->pos_x + widget->len_x, widget->pos_y + widget->len_y, 0);
		glVertex3i (widget->pos_x, widget->pos_y + widget->len_y, 0);
	glEnd ();
	/* Line between buttons and input */
	glBegin(GL_LINES);
		glVertex3i(widget->pos_x+widget->len_x-arrow_size, widget->pos_y,0);
		glVertex3i(widget->pos_x+widget->len_x-arrow_size, widget->pos_y+widget->len_y,0);
	glEnd();
	/* Up arrow */
	glBegin(GL_TRIANGLES);
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + arrow_size/4, widget->pos_y + widget->len_y/4+2, 0); //Left corner
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + arrow_size/2, widget->pos_y + 2, 0); //Top
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + 3*arrow_size/4, widget->pos_y + widget->len_y/4+2, 0); //Right corner
	glEnd();
	/* Button separator */
	glBegin(GL_LINES);
		glVertex3i(widget->pos_x+widget->len_x-arrow_size, widget->pos_y+widget->len_y/2,0);
		glVertex3i(widget->pos_x+widget->len_x, widget->pos_y+widget->len_y/2,0);
	glEnd();
	/* Down arrow */
	glBegin(GL_TRIANGLES);
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + arrow_size/4, widget->pos_y + widget->len_y - widget->len_y/4-2, 0); //Left corner
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + arrow_size/2, widget->pos_y + widget->len_y - 2, 0); //Bottom
		glVertex3i(widget->pos_x+widget->len_x-arrow_size + 3*arrow_size/4, widget->pos_y + widget->len_y - widget->len_y/4-2, 0); //Right corner
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int spinbutton_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval, float size)
{
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

	return widget_add (window_id, wid, OnInit, x, y, lx, ly, 0, size, &spinbutton_type, T, NULL);
}

int spinbutton_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval)
{
	return spinbutton_add_extended(window_id, widget_id++, OnInit, x, y, lx, ly, data_type, data, min, max, interval, 1);
}

// Helper functions for widgets

void draw_cross(int centre_x, int centre_y, int half_len, int half_width)
{
	glBegin(GL_QUADS);
		glVertex2i(centre_x - half_len, centre_y - half_len + half_width);
		glVertex2i(centre_x - half_len + half_width, centre_y - half_len);
		glVertex2i(centre_x + half_len, centre_y + half_len - half_width);
		glVertex2i(centre_x + half_len - half_width, centre_y + half_len);
		glVertex2i(centre_x + half_len, centre_y - half_len + half_width);
		glVertex2i(centre_x + half_len - half_width, centre_y - half_len);
		glVertex2i(centre_x - half_len, centre_y + half_len - half_width);
		glVertex2i(centre_x - half_len + half_width, centre_y + half_len);
	glEnd();
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
			return label_add_extended (winid, id, NULL, pos_x, pos_y, flags, size, text);
		case IMAGE:
			return image_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, tid, u1, v1, u2, v2);
		case CHECKBOX:
		{
			int *checked_ptr = calloc(1,sizeof(int));
			*checked_ptr = checked;
			return checkbox_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, checked_ptr);
		}
		case BUTTON:
			return button_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, text);
		case PROGRESSBAR:
			return progressbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, progress, NULL);
		case VSCROLLBAR:
			return vscrollbar_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, pos, pos_inc, len_y);
		case TABCOLLECTION:
		{
			xmlNode *tab;
			int colid = tab_collection_add_extended (winid, id, NULL, pos_x, pos_y, len_x, len_y, flags, size, max_tabs, tag_height);
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
