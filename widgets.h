/*!
 * \file
 * \ingroup widgets
 * \brief Functions for the widgets used by EL
 */
#ifndef	__WIDGETS_H
#define	__WIDGETS_H

#include "text.h"

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
	Uint32 type;   /*!< Specifies what kind of widget it is */
	Uint32 Flags;  /*!< Status flags...visible,enbled,etc */
	float size;    /*!< Size of text, image, etc */
	float r, g, b; /*!< Associated color */
    /*! @} */

	/*! \name The widget handlers */
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


/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// * Widget label
// */
//typedef struct {
//	char text[256]; /*!< Text */
//}label;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// * Image structure
// */
//typedef struct {
//	float u1,v1,u2,v2; /*!< Texture coordinates */
//	int id;            /*!< Texture id */
//}image;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Checkbox structure
// */
//typedef struct {
//	int checked;
//}checkbox;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Button structure
// */
//typedef struct {
//	char text[256];
//}button;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Progressbar structure
// */
//typedef struct {
//	float progress;
//}progressbar;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Vertical scrollbar structure
// */
//typedef struct {
//	int pos, pos_inc, bar_len;
//}vscrollbar;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Tabbed window structure
// */
//typedef struct {
//	Sint8 label[64];
//	Uint16 tag_width;
//	Uint32 content_id;
//} tab;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in widgets.c, no need to declare it here.
 */
//*!
// *  Tab collection structure
// */
//typedef struct {
//	int tag_height, tag_space, nr_tabs, max_tabs, cur_tab;
//	tab *tabs;
//} tab_collection;

/*!
 * \name	Flags for the text field
 */
/*! \{ */
#define TEXT_FIELD_BORDER	0x01
#define TEXT_FIELD_EDITABLE	0x02
/*! \} */

/*!
 * Text field structure
 */
typedef struct
{
	int msg, offset, cursor;
	float text_r, text_g, text_b;
	int buf_size, buf_fill;
	text_message *buffer;
	int chan_nr;
	Uint16 x_space, y_space;
} text_field;

// Common widget functions

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
widget_list * widget_find(Uint32 window_id, Uint32 widget_id);

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
int widget_destroy (Uint32 window_id, Uint32 widget_id);

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
int widget_set_OnDraw(Uint32 window_id, Uint32 widget_id, int (*handler)());

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
int widget_set_OnClick(Uint32 window_id, Uint32 widget_id, int (*handler)());

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's on-drag handler
 *
 * 		Finds the widget in the window and sets the widget's on-click handler
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler
 * \retval int  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 *
 * \sa widget_find
 */
int widget_set_OnDrag(Uint32 window_id, Uint32 widget_id, int (*handler)());

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
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());

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
int widget_set_OnKey ( Uint32 window_id, Uint32 widget_id, int (*handler)() );

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
int widget_move(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);

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
int widget_resize(Uint32 window_id, Uint32 widget_id, Uint16 x, Uint16 y);

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
int widget_set_flags(Uint32 window_id, Uint32 widget_id, Uint32 f);

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
int widget_set_size(Uint32 window_id, Uint32 widget_id, float size);

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
int widget_set_color(Uint32 window_id, Uint32 widget_id, float r, float g, float b);



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
int label_add_extended(Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);

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
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

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
int label_set_text(Uint32 window_id, Uint32 widget_id, char *text);



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
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa image_add
 */
int image_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int id, float u1, float v1, float u2, float v2);

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
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2);

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
int image_set_id(Uint32 window_id, Uint32 widget_id, int id);

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
int image_set_uv(Uint32 window_id, Uint32 widget_id, float u1, float v1, float u2, float v2);



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
 * \param   	checked Specified if the widget is checked or not
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa checkbox_add
 */
int checkbox_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int checked);

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
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int checked);

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
 * \brief 	The callback function for the checkbox
 *
 * 		When the checkbox is clicked this function will be called.
 *
 * \param   	W The widget that called the checkbox
 * \retval int  	Returns true
 */
int checkbox_click(widget_list *W);

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
int checkbox_get_checked(Uint32 window_id, Uint32 widget_id);

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
int checkbox_set_checked(Uint32 window_id, Uint32 widget_id, int checked);



// Button

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
int button_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, char *text);

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
int button_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

/*!
 * \ingroup	buttons
 * \brief 	Draws a button
 *
 * 		Draws the button widget pointed to by W.
 *
 * \param   	W The button widget
 * \retval int  	Returns true
 * \callgraph
 */
int button_draw(widget_list *W);

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
int button_set_text(Uint32 window_id, Uint32 widget_id, char *text);



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
 * \retval int  	Returns the new widgets unique ID
 *
 * \sa progressbar_add
 */
int progressbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, float progress);

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
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

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
float progressbar_get_progress(Uint32 window_id, Uint32 widget_id);

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
int progressbar_set_progress(Uint32 window_id, Uint32 widget_id, float progress);



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
int vscrollbar_add_extended(Uint32 window_id, Uint32 wid,  int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, int pos, int pos_inc, int bar_len);

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
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

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
 * \brief 	The callback for mouseclicks in the vertical scrollbar widget
 *
 * 		The callback for mouseclicks in the vertical scrollbar widget
 *
 * \param   	W The widget
 * \param   	x The mouse x position
 * \param   	y The mouse y position
 * \retval int  	Returns true
 */
int vscrollbar_click(widget_list *W, int x, int y);

