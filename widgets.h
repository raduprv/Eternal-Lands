/*!
 * \file
 * \brief Functions for the widgets used by EL
 * \ingroup widgets
 */
#ifndef	__WIDGETS_H
#define	__WIDGETS_H

/*!
 * \name    LABEL, IMAGE, CHECKBOX, BUTTON, PROGRESSBAR, VSCROLLBAR
 */
/*! \{ */
#define LABEL		0x01     /*!< LABEL */
#define IMAGE		0x02     /*!< IMAGE */
#define CHECKBOX	0x03     /*!< CHECKBOX */
#define BUTTON		0x04     /*!< BUTTON */
#define PROGRESSBAR	0x05     /*!< PROGRESSBAR */
#define VSCROLLBAR	0x06     /*!< VSCROLLBAR */
/*! \} */


/*!
 * The widget list structure - each window has a widget list.
 */
typedef struct wl{
	// Common widget data
	Uint16 pos_x, pos_y, len_x, len_y; /*!< Widget area */
	Uint32 id;                         /*!< Widget unique id */
	Uint32 type;   /*!< Specifies what kind of widget it is */
	Uint32 Flags;  /*!< Status flags...visible,enbled,etc */
	float size;    /*!< Size of text, image, etc */
	float r, g, b; /*!< Associated color */

	/*! \name The widget handlers */
	/*! \{ */
	int (*OnDraw)();
	int (*OnClick)();
	int (*OnDrag)();
	int (*OnInit)();
	int (*OnMouseover)();
	/*! \} */

	void *widget_info; /*!< Pointer to specific widget data */
	struct wl *next;   /*!< Pointer to the next widget in the window */
}widget_list;


/*!
 * Widget label
 */
typedef struct {
	char text[256]; /*!< Text */
}label;


/*!
 * Image structure
 */
typedef struct {
	float u1,v1,u2,v2; /*!< Texture coordinates */
	int id;            /*!< Texture id */
}image;

/*!
 *  Checkbox structure
 */
typedef struct {
	int checked;
}checkbox;

/*!
 *  Button structure
 */
typedef struct {
	char text[256];
}button;

/*!
 *  Progressbar structure
 */
typedef struct {
	float progress;
}progressbar;

/*!
 *  Vertical scrollbar structure
 */
typedef struct {
	int pos, pos_inc, bar_len;
}vscrollbar;


// Common widget functions

/*!
 * \ingroup	widgets
 * \brief 	Find a widget with the given widget_id
 *
 * 		Returns the widget with the given widget_id in window_id.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \return  	A widget_list pointer to the widget if found, otherwise NULL
 */
widget_list * widget_find(Uint32 window_id, Uint32 widget_id);

/*!
 * \ingroup	widgets
 * \brief 	Sets the widget's draw callback
 *
 * 		Finds the widget in the window and sets the widget's draw callback in the specified window.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The widget's unique ID
 * \param   	handler A function pointer to the handler
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
 */
int widget_set_OnMouseover(Uint32 window_id, Uint32 widget_id, int (*handler)());

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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns 1 on succes or 0 on failure (when the widget was not found in the given window)
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
 * \return  	Returns the new widgets unique ID 
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
 * \return  	Returns the new widgets unique ID 
 * \sa		label_add_extended
 * \callgraph
 */
int label_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

/*!
 * \ingroup	labels
 * \brief	Draws a label
 *
 * 		Draws the label given by the widget.
 *
 * \param   	W The widget that is to be drawn
 * \return  	Returns true
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
 * \return  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
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
 * \return  	Returns the new widgets unique ID 
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
 * \return  	Returns the new widgets unique ID
 * \callgraph
 */
int image_add(Uint32 window_id, int (*OnInit)(), int id, Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, float u1, float v1, float u2, float v2);

/*!
 * \ingroup	images
 * \brief 	Draws the image widget 
 * 
 * 		Draws an image widget as given by the widget *.
 *
 * \param   	W A pointer to the widget that should be drawn
 * \return  	Returns true
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
 * \return  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
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
 * \return  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
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
 * \return  	Returns the new widgets unique ID 
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
 * \return  	Returns the new widgets unique ID
 * \callgraph
 */
int checkbox_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly, int checked);

/*!
 * \ingroup	checkboxes
 * \brief 	Draws a checkbox
 *
 * 		Draws the checkbox pointed to by *W.
 *
 * \param   	W The widget you wish to draw
 * \return  	Returns true
 */
int checkbox_draw(widget_list *W);

