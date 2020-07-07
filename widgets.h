/*!
 * \file
 * \ingroup widgets
 * \brief Functions for the widgets used by EL
 */
#ifndef	__WIDGETS_H
#define	__WIDGETS_H

typedef struct select_info select_info;

#include <SDL_types.h>
#include <SDL_keycode.h>
#include "font.h"
#include "text.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char label[64];
	int content_id;
	Uint16 tag_width;
	Uint16 min_tag_width;
	float label_r, label_g, label_b;
	char closable;
} tab;

typedef struct {
	int tag_height, button_size;
	int nr_tabs, max_tabs, cur_tab, tab_offset, tab_last_visible;
	tab *tabs;
} tab_collection;

// Forward declaration
struct wl;

// The purpose of this implementation if to remove the need to edit
// the implementation of the widgets to add a new client widget. It
// also allows clients to be dynamically created and populated.
struct WIDGET_TYPE {
    // Function Pointers
	int (*init)();
	int (*draw)();
	int (*click)();
	int (*drag)();
	int (*mouseover)();
	int (*resize)();
	int (*key)();
	int (*destroy)();
	int (*move)();
	int (*font_change)();
	int (*paste)();
	int (*color_change)(struct wl*, float, float, float);
    // We can conceivably store other generic info here too
} ;

typedef struct {
	int pos, pos_inc, bar_len;
}vscrollbar;

/*!
 * The widget list structure - each window has a widget list.
 */
typedef struct wl{
    /*!
	 * \name Common widget data
     */
    /*! @{ */
	Uint16 pos_x, pos_y, len_x, len_y; /*!< Widget area */
	Uint32 id;                         /*!< Widget unique id */
	int window_id; /*!< The id of the parent window */
	const struct WIDGET_TYPE *type;  /*!< Specifies what properties the widget inherits from it's type */
	void *spec;		/*!< The specific implementation info for this widget which is passed to type-nonspecific handlers*/
	Uint32 Flags;  /*!< Status flags... visible, enabled, etc */
	float size;    /*!< Size of text, image, etc */
	float r, g, b; /*!< Associated color */
	font_cat fcat; /*!< Font category for drawing text contents in */
    /*! @} */

	/*! \name The specific widget handlers */
	/*! \{ */
	int (*OnDraw)();
	int (*OnClick)();
	int (*OnDrag)();
	int (*OnInit)();
	int (*OnMouseover)();
	int (*OnResize)();
	int (*OnKey)();
	int (*OnDestroy)();
	int (*OnFontChange)();
	/*! \} */

	void *widget_info; /*!< Pointer to specific widget data */
	struct wl *next;   /*!< Pointer to the next widget in the window */
}widget_list;

/*!
 * \name	Generic flags for widgets
 */
/*! \{ */
#define WIDGET_INVISIBLE	0x40
#define WIDGET_DISABLED		0x80
#define WIDGET_CLICK_TRANSPARENT 0x100
/*! \} */

/*!
 * \name	Flags for the buttons
 */
/*! \{ */
#define BUTTON_ACTIVE          0x0400
#define BUTTON_SQUARE          0x0800
#define BUTTON_VCENTER_CONTENT 0x1000
/*! \} */

/*!
 * \name	Flags for the text field
 */
/*! \{ */
#define TEXT_FIELD_BORDER          0x01
#define TEXT_FIELD_EDITABLE        0x02
#define TEXT_FIELD_NO_KEYPRESS     0x04
#define TEXT_FIELD_CAN_GROW        0x08
#define TEXT_FIELD_SCROLLBAR       0x10
#define TEXT_FIELD_IGNORE_RETURN   0x20
#define TEXT_FIELD_MOUSE_EDITABLE 0x200
/*! \} */

/*!
 * \name Flags for the password field
 */
/*! \{ */
#define PWORD_FIELD_NO_KEYPRESS TEXT_FIELD_NO_KEYPRESS
#define PWORD_FIELD_NO_BORDER   0x2000
#define PWORD_FIELD_DRAW_CURSOR 0x4000
#define PWORD_FIELD_NO_CURSOR   0x8000
/*! \} */

#define TF_BLINK_DELAY 500

/*!
 * Contains auxilary information for selection.
 */
typedef struct
{
	int msg, chr;
} text_field_line;

/*!
 * Contains selection information for text_field.
 */
struct select_info
{
	text_field_line* lines;
	int sm, sc, em, ec;
};

/*!
 * Checks if selection is empty.
 */
#define TEXT_FIELD_SELECTION_EMPTY(select) (((select)->em == -1) && ((select)->ec == -1))

/*!
 * Makes given selection empty.
 */
#define TEXT_FIELD_CLEAR_SELECTION(select) {(select)->em = (select)->ec = -1;}

/*!
 * Text field structure
 */
