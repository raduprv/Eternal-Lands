/*!
 * \file
 * \ingroup widgets
 * \brief Functions for the widgets used by EL
 */
#ifndef	__WIDGETS_H
#define	__WIDGETS_H

#include <SDL_types.h>
#include "text.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Sint8 label[64];
	int content_id;
	Uint16 tag_width;
	float label_r, label_g, label_b;
	char closable;
} tab;

typedef struct {
	int tag_height, button_size;
	int nr_tabs, max_tabs, cur_tab, tab_offset, tab_last_visible;
	tab *tabs;
} tab_collection;

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
    // We can conceivably store other generic info here too
} ;

/*!
 * These are the type declarations for widgets.c
  */
extern const struct WIDGET_TYPE label_type;
extern const struct WIDGET_TYPE image_type;
extern const struct WIDGET_TYPE checkbox_type;
extern const struct WIDGET_TYPE round_button_type;
extern const struct WIDGET_TYPE square_button_type;
extern const struct WIDGET_TYPE progressbar_type;
extern const struct WIDGET_TYPE vscrollbar_type;
extern const struct WIDGET_TYPE tab_collection_type;
extern const struct WIDGET_TYPE text_field_type;
extern const struct WIDGET_TYPE pword_field_type;
extern const struct WIDGET_TYPE multiselect_type;
extern const struct WIDGET_TYPE spinbutton_type;

// Type Conversion Function - TODO : Document'
int widget_set_type (int window_id, Uint32 widget_id, const struct WIDGET_TYPE *type);

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

#ifdef NEW_NEW_CHAR_WINDOW
/*!
 * \name	Flags for the buttons
 */
/*! \{ */
#define BUTTON_ACTIVE 0x400
#endif

/*!
 * \name	Flags for the text field
 */
/*! \{ */
#define TEXT_FIELD_BORDER	0x01
#define TEXT_FIELD_EDITABLE	0x02
#define TEXT_FIELD_NO_KEYPRESS	0x04
#define TEXT_FIELD_CAN_GROW	0x08
#define TEXT_FIELD_SCROLLBAR	0x10
#define TEXT_FIELD_IGNORE_RETURN 0x20
#define TEXT_FIELD_MOUSE_EDITABLE 0x200
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
typedef struct
{
	text_field_line* lines;
	int sm, sc, em, ec;
} select_info;

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

#ifdef NEW_NEW_CHAR_WINDOW
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
 * \param   r (0<=r<=1)
 * \param   g (0<=g<=1)
 * \param   b (0<=b<=1)
 * \param   type The widget type
 * \param   T Pointer to specific widget data
 * \param   S Pointer to specific implementation info
 * \retval int Returns the new widgets unique ID 
 */
Uint32 widget_add (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y,
	Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b,
	const struct WIDGET_TYPE *type, void *T, void *S);
#endif

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
 * 		Finds the widget in the given window and sets the r g b colour.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_color(int window_id, Uint32 widget_id, float r, float g, float b);

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
 * \param   	lx The width 
 * \param   	ly The height
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
int label_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint32 Flags, float size, float r, float g, float b, const char *text);

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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
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
int image_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int id, float u1, float v1, float u2, float v2, float alpha);

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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param   	checked Specifies if the widget is checked or not
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa checkbox_add
 */
int checkbox_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int *checked);

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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param   	text The button label
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa button_add
 */
int button_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, const char *text);

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
 * \ingroup	buttons
 * \brief 	Draws a smooth button
 *
 * 		Draws the smooth button widget pointed to by W.
 *
 * \param   	W The button widget
 * \retval int  	Returns true
 * \callgraph
 */
int button_draw(widget_list *W);

/*!
 * \ingroup	buttons
 * \brief 	Draws a square button
 *
 * 		Draws the square button widget pointed to by W.
 *
 * \param   	W The button widget
 * \retval int  	Returns true
 * \callgraph
 */