/*!
 * \ingroup	scrollbars
 * \brief 	The callback for dragging the vertical scrollbar widget
 *
 * 		The callback for dragging the vertical scrollbar widget
 *
 * \param   	W The vertical scrollbar widget
 * \param	x Specifies the x pos
 * \param	y Specifies the y pos
 * \param   	dx Specifies the delta x
 * \param   	dy Specifies the delta y
 * \retval int  	Returns true
 *
 * \sa vscrollbar_click
 */
int vscrollbar_drag(widget_list *W, int x, int y, int dx, int dy);

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
int vscrollbar_set_pos_inc(Uint32 window_id, Uint32 widget_id, int pos_inc);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets the position.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	pos The new position
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_set_pos(Uint32 window_id, Uint32 widget_id, int pos);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the logical length of vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets its logical bar length.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	bar_len The new logical bar length
 * \retval int  	Returns 1 on succes, 0 on failure (if the widget was not found in the given window)
 *
 * \sa widget_find
 */
int vscrollbar_set_bar_len (Uint32 window_id, Uint32 widget_id, int bar_len);

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
int vscrollbar_get_pos(Uint32 window_id, Uint32 widget_id);


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
int tab_collection_get_tab (Uint32 window_id, Uint32 widget_id);

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
int tab_collection_get_tab_id (Uint32 window_id, Uint32 widget_id);

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
int tab_set_label_color_by_id (Uint32 window_id, Uint32 col_id, Uint32 tab_id, float r, float g, float b);

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
int tab_collection_select_tab (Uint32 window_id, Uint32 widget_id, int tab);

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
int tab_collection_add (Uint32 window_id, int (*OnInit)(), Uint16 x, 
Uint16 y, Uint16 lx, Uint16 ly, Uint16 tag_height, Uint16 tag_space);

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
int tab_collection_add_extended (Uint32 window_id, Uint32 wid, int 
(*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, 
float size, float r, float g, float b, int max_tabs, Uint16 tag_height, Uint16 tag_space);

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
 * \brief 	The callback for mouseclicks in the tabbed window collection widget
 *
 * 		The callback for mouseclicks in the tabbed window collection widget
 *
 * \param   	W The widget
 * \param   	x The mouse x position
 * \param   	y The mouse y position
 * \retval int  	Returns 1 if a new tab is selected, 0 otherwise
 * \callgraph
 */
int tab_collection_click (widget_list *W, int x, int y);

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
 * \retval int  	Returns 1 if a new tab is selected, 0 otherwise
 * \callgraph
 */
int tab_add (Uint32 window_id, Uint32 col_id, const char *label, Uint16 tag_width, int closable);

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
 * \param	chan_nr the channel of which messages are drawn
 * \param	x_space the number of pixels in the x-direction between the border and the text
 * \param	y_space the number of pixels in the y-direction between the border and the text
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa text_field_add_extended
 */
int text_field_add (Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, text_message *buf, int buf_size, int x_space, int y_space);

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
 * \param	x_space the number of pixels in the x-direction between the border and the text
 * \param	y_space the number of pixels in the y-direction between the border and the text
 * \param	text_r red component of the text color, or -1.0 for default
 * \param	text_g green component of the text color, or -1.0 for default
 * \param	text_b blue component of the text color, or -1.0 for default
 * \retval int  	Returns the new widgets unique ID 
 *
 * \sa text_field_add
 */
int text_field_add_extended (Uint32 window_id, Uint32 wid, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, Uint32 Flags, float size, float r, float g, float b, text_message *buf, int buf_size, int chan_nr, int x_space, int y_space, float text_r, float text_g, float text_b);

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
int text_field_set_buf_pos (Uint32 window_id, Uint32 widget_id, int msg, int offset);

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
int text_field_set_text_color (Uint32 window_id, Uint32 widget_id, float r, float g, float b);

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

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup	xml_windows
// * \brief 	Reads an xml-file and parses the window/widget data
// *
// * 		Reads an xml-file and parses the window/widget data
// *
// * \param   	a_node The xmlNode to be parsed.
// * \retval int  	Returns the window position in the windows_list.windows[] array on succes.
// * \callgraph
// */
//int ReadXMLWindow(xmlNode * a_node);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup	xml_windows
// * \brief 	Parses xml-window data
// *
// * 		The function is called from ReadXMLWindow, then parses the window data.
// *
// * \param   	node The xmlNode to be parsed
// * \retval int  Returns the window_id in the windows_list.windows array on succes.
// * \callgraph
// */
//int ParseWindow (xmlNode *node);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup	xml_windows
// * \brief 	Parses xml-widget data
// *
// * 		The function is called from ParseWindow or ParseTab, then parses the widget data
// *
// * \param   	node The xmlNode describing the widget
// * \param   	winid The window ID the widget belongs to
// * \retval int  Returns the new widget id 
// * \callgraph
// */
//int ParseWidget (xmlNode *node, int winid);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup	xml_windows
// * \brief 	Parses xml window tab
// *
// * 		The function is called from ParseWidget, then parses the tab data
// *
// * \param   	node The xmlNode describing the tab
// * \param   	winid The window ID to which the tab belongs
// * \param   	colid The widget id of the tab collection the tab belongs to
// * \retval int  Returns the window id of the tab
// * \callgraph
// */
//int ParseTab (xmlNode *node, int winid, int colid);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup	xml_windows
// * \brief 	Gets the widget type
// *
// * 		Gets the widget type from the name
// *
// * \param   	w The Widget's name
// * \retval int 	Returns the type of widget on succes, 0 on failure.
// */
//int GetWidgetType (const char *w);
#endif