typedef struct
{
	int msg, offset;
	int cursor, cursor_line;
	int buf_size, buf_fill;
	int nr_lines, nr_visible_lines;
	int update_bar;
	int scroll_id;
	int scrollbar_width;
	int line_offset;
	text_message *buffer;
	Uint8 chan_nr;
	Uint16 x_space, y_space;
	Uint32 next_blink;
	select_info select;
} text_field;

typedef struct {
	void *data;
	char input_buffer[255];
	float max;
	float min;
	Uint8 type;
	float interval;
}spinbutton;

/* SPLIT INTO ELWIDGETS.C and ELWIDGETS.H */

// Common widget functions

/*!
 * \ingroup	widgets
 * \brief 	Creates a widget and adds it to the given window
 *
 * 		Creates a widget and adds it to the given window.
 *
 * \param  	window_id The location of the window in the windows_list.window[] array
 * \param	wid The widget's unique ID
 * \param   OnInit The function used for initiating the label
 * \param   x The x location
 * \param   y The y location
 * \param   lx The width
 * \param   ly The height
 * \param   Flags The flags
 * \param   size The text size
 * \param   type The widget type
 * \param   T Pointer to specific widget data
 * \param   S Pointer to specific implementation info
 * \retval int Returns the new widgets unique ID
 */
Uint32 widget_add (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y,
	Uint16 lx, Uint16 ly, Uint32 Flags, float size, const struct WIDGET_TYPE *type, void *T, void *S);

/*!
 * \ingroup	widgets
 * \brief 	Find a widget with the given widget_id
 *
 * 		Returns the widget with the given widget_id in window_id.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \retval widget_list*  	A widget_list pointer to the widget if found, otherwise NULL
 */
widget_list * widget_find(int window_id, Uint32 widget_id);

/*!
 * \ingroup	widgets
 * \brief 	Destroy a widget with a given ID
 *
 * 		Destroys a widget with ID \a widget_id, and removes it from the window's widget list.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \retval	int 1 on success, 0 on failure
 */
int widget_destroy (int window_id, Uint32 widget_id);

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's draw callback
 *
 * 		Finds the widget in the window and sets the widget's draw callback in the specified window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnDraw(int window_id, Uint32 widget_id, int (*handler)());

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's on-click handler
 *
 *      	Finds the widget in the window and sets the widget's on-click handler
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnClick(int window_id, Uint32 widget_id, int (*handler)());

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's on-drag handler
 *
 * 		Finds the widget in the window and sets the widget's on-drag handler
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnDrag(int window_id, Uint32 widget_id, int (*handler)());

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's on-mouse-over handler
 *
 * 		Finds the widget in the window and sets the widget's on-mouse-over handler.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler.
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnMouseover(int window_id, Uint32 widget_id, int (*handler)());

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's on-keypress handler
 *
 * 		Finds the widget in the window and sets the widget's on-keypress handler.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler.
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnKey ( int window_id, Uint32 widget_id, int (*handler)() );

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's specific argument (passed to specific handlers)
 *
 * 		Finds the widget in the window and sets the widget's specific argument.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	spec A pointer to the memory of the argument.
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_args (int window_id, Uint32 widget_id, void *spec);

/*!
 * \ingroup 	widgets
 * \brief 	Moves the widget
 *
 *      	Finds the widget in the window and moves the widget to the new x,y in the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	x The new x location
 * \param   	y The new y location
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_move(int window_id, Uint32 widget_id, Uint16 x, Uint16 y);

/*!
 * \ingroup 	widgets
 * \brief 	Moves the widget to a new window
 *
 *      	Finds the widget in the window and moves it to the window new_win_id
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	new_win_id The location of the target window in the windows_list.window[] array
 * \retval Uint32 	 Returns the new widget id on success or 0 on failure (the new id will never be 0)
 *
 * \sa widget_find
 */
Uint32 widget_move_win(int window_id, Uint32 widget_id, int new_win_id);

/*!
 * \ingroup 	widgets
 * \brief 	Moves the widget relative to it's current position
 *
 *      	Finds the widget in the window and moves the widget relative to it's current position.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	dx The shift in the x-direction
 * \param   	dy The shift in the y-direction
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_move_rel (int window_id, Uint32 widget_id, Sint16 dx, Sint16 dy);

/*!
 * \ingroup	widgets
 * \brief 	Resizes the widget
 *
 * 		Finds the widget in the window and resizes the widget to the given x_len and y_len.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	x The new width
 * \param   	y The new height
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_resize(int window_id, Uint32 widget_id, Uint16 x, Uint16 y);

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's flags to f
 *
 * 		Finds the widget in the window and sets the widgets flags.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	f The flags
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_flags(int window_id, Uint32 widget_id, Uint32 f);

/*!
 * \ingroup	widgets
 * \brief 	Unsets the specified flags
 *
 * 		Finds the widget in the window and unsets the specified flags.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	f The flags
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_unset_flags (int window_id, Uint32 widget_id, Uint32 f);

/*!
 * \ingroup	widgets
 * \brief 	Set the widget's text size
 *
 * 		The function finds the widget in the window and sets it's text size.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	size The new text size
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_size(int window_id, Uint32 widget_id, float size);

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget colour
 *
 * 		Finds the widget in the given window and sets the r g b foreground colour.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \retval int  Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_color(int window_id, Uint32 widget_id, float r, float g, float b);
/*!
 * \ingroup	widgets
 * \brief 	Unsets the widget colour
 *
 * Finds the widget in the given window and removes the color. The widget will be drawn in
 * the last color used.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \retval int  Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
static __inline__ int widget_unset_color(int window_id, Uint32 widget_id)
{
	return widget_set_color(window_id, widget_id, -1.0f, -1.0f, -1.0f);
}

/*!
 * \ingroup widgets
 * \brief Set the font category
 *
 * Set the font category for the textual elements in this widget to \a fcat
 *
 * \param window_id The location of the window in the windows_list.window[] array
 * \param widget_id The widget's unique ID
 * \param fcat      The new font category for this widget
 * \return 1 on succes or 0 on failure
 */