/*!
 * \ingroup	checkboxes
 * \brief 	The callback function for the checkbox
 *
 * 		When the checkbox is clicked this function will be called.
 *
 * \param   	W The widget that called the checkbox
 * \return  	Returns true
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
 * \return  	Returns 0 if the checkbox is unchecked, 1 if the checkbox is checked and -1 if the checkbox is not even found.
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
 * \return  	Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
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
 * \return  	Returns the new widgets unique ID 
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
 * \return  	Returns the new widgets unique ID 
 * \callgraph
 */
int button_add(Uint32 window_id, int (*OnInit)(), char *text, Uint16 x, Uint16 y);

/*!
 * \ingroup	buttons
 * \brief 	Draws a button
 *
 * 		Draws the button widget pointed to by W.
 *
 * \param   	W The button widget
 * \return  	Returns true
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
 * \return  Returns 1 on succes, 0 on failure (if the widget is not found in the given window)
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
 * \return  	Returns the new widgets unique ID
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
 * \return  	Returns the new widgets unique ID 
 * \callgraph
 */
int progressbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \ingroup	progressbars
 * \brief 	Draws a progressbar
 *
 * 		The function draws the progressbar pointed to by *W
 *
 * \param   	W The progressbar widget that is going to be drawn
 * \return  	Returns false
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
 * \return  	Returns -1 on failure, otherwise the current progress.
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
 * \return  	Returns 1 on succes, 0 on failure (if the widget_id was not found in that window).
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
 * \return  	Returns the new widgets unique ID 
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
 * \return  	Returns the new widgets unique ID 
 * \callgraph
 */
int vscrollbar_add(Uint32 window_id, int (*OnInit)(), Uint16 x, Uint16 y, Uint16 lx, Uint16 ly);

/*!
 * \ingroup	scrollbars
 * \brief 	Draws a vertical scrollbar
 *
 * 		Draws the vertical scrollbar given by *W
 *
 * \param   	W A pointer to the vertical scrollbar widget you wish to draw
 * \return  	Returns false
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
 * \return  	Returns true
 */
int vscrollbar_click(widget_list *W, int x, int y);

/*!
 * \ingroup	scrollbars
 * \brief 	The callback for dragging the vertical scrollbar widget
 *
 * 		The callback for dragging the vertical scrollbar widget
 *
 * \param   	W The vertical scrollbar widget
 * \param   	dx Specifies the delta x
 * \param   	dy Specifies the delta y
 * \return  	Returns true
 */
int vscrollbar_drag(widget_list *W, int dx, int dy);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and sets the position.
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \param   	pos_inc The position increase (or decrease)
 * \return  	Returns 1 on succes, 0 on failure (if the widget was not found in the given window)
 */
int vscrollbar_set_pos_inc(Uint32 window_id, Uint32 widget_id, int pos_inc);

/*!
 * \ingroup	scrollbars
 * \brief 	Sets the position of the vertical scrollbar
 *
 * 		Finds the vertical scrollbar widget and returns the position
 *
 * \param   	window_id The location of the window in the windows_list.window[] array
 * \param   	widget_id The unique widget ID
 * \return  	Returns pos on succes, -1 on failure (if the widget was not found in the given window)
 */
int vscrollbar_get_pos(Uint32 window_id, Uint32 widget_id);


// XML Windows

/*!
 * \ingroup 	xml_windows
 * \brief 	Adds a window from an xml-file.
 *
 * 		Adds a window from an xml-file.
 *
 * \param   	fn The filename
 * \return  	Returns 0 on failure and the window_id on succes
 * \callgraph
 */
int AddXMLWindow(char *fn);

/*!
 * \ingroup	xml_windows
 * \brief 	Reads an xml-file and parses the window/widget data
 *
 * 		Reads an xml-file and parses the window/widget data
 *
 * \param   	a_node The xmlNode to be parsed.
 * \return  	Returns the window position in the windows_list.windows[] array on succes.
 */
int ReadXMLWindow(xmlNode * a_node);

/*!
 * \ingroup	xml_windows
 * \brief 	Parses xml-window data
 *
 * 		The function is called from ReadXMLWindow, then parses the window data.
 *
 * \param   	a_node The xmlNode to be parsed
 * \return  	Returns the window_id in the windows_list.windows array on succes.
 */
int ParseWindow(xmlAttr *a_node);

/*!
 * \ingroup	xml_windows
 * \brief 	Parses xml-widget data
 *
 * 		The function is called from ReadXMLWindow, then parses the widget data
 *
 * \param   	wn The widget name
 * \param   	winid The window ID
 * \param   	a_node The current xmlAttr node
 * \return  	Returns true
 */
int ParseWidget(char *wn, int winid, xmlAttr *a_node);

/*!
 * \ingroup	xml_windows
 * \brief 	Gets the widget type
 *
 * 		Gets the widget type from the name
 *
 * \param   	w The Widget's name
 * \return 	Returns the type of widget on succes, 0 on failure.
 */
int GetWidgetType(char *w);
#endif