int square_button_draw(widget_list *W);

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
int button_set_text(int window_id, Uint32 widget_id, char *text);



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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param   	progress The current progress
 * \param     colors The colors of the four corners of the bar. Pointer to an array of 12 floats (4 consecutive RGB colors). May be NULL.
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa progressbar_add
 */
int progressbar_add_extended(int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress, const float * colors);

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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param   	pos
 * \param   	pos_inc
 * \param		bar_len
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa vscrollbar_add
 */
int vscrollbar_add_extended(int window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc, int bar_len);

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
 * \param	tag_height The height of the tags
 * \param	tag_space The spacing between two neigboring tags
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa tab_collection_add_extended
 */
int tab_collection_add (int window_id, int (*OnInit)(), Uint16 x, 
Uint16 y, Uint16 lx, Uint16 ly, Uint16 tag_height);

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
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param	max_tabs The largest number of tabs this collection will hold
 * \param	tag_height The height of the tags
 * \param	tag_space The spacing between two neigboring tags
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa tab_collection_add
 */
int tab_collection_add_extended (int window_id, Uint32 wid, int 
(*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, 
float size, float r, float g, float b, int max_tabs, Uint16 tag_height);

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
 * \param   	size The text size
 * \param   	r (0<=r<=1)
 * \param   	g (0<=g<=1)
 * \param   	b (0<=b<=1)
 * \param	buf the text buffer
 * \param 	buf_size the size of the text buffer
 * \param	chan_filt the channel of which messages are drawn
 * \param	x_space the number of pixels in the x-direction between the border and the text
 * \param	y_space the number of pixels in the y-direction between the border and the text
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa text_field_add
 */
int text_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, text_message *buf, int buf_size, Uint8 chan_filt, int x_space, int y_space);

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
 * \param	key the SDL key code
 * \param	unikey the unicode representation of the key pressed
 * retval	1 if the event is handled 0 otherwise
 */
int text_field_keypress (widget_list *w, int mx, int my, Uint32 key, Uint32 unikey);

/*!
 * \brief set cursor_line according to cursor position in current message.
 *
 * \param[in] tf text_field where we set cursor_line.
 */
void text_field_find_cursor_line(text_field* tf);


//FIXME: Write documentation for these...
#define P_NORMAL    0
#define P_TEXT      1
#define P_NONE      2

int pword_keypress (widget_list *w, int mx, int my, Uint32 key, Uint32 unikey);
unsigned char * pword_field_get(widget_list *w);
int pword_field_add (int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, unsigned char *buffer, int buffer_size);
int pword_field_add_extended (int window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 status, float size, float r, float g, float b, unsigned char *buffer, int buffer_size);
int pword_field_click(widget_list *w, int mx, int my, Uint32 flags);
void pword_set_status(widget_list *w, Uint8 status);

int multiselect_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, int width);
int multiselect_add_extended(int window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, int width, Uint16 max_height, float size, float r, float g, float b, float hr, float hg, float hb, int max_buttons);
int multiselect_button_add(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, const char *text, const char selected);
int multiselect_button_add_extended(int window_id, Uint32 multiselect_id, Uint16 x, Uint16 y, int width, const char *text, float size, const char selected);
int multiselect_get_selected(int window_id, Uint32 widget_id);
int multiselect_set_selected(int window_id, Uint32 widget_id, int button_id);
int multiselect_get_height(int window_id, Uint32 widget_id);

#define SPIN_FLOAT 0
#define SPIN_INT 1

int spinbutton_add(int window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval);
int spinbutton_add_extended(int window_id, Uint32 widget_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint8 data_type, void *data, float min, float max, float interval, float size, float r, float g, float b);

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
 * \param	key the SDL key code
 * \param	unikey the unicode representation of the key pressed
 * \retval 	1 if the event is handled, 0 otherwise
 */
int widget_handle_keypress (widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey);

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

/*!
 * \brief checks if message fits the filter.
 *
 * \param[in] msg message to test.
 * \param[in] filter filter.
 *
 * \return 1 if message doesnt fit filter (should be skipped), 0 otherwise.
 */
int skip_message (const text_message *msg, Uint8 filter);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