int widget_set_font_cat(int window_id, int widget_id, font_cat fcat);

/*!
 * \ingroup	widgets
 * \brief 	Return the widget width
 *
 * 		Finds the widget in the given window and returns its width
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \retval int  	Returns the width on succes or -1 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_get_width (int window_id, Uint32 widget_id);

/*!
 * \ingroup	widgets
 * \brief 	Return the widget height
 *
 * 		Finds the widget in the given window and returns its height
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \retval int  	Returns the width on succes or -1 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_get_height (int window_id, Uint32 widget_id);

// Label

/*!
 * \ingroup	labels
 * \brief 	Creates an extended label widget
 *
 * 		Creates an extended label widget and adds it to the given window.
 *
 * \param  	window_id The location of the window in the windows_list.window[] array
 * \param	wid The widget's unique ID
 * \param   	OnInit The function used for initiating the label
 * \param   	x The x location
 * \param   	y The y location
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param   	text The text
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa lable_add
 */
int label_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint32 Flags, float size, const char *text);

/*!
 * \ingroup	labels
 * \brief 	Creates a label and adds it to the given window.
 *
 * 		Creates a label and adds it to the given window - calls label_add_extended.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function called on init
 * \param   	text The text
 * \param   	x The x position
 * \param   	y The y position
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa		label_add_extended
 */
int label_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y);

/*!
 * \ingroup	labels
 * \brief	Draws a label
 *
 * 		Draws the label given by the widget.
 *
 * \param   	W The widget that is to be drawn
 * \retval int  	Returns true
 * \callgraph
 */
int label_draw(widget_list *W);

/*!
 * \ingroup	labels
 * \brief 	Sets the text of the given widget
 *
 * 		Finds the widget in the given window and sets the text.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	text The new text
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
 *
 * \sa widget_find
 */
int label_set_text(int window_id, Uint32 widget_id, const char *text);



// Image

/*!
 * \ingroup	images
 * \brief 	Create an extended image widget
 *
 * 		Creates an extended image widget and adds it to the window_id.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The widget's unique ID
 * \param   	OnInit Sets the function to run on init
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	id The offset in the texture_cache
 * \param   	u1 The start u texture coordinate
 * \param   	v1 The start v texture coordinate
 * \param   	u2 The end u texture coordinate
 * \param   	v2 The end v texture coordinate
 * \param   	alpha The alpha value for the image
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa image_add
 */
int image_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int id, float u1, float v1, float u2, float v2, float alpha);

/*!
 * \ingroup	images
 * \brief 	Creates an image widget
 *
 *      	Creates an image widget. Calls image_add_extended.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit Sets the init handler
 * \param   	id The texture id in the texture_cache
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	u1 The start u texture coordinate
 * \param   	v1 The start v texture coordinate
 * \param   	u2 The end u texture coordinate
 * \param   	v2 The end v texture coordinate
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa image_add_extended
 */
int image_add(int window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2);

/*!
 * \ingroup	images
 * \brief 	Draws the image widget
 *
 * 		Draws an image widget as given by the widget *.
 *
 * \param   	W A pointer to the widget that should be drawn
 * \retval int  	Returns true
 * \callgraph
 */
int image_draw(widget_list *W);

/*!
 * \ingroup	images
 * \brief 	Sets the texture ID
 *
 * 		The function sets the texture ID (or rather, the location in the texture_cache) of the given widget.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widgets unique ID
 * \param   	id The location in the texture_cache array
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
 *
 * \sa widget_find
 */
int image_set_id(int window_id, Uint32 widget_id, int id);

/*!
 * \ingroup 	images
 * \brief 	Sets the UV coordinates of the image widget
 *
 * 		Sets the UV coordinates of the image widget
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	u1 The start u texture coordinate
 * \param   	v1 The start v texture coordinate
 * \param   	u2 The end u texture coordinate
 * \param   	v2 The end v texture coordinate
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
 *
 * \sa widget_find
 */
int image_set_uv(int window_id, Uint32 widget_id, float u1, float v1, float u2, float v2);



// Checkbox

/*!
 * \ingroup	checkboxes
 * \brief 	Create an extended checkbox label
 *
 * 		Creates an extended checkbox label and adds it to the given image.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function used for initiating the widget
 * \param   	x The x location
 * \param   	y The y locatoin
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	checked Specifies if the widget is checked or not
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa checkbox_add
 */
int checkbox_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int *checked);

/*!
 * \ingroup	checkboxes
 * \brief 	Creates a checkbox
 *
 * 		Creates a checkbox and adds it to the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function used for initiating the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	checked Specifies whether the checkbox is checked or not
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa checkbox_add_extended
 */
int checkbox_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int *checked);

/*!
 * \ingroup	checkboxes
 * \brief 	Draws a checkbox
 *
 * 		Draws the checkbox pointed to by *W.
 *
 * \param   	W The widget you wish to draw
 * \retval int  	Returns true
 * \callgraph
 */
int checkbox_draw(widget_list *W);

/*!
 * \ingroup	checkboxes
 * \brief 	Checks if the given checkbox is checked
 *
 * 		Is used for checking if the given checkbox widget is checked or not.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \retval int  	Returns 0 if the checkbox is unchecked, 1 if the checkbox is checked and -1 if the checkbox is not even found.
 *
 * \sa widget_find
 */
int checkbox_get_checked(int window_id, Uint32 widget_id);

/*!
 * \ingroup	checkboxes
 * \brief 	Is used for setting the checkbox as checked or not
 *
 * 		Finds the given checkbox in the window and sets it as checked or not
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	checked Whether it should be checked or not
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
 *
 * \sa widget_find
 */
int checkbox_set_checked(int window_id, Uint32 widget_id, int checked);


//Button

/* Config option: disables double click protection. */
extern int disable_double_click;

/*!
 * \ingroup	buttons
 * \brief 	Check for safety protected button press.
 *
 * 		Some buttons are protected from mis-click by requiring you to
 * 		double-click them.  This protection can be disabled by setting
 *		the disable_double_click config option to true.  This function
 *		tests that option and impliments the double click test if needed.
 *
 * \param last_click	The SDL_GetTicks() value from the last click
 * \retval int			Returns 1 if the button press should be actioned, else 0.
 */
int safe_button_click(Uint32 *last_click);

/*!
 * \ingroup	buttons
 * \brief 	Creates an extended button widget
 *
 * 		Creates an extended button widget and adds it to the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function called on initiating the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	text The button label
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa button_add
 */
int button_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, const char *text);

/*!
 * \ingroup	buttons
 * \brief 	Creates a button widget
 *
 * 		Creates a button widget and adds it to the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function called on initiating the widget
 * \param   	text The button label
 * \param   	x The x position
 * \param   	y The y position
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa button_add_extended
 */
int button_add(int window_id, int (*OnInit)(), const char *text, Uint16 x, Uint16 y);

/*!
 * \ingroup buttons
 * \brief Resize a button widget
 *
 * Resize a button widget using to the specified dimensions. If \a lx or \a ly
 * are zero, the corresponding dimension is calculated from the button's contents.
 *
 * \param window_id The location of the window in the windows_list.window[] array
 * \param wid       The unique widget ID for the button
 * \param lx        The new width
 * \param ly        The new height
 * \param size      the new font and button size
 * \retval int Returns the new widgets unique ID
 *
 * \sa button_add_extended
 */
int button_resize(int window_id, Uint32 wid, Uint16 lx, Uint16 ly, float size);

/*!
 * \ingroup	buttons
 * \brief 	Sets the button text
 *
 * 		Finds the given button widget and sets the button text
 *
 * \param   window_id The location of the window in the windows_list.window[] array
 * \param   widget_id The unique widget ID
 * \param   text The button label
 * \retval int  Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
 *
 * \sa widget_find
 */
int button_set_text(int window_id, Uint32 widget_id, const char *text);

/*!
 * \ingroup buttons
 * \brief Draws a button with round corners.
 *
 * 	Draws a button with round corners. The box can be highlighted with the chosen highlight colors (r,g,b,a).
 *
 * \param str The name to write within the button, optional
 * \param cat The category for the font with which to draw \a str
 * \param size The size of the text
 * \param x The start x position
 * \param y The start y position
 * \param w The width
 * \param lines The number of lines (determines the height)
 * \param r The red color for border and text
 * \param g The green color for border and text
 * \param b The blue color for border and text
 * \param highlight If the button is highlighted or not
 * \param hr The red color for highlighted buttons
 * \param hg The green color for highlighted buttons
 * \param hb The blue color for highlighted buttons
 * \param ha The alpha color for highlighted buttons
 */
void draw_smooth_button(const unsigned char* str, font_cat cat, float size,
	int x, int y, int w, int lines, float r, float g, float b,
	int highlight, float hr, float hg, float hb, float ha);
/*!
 * \ingroup buttons
 * \brief Compute the width of a button
 *
 * Calculate the normal width of a button of size \a size, with label \a label drawn in the font
 * for category \a cat.
 *
 * \param label The text to draw on the button
 * \param cat   The font category for the button
 * \param size  The size scale factor for the button
 * \return The width of the button, in pixels
 */
int calc_button_width(const unsigned char* label, font_cat cat, float size);

// Progressbar

/*!
 * \ingroup	progressbars
 * \brief 	Adds an extended progressbar widget
 *
 * 		Adds an extended progressbar widget to the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function called on initiating the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	progress The current progress
 * \param     colors The colors of the four corners of the bar. Pointer to an array of 12 floats (4 consecutive RGB colors). May be NULL.
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa progressbar_add
 */
int progressbar_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float progress, const float * colors);

/*!
 * \ingroup	progressbars
 * \brief 	Adds a progressbar widget
 *
 * 		Adds a progressbar widget to the given window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function called on initiating the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa progressbar_add_extended
 */
int progressbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \ingroup	progressbars
 * \brief 	Draws a progressbar
 *
 * 		The function draws the progressbar pointed to by *W
 *
 * \param   	W The progressbar widget that is going to be drawn
 * \retval int  	Returns false
 * \callgraph
 */
int progressbar_draw(widget_list *W);

/*!
 * \ingroup	progressbars
 * \brief 	Gets the progress from a progressbar
 *
 * 		Finds the progressbar widget and returns the current progress
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \retval float  	Returns -1 on failure, otherwise the current progress.
 *
 * \sa widget_find
 */
float progressbar_get_progress(int window_id, Uint32 widget_id);

/*!
 * \ingroup	progressbars
 * \brief 	Sets the current progress in the progressbar
 *
 * 		The function finds the progressbar and sets it's progress.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	progress The new progress
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget_id was not found in that window).
 *
 * \sa widget_find
 */
int progressbar_set_progress(int window_id, Uint32 widget_id, float progress);



// Vertical Scrollbar

/*!
 * \ingroup	scrollbars
 * \brief 	Creates an extended vertical scrollbar widget
 *
 * 		Creates an extended vertical scrollbar widget and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function called when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param   	pos
 * \param   	pos_inc
 * \param		bar_len
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa vscrollbar_add
 */
int vscrollbar_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, int pos, int pos_inc, int bar_len);

/*!
 * \ingroup	scrollbars
 * \brief 	Creates a vertical scrollbar
 *
 * 		Creates a vertical scrollbar widget and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function used when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa vscrollbar_add_extended
 */
int vscrollbar_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \ingroup	scrollbars
 * \brief 	Draws a vertical scrollbar
 *
 * 		Draws the vertical scrollbar given by *W
 *
 * \param   	W A pointer to the vertical scrollbar widget you wish to draw
 * \retval int  	Returns false
 * \callgraph
 */
int vscrollbar_draw(widget_list *W);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets the position.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	pos_inc The position increase (or decrease)
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_set_pos_inc(int window_id, Uint32 widget_id, int pos_inc);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets the position.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	pos The new position
 * \retval int  	Returns 1 on success, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_set_pos(int window_id, Uint32 widget_id, int pos);

/*!
 * \ingroup	scrollbars
 * \brief 	Scrolls the scrollbar up
 *
 * 		Finds the vertical scrollbar widget and sets the position a bit up.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \retval int  	Returns 1 on success, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_scroll_up(int window_id, Uint32 widget_id);

/*!
 * \ingroup	scrollbars
 * \brief 	Scrolls the scrollbar down
 *
 * 		Finds the vertical scrollbar widget and sets the position a bit down.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \retval int  	Returns 1 on success, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_scroll_down(int window_id, Uint32 widget_id);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the logical length of vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets its logical bar length.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	bar_len The new logical bar length
 * \retval int  	Returns 1 on success, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_set_bar_len (int window_id, Uint32 widget_id, int bar_len);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and returns the position
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \retval int  	Returns pos on succes, -1 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_get_pos(int window_id, Uint32 widget_id);


// Tabbed window

/*!
 * \ingroup	tabs
 * \brief 	Returns the number of the currently selected tab
 *
 * 		Returns the number of the currently selected tab in a tabbed window collection. Numbers are in the range 0...nr_tabs-1.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID of the tab collection
 * \retval int  	Returns the tab number on succes, -1 on failure
 *
 * \sa widget_find
 */
int tab_collection_get_tab (int window_id, Uint32 widget_id);

/*!
 * \ingroup	tabs
 * \brief 	Returns the window ID of the currently selected tab
 *
 * 		Returns the window ID of the currently selected tab in a tabbed window collection.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID of the tab collection
 * \retval int  	Returns the tab's window ID number on succes, -1 on failure
 */
int tab_collection_get_tab_id (int window_id, Uint32 widget_id);

/*!
 * \ingroup	tabs
 * \brief 	Returns the position of a tab in the collection from its window ID
 *
 * 		Returns the position of a tab in the collection from its window ID
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	col_id The unique widget ID of the tab collection
 * \param   	tab_id The tab's window ID
 * \retval int  	Returns the tab's number on succes, -1 on failure
 */
int tab_collection_get_tab_nr (int window_id, Uint32 col_id, int tab_id);

/*!
 * \ingroup	tabs
 * \brief 	Returns the number of tabs in this collection
 *
 * 		Returns the number of tabs in this collection
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID of the tab collection
 * \retval int  	Returns the number of tabs, or -1 on failure
 */
int tab_collection_get_nr_tabs (int window_id, Uint32 widget_id);

/*!
 * \ingroup	tabs
 * \brief 	Sets the label color for a tab
 *
 * 		Sets the color with which the label of the tab belonging to the window with ID \a tab_id is drawn.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	col_id The unique widget ID of the tab collection
 * \param	tab_id The window ID of the tab window
 * \param	r the red component of the color
 * \param	g the green component of the color
 * \param	b the blue component of the color
 * \retval int  	Returns the tab's window ID number on succes, -1 on failure
 */
int tab_set_label_color_by_id (int window_id, Uint32 col_id, int tab_id, float r, float g, float b);

/*!
 * \ingroup	tabs
 * \brief 	Selects a tab in the tab collection
 *
 * 		Select a tab from the tab collection and bring it to the front
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID of the tab collection
 * \param	tab The number of the tab to be selected
 * \retval int  	Returns the tab number on succes, -1 on failure (if the tab number was greater than or equal to the number of tabs in the collection)
 * \callgraph
 */
int tab_collection_select_tab (int window_id, Uint32 widget_id, int tab);

/*!
 * \ingroup	tabs
 * \brief 	Closes a tab in the tab collection
 *
 * 		Closes a tab from the tab collection and destroys the associated window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID of the tab collection
 * \param	tab The number of the tab to be closed
 * \retval int  	Returns the tab number on succes, -1 on failure (if the tab number was greater than or equal to the number of tabs in the collection)
 * \callgraph
 */
int tab_collection_close_tab (int window_id, Uint32 widget_id, int tab);

/*!
 * \ingroup	tabs
 * \brief 	Calculate the tab tag height
 *
 * Calculate the tab tag height given the specified size and font category for
 * the label.
 *
 * \param cat  the category for the font with which the label is drawn
 * \param size the scale factor
 * \retval int  	Returns the calculate tag tag height.
 * \callgraph
 */
int tab_collection_calc_tab_height(font_cat cat, float size);

/*!
 * \ingroup	tabs
 * \brief 	Creates a tabbed window collection
 *
 * 		Creates a tabbed window collection and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function used when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa tab_collection_add_extended
 */
int tab_collection_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \ingroup	tabs
 * \brief 	Creates an extended tabbed window collection
 *
 * 		Creates an extended tabbed window collection and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function called when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param   	size The text size
 * \param	max_tabs The largest number of tabs this collection will hold
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa tab_collection_add
 */
int tab_collection_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y,
	Uint16 lx, Uint16 ly, Uint32 Flags, float size, int max_tabs);

/*!
 * \ingroup	tabs
 * \brief 	Draws a tabbed window collection
 *
 * 		Draws the vertical tabbed window collection given by *W
 *
 * \param   	W A pointer to the tabbed window collection you wish to draw
 * \retval int  	Returns 1 on success, 0 on error
 * \callgraph
 */
int tab_collection_draw (widget_list *W);

/*!
 * \ingroup	tabs
 * \brief 	The callback for resizing the tabbed window collection widget
 *
 * 		The callback for resizing the tabbed window collection widget
 *
 * \param   	W The widget
 * \param   	w the new width
 * \param   	h the new height
 * \retval int  	Returns 1 on success, 0 on failure
 * \callgraph
 */
int tab_collection_resize (widget_list *W, Uint32 w, Uint32 h);

/*!
 * \ingroup	tabs
 * \brief 	Move the tabbed window collection widget tabs
 *
 * 		Move the tabbed window collection widget tabs
 *
 * \param   	W The widget
 * \param   	pos_x the absolute x position
 * \param   	pos_y the absolute y position
 * \retval int  	Returns 1 on success, 0 on failure
 * \callgraph
 */
int tab_collection_move (widget_list *W, Uint32 pos_x, Uint32 pos_y);

/*!
 * \ingroup	tabs
 * \brief 	Creates a new tabbed window
 *
 * 		Creates a new tabbed window
 *
 * \param   	window_id The location of the parent window in the windows_list.window[] array
 * \param   	col_id The unique widget id of the tabbed window collection in which this tab is created
 * \param   	label The name of this tab as it appears on its tag
 * \param	tag_width The width of the tag
 * \param	closable Flag indicating if the tab can be closed
 * \param	flags Flags to be passed to the create_window() function
 * \retval int  	Returns 1 if a new tab is selected, 0 otherwise
 * \callgraph
 */
int tab_add (int window_id, Uint32 col_id, const char *label, Uint16 tag_width, int closable, Uint32 flags);

/*!
 * \ingroup	textfields
 * \brief 	Creates a text field
 *
 * 		Creates a text field and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	OnInit The function used when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param	buf the message buffer
 * \param 	buf_size the size of the message buffer
 * \param	x_space the number of pixels in the x-direction between the border and the text
 * \param	y_space the number of pixels in the y-direction between the border and the text
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa text_field_add_extended
 */
int text_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, text_message *buf, int buf_size, int x_space, int y_space);

/*!
 * \ingroup	textfields
 * \brief 	Creates an extended text field
 *
 * 		Creates an extended text field and adds it to the given window
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	wid The unique widget ID
 * \param   	OnInit The function used when initializing the widget
 * \param   	x The x position
 * \param   	y The y position
 * \param   	lx The width
 * \param   	ly The height
 * \param   	Flags The flags
 * \param		fcat Font category for the text
 * \param   	size The text size
 * \param	buf the text buffer
 * \param 	buf_size the size of the text buffer
 * \param	chan_filt the channel of which messages are drawn
 * \param	x_space the number of pixels in the x-direction between the border and the text
 * \param	y_space the number of pixels in the y-direction between the border and the text
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa text_field_add
 */
int text_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(),
	Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, font_cat fcat,
	float size, text_message *buf, int buf_size, Uint8 chan_filt, int x_space, int y_space);

/*!
 * \ingroup	textfields
 * \brief 	Draws a text field
 *
 * 		Draws the vertical textfield given by \a *w
 *
 * \param   	w A pointer to the text field you wish to draw
 * \retval int  	Returns 1 on success, 0 on error
 * \callgraph
 */
int text_field_draw (widget_list *w);

/*!
 * \ingroup	textfields
 * \brief 	Sets the offset in the text buffer
 *
 * 		Sets the offset in the buffer at which the text_field starts drawing
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param	widget_id The unique widget ID
 * \param	msg the new message nr
 * \param	offset the new offset within the message
 * \retval int  	Returns 1 on success, 0 on error
 * \callgraph
 */
int text_field_set_buf_pos (int window_id, Uint32 widget_id, int msg, int offset);

/*!
 * \ingroup	textfields
 * \brief       Clear an editable text field
 *
 *              Clear an editable text field, erasing its current buffer and
 *              moving the cursor to the start
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param	widget_id The unique widget ID
 * \retval int  	Returns 1 on success, 0 on error
 * \callgraph
 */
int text_field_clear (int window_id, Uint32 widget_id);

/*!
 * \ingroup	textfields
 * \brief 	Sets the text color
 *
 * 		Sets the color with which the text is drawn. Not that color characters in the text override this setting.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param	widget_id The unique widget ID
 * \param	r the red component of the text color
 * \param	g the green component of the text color
 * \param	b the blue component of the text color
 * \retval int  	Returns 1 on success, 0 on error
 * \callgraph
 */
int text_field_set_text_color (int window_id, Uint32 widget_id, float r, float g, float b);

/*!
 * \ingroup	widgets
 * \brief	Is called on keypress in the given widget
 *
 * 		Is called on keypress in the given widget
 *
 * \param	w pointer to the widget structure
 * \param   	mx the mouse x position relative to the widgets origin
 * \param   	my the mouse y position relative to the widgets origin
 * \param	key_code the SDL key code
 * \param	key_unicode the unicode representation of the key pressed
 * \param	key_mod the status bitmask for mod keys
 * retval	1 if the event is handled 0 otherwise
 */
int text_field_keypress (widget_list *w, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

/*!
 * \ingroup widgets
 *
 * \brief Force a text field to rewrap the lines.
 *
 * Force the textfield identified by window ID \a window_id and widget ID
 * \a widget_id, to recalculate the positions of the soft line breaks. This is
 * done e.g. in situations where the font or font size is changed.
 *
 * \param window_id The identifier for the window the text field resides in
 * \param widget_id The identifier for the text field widget
 */
void text_field_force_rewrap(int window_id, Uint32 widget_id);

//FIXME: Write documentation for these...
#define P_NORMAL    0
#define P_TEXT      1
#define P_NONE      2

int pword_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, unsigned char *buffer, int buffer_size);
int pword_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, float size, unsigned char *buffer, int buffer_size);
int pword_field_set_content(int window_id, Uint32 widget_id, const unsigned char* buf, size_t len);
/*!
 * \ingroup widgets
 *
 * \brief Set the shadow color of the text
 *
 * Set the shadow color of the text in the password field identified by window ID \a window_id
 * and widget ID \a widget_id, to \a r, \a g, \a b. If this function is not called, or \a r < 0,
 * no shadow is drawn.
 *
 * \param window_id The window identifier for the password field
 * \param widget_id The widget identifier for the password field
 * \param r         The red component of the shadow color
 * \param g         The green component of the shadow color
 * \param b         The blue component of the shadow color
 * \return 1 on success, 0 on failure (widget not found)
 */
int pword_field_set_shadow_color(int window_id, Uint32 widget_id, float r, float g, float b);
void pword_set_status(widget_list *w, Uint8 status);
int pword_clear(int window_id, Uint32 widget_id);

int multiselect_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, int width);
int multiselect_add_extended(int window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, int width, Uint16 max_height, float size, float r, float g, float b, float hr, float hg, float hb, int max_buttons);
int multiselect_button_add(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, const char *text, const char selected);
int multiselect_button_add_extended(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, int width, const char *text, float size, const char selected);
int multiselect_get_selected(int window_id, Uint32 widget_id);
int multiselect_set_selected(int window_id, Uint32 widget_id, int button_id);
int multiselect_get_height(int window_id, Uint32 widget_id);
int multiselect_clear(int window_id, Uint32 widget_id);

#define SPIN_FLOAT 0
#define SPIN_INT 1

int spinbutton_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval);
int spinbutton_add_extended(int window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval, float size);

/*!
 * \ingroup	widgets
 * \brief 	Handles a mouseover event
 *
 * 		Handles a mousover event for a widget.
 *
 * \param   	widget pointer to the widget structure
 * \param   	mx the mouse x position relative to the widgets origin
 * \param   	my the mouse y position relative to the widgets origin
 * \retval 	1 if the event is handled, 0 otherwise
 */
int widget_handle_mouseover (widget_list *widget, int mx, int my);

/*!
 * \ingroup	widgets
 * \brief 	Handles a mouse click event
 *
 * 		Handles a mous click event for a widget.
 *
 * \param   	widget pointer to the widget structure
 * \param   	mx the mouse x position relative to the widgets origin
 * \param   	my the mouse y position relative to the widgets origin
 * \param	flags flags specifying the mouse button and modifier state
 * \retval 	1 if the event is handled, 0 otherwise
 */
int widget_handle_click (widget_list *widget, int mx, int my, Uint32 flags);

/*!
 * \ingroup	widgets
 * \brief 	Handles a mouse drag event
 *
 * 		Handles a mouse drag event for a widget.
 *
 * \param   	widget pointer to the widget structure
 * \param   	mx the mouse x position relative to the widgets origin
 * \param   	my the mouse y position relative to the widgets origin
 * \param	flags flags specifying the mouse button and modifier state
 * \param   	dx the change in mouse position in the x direction
 * \param   	dy the change in mouse position in the y direction
 * \retval 	1 if the event is handled, 0 otherwise
 */
int widget_handle_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy);

/*!
 * \ingroup	widgets
 * \brief 	Handles a keypess event
 *
 * 		Handles a keypress event for a widget.
 *
 * \param   	widget pointer to the widget structure
 * \param   	mx the mouse x position relative to the widgets origin
 * \param   	my the mouse y position relative to the widgets origin
 * \param	key_code the SDL key code
 * \param	key_unicode the unicode representation of the key pressed
 * \param	key_mod the status bitmask for mod keys
 * \retval 	1 if the event is handled, 0 otherwise
 */
int widget_handle_keypress (widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
/*!
 * \ingroup widgets
 *
 * Handle a change in font
 *
 * Handle a change in font or font size in font category \a cat for widget \a widget.
 *
 * \param widget The widget to handle the font change for.
 * \param cat    The font category that was changed.
 *
 * \return 1 if the widget handled the change, 0 otherwise
 */
int widget_handle_font_change(widget_list *widget, font_cat cat);
/*!
 * \ingroup widgets
 *
 * Handle a paste event
 *
 * Handle a text paste event, and paste text \a text into widget \a widget.
 *
 * \param widget The widget to handle the paste event
 * \param text   The text to paste into the widget
 *
 * \return 1 if the widget handled the change, 0 otherwise
 */
int widget_handle_paste(widget_list *widget, const char* text);



/*!
 * \ingroup	widgets
 * \brief 	A general helper function to draw a cross.
 *
 * 		Draw a scalable cross centred the specified position and of the
 * 	specified size and width.
 *
 * \param	the centre x coordinate of the cross.
 * \param	the centre y coordinate of the cross.
 * \param	half the length in pixels of the cross width/height.
 * \param	half the width in pixels of cross lines
 */
void draw_cross(int centre_x, int centre_y, int half_len, int half_width);


// XML Windows

/*!
 * \ingroup 	xml_windows
 * \brief 	Adds a window from an xml-file.
 *
 * 		Adds a window from an xml-file.
 *
 * \param   	fn The filename
 * \retval int  	Returns 0 on failure and the window_id on succes
 * \callgraph
 */
int AddXMLWindow(char *fn);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
