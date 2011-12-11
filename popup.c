/*!
 * \file popup.c
 * \ingroup popup_window
 * \brief Implementation of popup windows
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include <time.h>
#include "popup.h"
#include "chat.h"
#include "console.h"
#include "elwindows.h"
#include "gamewin.h"
#include "init.h"
#include "interface.h"
#include "multiplayer.h"
#include "queue.h"
#include "translate.h"
#include "font.h"
#include "list.h"
#include "asc.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "misc.h"
#include "errors.h"
#include "translate.h"

#undef POPUP_DEBUG

#ifdef POPUP_DEBUG

#define POPUP_FUNC_ENTER \
    fprintf(stderr,"Entering %s\n", __FUNCTION__ )

#define POPUP_FUNC_LEAVE \
    fprintf(stderr,"Leaving %s\n", __FUNCTION__ )

#else /* POPUP_DEBUG */

#define POPUP_FUNC_ENTER /* */
#define POPUP_FUNC_LEAVE /* */

#endif /* POPUP_DEBUG */

static list_node_t *popup_list;

static float popup_font_zoom = 0.8f;
static int popup_position_x = 500/2;
static int popup_position_y = 480/4;
static int button_width = 70;
static int button_height = 30;
    
/* Constant margins */

#define POPUP_TOP_TEXT_LEFT_MARGIN 10
#define POPUP_TOP_TEXT_RIGHT_MARGIN 15
#define POPUP_TOP_TEXT_TOP_MARGIN 10
#define POPUP_TOP_TEXT_BOTTOM_MARGIN 0

#define POPUP_OPTION_TEXT_LEFT_MARGIN 20
#define POPUP_OPTION_TEXT_RIGHT_MARGIN 20
#define POPUP_OPTION_TEXT_TOP_MARGIN 3

#define POPUP_BOTTOM_MARGIN 10

#define RADIO_OFFSET 16
#define POPUP_TEXTENTRY_HEIGHT (4 + (DEFAULT_FONT_Y_LEN*popup_font_zoom))

/* Maximum number of new lines that can be added to
 text while performing text flow */

#define POPUP_MAX_NEWLINES_IN_TEXT 20

/* Forward declarations for window callbacks. */

static int popup_display_handler(window_info *win);
static int popup_close_handler(window_info *win);
static int popup_click_handler(window_info *win, int mx, int my, Uint32 flags);

/* Forward helpers */
static popup_node_t *popup_node_find_by_window( window_info *win );
static popup_t *popup_allocate();
static void popup_free(popup_t*);
static void popup_realize( popup_t *this_popup );
static void popup_recompute_sizes( popup_t *this_popup );
static void popup_create_window( popup_t *this_popup );
static void popup_send_to_server( popup_t *popup );

/* Helper macros */

#define TEXT_HEIGHT( lines ) \
	(int)((float)(lines)*DEFAULT_FONT_Y_LEN*popup_font_zoom)

/* Some helper macros to aid us fetching and validating network data */

#define POPUP_NETWORK_ASSERT(x) \
	if (!(x)) { \
	LOG_ERROR("CAUTION: %s: assertion %s failed\n", __FUNCTION__, #x);\
	return;\
	}

#define DECREASE_SIZE( rsize ) \
	do { \
	POPUP_NETWORK_ASSERT( size >= rsize ); \
	size-=rsize; \
	} while (0);

#define FETCH_U16( var ) \
	do { \
	DECREASE_SIZE(2); \
	var = SDL_SwapLE16( *((Uint16*)payload) ); \
	payload+=2; \
	} while (0);

#define FETCH_SIZESTRING( var ) \
	do { \
	Uint8 stringsize; \
	FETCH_U8(stringsize); \
	DECREASE_SIZE( stringsize ); \
	safe_strncpy( var, (char*)payload, stringsize+1 ); \
	payload+=stringsize; \
	} while (0);

#define FETCH_U8( var ) \
	do { \
	DECREASE_SIZE(1); \
	var = *((Uint8*)payload); \
	payload+=1; \
	} while (0);


/*!
 * \ingroup popup_window
 * \brief Zero a flowing text structure.
 * \param text the flowing text structure
 * \returns nothing
 */

static void flowing_text_zeroe( flowing_text_t *text )
{
	text->str = NULL;
	text->lines = -1;
}

/*!
 * \ingroup popup_window
 * \brief Locate a popup node in the global list using the window pointer
 * \returns The popup node on the global list, or NULL if not found
 */

popup_node_t *popup_node_find_by_window( window_info *win )
{
	popup_node_t *popup_list_entry;

	list_for_each_node( popup_list_entry, popup_list ) {

		if ( POPUP_NODE(popup_list_entry)->win == win->window_id )
			return popup_list_entry;
	}

	return NULL;
}

/*!
 * \ingroup popup_window
 * \brief Locate a popup node in the global list using the popup pointer
 * \returns The popup node on the global list, or NULL if not found
 */

static popup_node_t *popup_node_find_by_popup( popup_t *popup )
{
	popup_node_t *popup_list_entry;

	list_for_each_node( popup_list_entry, popup_list ) {

		if ( POPUP_NODE(popup_list_entry) == popup )
			return popup_list_entry;
	}

	return NULL;
}

/*!
 * \ingroup popup_window
 * \brief Locate a popup node in the global list using the popup ID
 * \returns The popup node on the global list, or NULL if not found
 */

static popup_node_t *popup_node_find_by_id( popup_id_t id )
{
	popup_node_t *popup_list_entry;

	list_for_each_node( popup_list_entry, popup_list ) {

		if ( POPUP_NODE(popup_list_entry)->id == id )
			return popup_list_entry;
	}

	return NULL;
}


/*!
 * \ingroup popup_window
 * \brief Allocate a new popup structure
 * \returns A pointer to new popup structure, or NULL if some error
 */

popup_t *popup_allocate()
{
	popup_t *new_popup = calloc( 1,sizeof(popup_t) );
	if (NULL!=new_popup) {
		flowing_text_zeroe( &new_popup->text );
		new_popup->win = -1;
		new_popup->grouped_options = NULL;
		new_popup->option_click_callback = NULL;
		new_popup->realized = 0;
		new_popup->has_send_button = 0;
		new_popup->button_widget_id = 0;

		/* Hardcoded X size for now. Can be overriden using size_hints */
		new_popup->width = 400;

	}
	return new_popup;
}

/*!
 * \ingroup popup_window
 * \brief Frees a flowing text structure (internally).
 *   This does not free the structure itself, but only the text inside.
 * \param text The pointer to flowing text structure
 * \returns nothing
 */

static void flowing_text_free( flowing_text_t *text )
{
	if ( text->str )
		free(text->str);
}

/*!
 * \ingroup popup_window
 * \brief Set up the size hint for this popup.
 *  This must be called before the popup is realized.
 * \param this_popup The pointer to the popup structure
 * \returns 
 */

static void popup_set_sizehint( popup_t *this_popup, unsigned int size )
{
	if (size>0)
		this_popup->width = size;
}

/*!
 * \brief Frees a popup option structure.
 *   Frees the text structure too.
 * \param this_option The option to be freed
 * \returns Nothing
 */

static void popup_option_free( popup_option_t *this_option )
{
	flowing_text_free( &this_option->text );
	if ( this_option->type == OPTION_TYPE_TEXTENTRY ) {
		if ( this_option->value.str )
            free( this_option->value.str );
	}
	free(this_option);
}

/*!
 * \brief Frees a popup group option structure.
 * Frees the options inside the group too.
 * \param this_option_group The option group to be freed
 * \returns Nothing
 */

static void popup_grouped_option_free( popup_grouped_option_t *this_option_group )
{
	list_destroy_with_func( this_option_group->options, (list_free_func_t)popup_option_free );
	
	free(this_option_group);
}

/*!
 * \brief Frees a popup structure.
 * Frees all of the options too
 * \returns Nothing
 */

static void popup_free(popup_t *popup)
{
	if (popup) {
		if ( popup->grouped_options ) {
            list_destroy_with_func( popup->grouped_options, (list_free_func_t)popup_grouped_option_free );
		}
		if (popup->title)
			free( popup->title );

        flowing_text_free( &popup->text );

		free( popup );
	}
}

/*!
 * \brief Allocate a new popup option structure
 * \returns The new popup option structure, or NULL if some error
 */

static popup_option_t *popup_option_allocate()
{
	popup_option_t *new_popup_option = calloc( 1, sizeof(popup_option_t ));

	if ( new_popup_option ) {
		flowing_text_zeroe( &new_popup_option->text );
	}

    return new_popup_option;
}


static void flowing_text_perform_flow( flowing_text_t *text, int max_length )
{
	float text_width;

	text->lines = reset_soft_breaks (text->str,
									 strlen(text->str),
									 text->str_size_allocated,
									 popup_font_zoom,
									 max_length,
									 NULL,
									 &text_width);

	text->width = (int)text_width;

    text->height = TEXT_HEIGHT( text->lines );

}

/*!
 * \brief Create new popup option structure, with the display text and return values
 * \returns The new popup option structure, or NULL if some error
 */

static popup_option_t *popup_option_create( const char *const text,
                                           popup_option_group_t group,
										   popup_option_value_t value,
										   popup_option_type_t type)
{
	popup_option_t *new_popup_option;
	size_t text_size = strlen(text);

	new_popup_option = popup_option_allocate();

	if (NULL!=new_popup_option)
	{
		/* Alloc more POPUP_MAX_NEWLINES_IN_TEXT chars than needed -
		 this allows for extra POPUP_MAX_NEWLINES_IN_TEXT lines */
        new_popup_option->text.str_size_allocated = text_size+POPUP_MAX_NEWLINES_IN_TEXT;
		new_popup_option->text.str = calloc( new_popup_option->text.str_size_allocated , sizeof (char));

		if (NULL==new_popup_option->text.str) {
            popup_option_free( new_popup_option );
			return NULL;
		}

		safe_strncpy(new_popup_option->text.str, text, (strlen(text)+1) * sizeof(char));

		new_popup_option->group = group;
		new_popup_option->value = value;
		new_popup_option->selected = 0;
        new_popup_option->type = type;
	}
	return new_popup_option;
}

/*!
 * \brief Init the popup structures
 * \returns Nothing, as usual
 */

void popup_init()
{
	popup_list = NULL;
}
/*!
 * \ingroup popup_window
 * \brief Finish the popup setup. Creates the popup window itself.
 * \param this_popup The popup we're dealing with
 * \returns Nothing
 */

void popup_finish_setup( popup_t *this_popup )
{
	if (!this_popup->realized)
        popup_realize( this_popup );
}

popup_grouped_option_t *popup_grouped_option_create( popup_option_group_t id, popup_option_type_t type)
{
	popup_grouped_option_t *this_group = calloc( 1, sizeof(popup_grouped_option_t) );
	if (this_group) {
		this_group->options = NULL;
		this_group->group_id = id;
		this_group->type = type;
	}
    return this_group;
}

/*!
 * \ingroup popup_window
 * \brief locate a popup option group by id, or create if not found
 */
popup_grouped_option_t *popup_find_or_create_group(popup_t *this_popup,
												   popup_option_group_t id,
												   popup_option_type_t type)
{
	list_node_t *this_option_group_node;
	popup_grouped_option_t *this_group;

	list_for_each_node( this_option_group_node, this_popup->grouped_options ) {

		this_group = POPUP_GROUP_NODE(this_option_group_node);

		if ( this_group->group_id == id ) {
            return this_group;
		}
	}
	/* Not found, create it */

	this_group = popup_grouped_option_create( id, type );

	list_append( &this_popup->grouped_options, this_group );

    return this_group;
}

/*!
 * \ingroup popup_window
 * \brief Add a text option to a popup, with associated return values.
 * \param this_popup The popup we're dealing with
 * \param group_id The group ID for this option
 * \param text The option text
 * \param value The value to be sent back to server
 * \returns Nothing
 */

void popup_add_option(popup_t *this_popup,
					  popup_option_group_t group_id,
					  const char *const text,
					  popup_option_value_t value)
{
	popup_option_t *new_popup_option =  popup_option_create( text, group_id, value, OPTION_TYPE_TEXTOPTION );

	popup_grouped_option_t *this_group = popup_find_or_create_group( this_popup,
																	group_id,
																	OPTION_TYPE_TEXTOPTION );

	if (NULL!=new_popup_option) {
        list_append(&this_group->options, new_popup_option);
	}
}

/*!
 * \ingroup popup_window
 * \brief Add an option to a popup, with associated return values.
 * \param this_popup The popup we're dealing with
 * \param group_id The group ID for this option
 * \param text The option text
 * \param value The value to be sent back to server
 * \param type The type of the option
 * \returns Nothing
 */
static void popup_add_option_extended(popup_t *this_popup,
									  popup_option_group_t group_id,
									  const char *const text,
									  popup_option_value_t value,
									  popup_option_type_t type)
{
	popup_option_t *new_popup_option =  popup_option_create( text, group_id, value, type );

	popup_grouped_option_t *this_group = popup_find_or_create_group( this_popup, group_id, type );

	if (NULL!=new_popup_option) {
		list_append(&this_group->options, new_popup_option);
	}
}

/*!
 * \ingroup popup_window
 * \brief Add a radio option to a popup, with associated return values.
 * \param this_popup The popup we're dealing with
 * \param group_id The group ID for this option
 * \param text The option text
 * \param value The value to be sent back to server
 * \returns Nothing
 */

void popup_add_option_radio(popup_t *this_popup,
							popup_option_group_t group_id,
							const char *const text,
							popup_option_value_t value)
{
	popup_add_option_extended( this_popup, group_id, text, value, OPTION_TYPE_RADIOOPTION );
    this_popup->has_send_button = 1;
}

/*!
 * \ingroup popup_window
 * \brief Add a text entry to a popup.
 * \param this_popup The popup we're dealing with
 * \param group_id The group ID for this option
 * \param text The option text
 * \returns Nothing
 */

void popup_add_option_textentry(popup_t *this_popup,
								popup_option_group_t group_id,
								const char *const text)
{
	popup_option_value_t dummy_value;
	dummy_value.str = calloc( 256, sizeof(char) );

	popup_add_option_extended( this_popup, group_id, text, dummy_value, OPTION_TYPE_TEXTENTRY );
    this_popup->has_send_button = 1;
}

/*!
 * \ingroup popup_window
 * \brief Add a text to a popup
 * \param this_popup The popup we're dealing with
 * \param group_id The group ID for this option
 * \param text The text
 * \returns Nothing
 */

void popup_add_option_displaytext(popup_t *this_popup,
								  popup_option_group_t group_id,
								  const char *const text)
{
	popup_option_value_t dummy_value;
	popup_add_option_extended( this_popup, group_id, text, dummy_value, OPTION_TYPE_DISPLAYTEXT );
}

/*!
 * \brief Helper: check if the mouse is under the specified rectangle
 * \returns 0 if not, !0 otherwise
 */

static __inline__ int is_mouse_over(window_info *win, int x, int y, int w, int h)
{
	return ( mouse_x>win->cur_x+x && mouse_x<(win->cur_x+x+w) &&
			mouse_y>y+win->cur_y && mouse_y<(y+win->cur_y+h) );
}

static void draw_circle_pure(float x, float y, float radius, int interval, int angle_from, int angle_to)
{
	const float mul=M_PI/180.0f;
	int angle;

	if(radius==0){
		glVertex2f(x, y);
	} else if(interval>0){
		for(angle=angle_from;angle<angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f((float)x+cos(rad)*radius, (float)y+sin(rad)*radius);
		}
	} else { 
		for(angle=angle_from;angle>angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f((float)x+cos(rad)*radius, (float)y+sin(rad)*radius);
		}
	}
}


/*!
 * \brief Display the specific popup, on the specific window
 * This method is called by display_handler, after locating the
 * specific popup structure
 * \returns 1
 */

static int popup_display_object( popup_t *this_popup, window_info *win )
{
	float half_text_height = 5;
    POPUP_FUNC_ENTER;

	if ( this_popup->text.str ) {
		glColor3f(0.3,0.6,1.0);

		draw_string_zoomed(POPUP_TOP_TEXT_LEFT_MARGIN,
						   POPUP_TOP_TEXT_TOP_MARGIN,
						   (unsigned char*)this_popup->text.str,
						   100, /* Max lines */
						   popup_font_zoom);
	}

	/* Draw options, if present */

	/* Iterate over the groups */

	if ( this_popup->grouped_options ) {

		list_node_t *this_group_node;

		list_for_each_node( this_group_node, this_popup->grouped_options ) {

			popup_grouped_option_t *this_group = POPUP_GROUP_NODE(this_group_node);

			if ( this_group->options ) {
				list_node_t *this_option_node;

				list_for_each_node( this_option_node, this_group->options ) {

					popup_option_t *this_option = POPUP_OPTION_NODE(this_option_node);

                    int offset_for_radio = 0;

					/* hack - if we have a send button, demote text options to radio options */
					if (this_popup->has_send_button && this_option->type == OPTION_TYPE_TEXTOPTION ) {

						this_group->type = OPTION_TYPE_RADIOOPTION;
                        this_option->type = OPTION_TYPE_RADIOOPTION;
					}
					
					if ( this_group->type == OPTION_TYPE_RADIOOPTION ) {
						offset_for_radio += RADIO_OFFSET;
					}

					if ( this_option->type == OPTION_TYPE_DISPLAYTEXT || this_option->type == OPTION_TYPE_TEXTENTRY )
					{
						glColor3f(0.3,0.6,1.0);
					} else {
						if ( is_mouse_over( win, POPUP_OPTION_TEXT_LEFT_MARGIN - offset_for_radio,
										   this_option->computed_y_pos,
										   this_option->text.width + POPUP_OPTION_TEXT_LEFT_MARGIN,
										   this_option->text.height ) ) {
							glColor3f(1.0,1.0,1.0);
						}
						else {
							glColor3f(0.6,0.3,1.0);
						}
					}


					if ( this_option->type == OPTION_TYPE_DISPLAYTEXT || this_option->type == OPTION_TYPE_TEXTENTRY ) {
						draw_string_zoomed(POPUP_TOP_TEXT_LEFT_MARGIN,
										   this_option->computed_y_pos,
										   (unsigned char*)this_option->text.str,
										   100,
										   popup_font_zoom);
					} else {
						draw_string_zoomed(POPUP_OPTION_TEXT_LEFT_MARGIN,
										   this_option->computed_y_pos,
										   (unsigned char*)this_option->text.str,
										   100,
										   popup_font_zoom);
					}

					switch ( this_group->type ) {

					case OPTION_TYPE_RADIOOPTION:
						glDisable(GL_TEXTURE_2D);

						glBegin(GL_LINE_LOOP);
						draw_circle_pure(10.0f, (float)this_option->computed_y_pos+half_text_height+1.0, half_text_height, 8,0,360);
						glEnd();

						/* If selected, draw the inner cicle */
						if ( this_option->selected ) {
							glBegin(GL_POLYGON);
							draw_circle_pure(10.0f, (float)this_option->computed_y_pos+half_text_height+1.0, half_text_height-2.0f, 8,0,360);
							glEnd();
						}
						glEnable(GL_TEXTURE_2D);
					default:
                        break;
					}
				}
			}
		}
	}

	glColor3f(0.77f,0.57f,0.39f);
    POPUP_FUNC_LEAVE;
	return 1;
}

static void popup_realize( popup_t *this_popup )
{
    POPUP_FUNC_ENTER;
	popup_recompute_sizes( this_popup );
	popup_create_window( this_popup );
    POPUP_FUNC_LEAVE;
}

/*!
 * \brief Recompute sizes for the popup. This must be called after all
 * text is fully set up, otherwise the window size may not match.
 * \returns Nothing
 */

static void popup_recompute_sizes( popup_t *this_popup )
{

	int current_y = POPUP_TOP_TEXT_TOP_MARGIN;
	popup_option_node_t *this_option_node;
	list_node_t *this_option_group_node;

	POPUP_FUNC_ENTER;

	if ( this_popup->text.str ) {
		flowing_text_perform_flow(&this_popup->text,
								  this_popup->width - (POPUP_TOP_TEXT_LEFT_MARGIN+POPUP_TOP_TEXT_RIGHT_MARGIN)
								 );

		current_y += this_popup->text.height;
	}

	/* Iterate over all groups */

	list_for_each_node( this_option_group_node, this_popup->grouped_options)
	{
		/* And all options in group */

		list_for_each_node(this_option_node, POPUP_GROUP_NODE(this_option_group_node)->options)
		{
			popup_option_t *this_option = POPUP_OPTION_NODE(this_option_node);

			this_option->computed_y_pos = current_y;

			if ( this_option->type == OPTION_TYPE_DISPLAYTEXT || this_option->type == OPTION_TYPE_TEXTENTRY ) {
				flowing_text_perform_flow(
										  &this_option->text,
										  this_popup->width - (POPUP_TOP_TEXT_LEFT_MARGIN+POPUP_TOP_TEXT_RIGHT_MARGIN)
										 );
			} else {
				flowing_text_perform_flow(
										  &this_option->text,
										  this_popup->width - (POPUP_OPTION_TEXT_LEFT_MARGIN+POPUP_OPTION_TEXT_RIGHT_MARGIN)
										 );
			}
			current_y += this_option->text.height + POPUP_OPTION_TEXT_TOP_MARGIN;

			if ( this_option->type == OPTION_TYPE_TEXTENTRY )
				current_y += POPUP_TEXTENTRY_HEIGHT;
		}
	}

	this_popup->height = current_y + POPUP_BOTTOM_MARGIN;

	if (this_popup->has_send_button) {
		this_popup->height += button_height + POPUP_BOTTOM_MARGIN;
	}

	POPUP_FUNC_LEAVE;
}



/*!
 * \brief Close the object
 * Callback when someone closes the popup. Frees up the associated structures.
 * \returns 1
 */

static int popup_close_object( popup_node_t *this_popup_node )
{
	/* Destroy and remove from the list */
	list_remove_node_and_free_data( &popup_list, this_popup_node, (list_free_func_t)&popup_free );

	return 1;

}

static void popup_node_destroy( popup_node_t *this_popup_node )
{
	popup_t *this_popup = POPUP_NODE(this_popup_node);

	if ( this_popup->button_widget_id > 0 )
		widget_destroy( this_popup->win, this_popup->button_widget_id );

	destroy_window( this_popup->win );

	list_remove_node_and_free_data( &popup_list, this_popup_node, (list_free_func_t)&popup_free );

}

static int popup_are_all_options_chosen( popup_t *this_popup )
{
	list_node_t *this_group_node;
    list_node_t *this_option_node;
	popup_grouped_option_t *this_group;

	list_for_each_node( this_group_node, this_popup->grouped_options )
	{
		this_group = POPUP_GROUP_NODE(this_group_node);

		if ( this_group->type == OPTION_TYPE_RADIOOPTION ) {
			/* Ensure we have selected at least one option */
			int saw_selected = 0;
			list_for_each_node( this_option_node, this_group->options ) {
				if ( POPUP_OPTION_NODE(this_option_node)->selected ) {
					saw_selected=1;
                    break;
				}
			}

			if (!saw_selected)
                return 0;
		}
	}

    return 1;
}
int popup_send_button_clicked(widget_list *w, int mx, int my, Uint32 flags)
{
    list_node_t *popup_node;

	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	/* Look up widget */
	list_for_each_node( popup_node, popup_list ) {
		popup_t *popup = POPUP_NODE(popup_node);
		if ( popup->has_send_button && popup->button_widget_id == w->id ) {
			/* Found */
			/* Ensure all options were set */
			if ( popup_are_all_options_chosen( popup ) ) {
				popup_send_to_server( popup );
				popup_node_destroy( popup_node );
			}
            break;
		}
	}

    return 1;
}

static void popup_create_window(popup_t *this_popup)
{
	list_node_t *group;

	POPUP_FUNC_ENTER;

	this_popup->win = create_window(this_popup->title,
									game_root_win,
									0,
									popup_position_x,
                                    popup_position_y,
									this_popup->width,
									this_popup->height,
									ELW_WIN_DEFAULT);

	set_window_handler( this_popup->win, ELW_HANDLER_DISPLAY, &popup_display_handler);
	set_window_handler( this_popup->win, ELW_HANDLER_CLOSE, &popup_close_handler);
	set_window_handler( this_popup->win, ELW_HANDLER_CLICK, &popup_click_handler);

	if (this_popup->has_send_button) {
		this_popup->button_widget_id = button_add( this_popup->win, NULL, button_send,
												  this_popup->width/2 - button_width/2,
												  this_popup->height- button_height - POPUP_BOTTOM_MARGIN );

		widget_set_OnClick(this_popup->win,
						   this_popup->button_widget_id,
                           &popup_send_button_clicked
						  );
	}

	/* Place text entries */

	list_for_each_node( group, this_popup->grouped_options ) {
		if ( POPUP_GROUP_NODE(group)->type == OPTION_TYPE_TEXTENTRY ) {
			/* We must only have one entry in this group */
			popup_option_t *option = POPUP_OPTION_NODE(POPUP_GROUP_NODE(group)->options);

			if (option) { /* Just not to crash */
				option->widget_id = pword_field_add_extended ( this_popup->win,
															  POPUP_GROUP_NODE(group)->group_id,
															  NULL,
															  POPUP_TOP_TEXT_LEFT_MARGIN,
															  option->computed_y_pos + option->text.height,
															  this_popup->width - POPUP_TOP_TEXT_LEFT_MARGIN - POPUP_TOP_TEXT_RIGHT_MARGIN,
															  POPUP_TEXTENTRY_HEIGHT,
															  P_TEXT,
															  popup_font_zoom,
															  0.77f, 0.57f, 0.39f,
															  (unsigned char*)option->value.str,
															  256
															 );
			}
		}
	}

	POPUP_FUNC_LEAVE;
}

popup_t* popup_create( const char *title, popup_id_t id, int persistent )
{
	popup_t *new_popup = popup_allocate();
	if (NULL==new_popup)
		return NULL;

	new_popup->id = id;
	new_popup->win = -1;
	new_popup->realized = 0;
	new_popup->is_persistent = (persistent != 0);
	new_popup->title = calloc( strlen(title)+1, sizeof(char) );
    safe_strncpy( new_popup->title, title, (strlen(title)+1)*sizeof(char) );

	list_push( &popup_list, new_popup );

	return new_popup;
}

/*!
 * \brief Click handler for a popup window
 * Callback called for the specific popup window when the user clicks on it
 * \returns 1
 */

int popup_click_object(popup_t *this_popup, window_info *win, int mx, int my, Uint32 flags)
{
	popup_option_node_t *this_option_node;
	list_node_t *this_option_group_node;
	int offset;
	
	if ( NULL == this_popup->grouped_options )
		return 1;

	list_for_each_node(this_option_group_node, this_popup->grouped_options )
	{
		list_for_each_node(this_option_node, POPUP_GROUP_NODE(this_option_group_node)->options)
		{
			popup_option_t *this_option = POPUP_OPTION_NODE(this_option_node);

			if ( this_option->type == OPTION_TYPE_DISPLAYTEXT ||
				this_option->type == OPTION_TYPE_TEXTENTRY )
				continue;

			offset = POPUP_OPTION_TEXT_LEFT_MARGIN;
			if ( this_option->type == OPTION_TYPE_RADIOOPTION ) {
				offset -= RADIO_OFFSET;
			}

			if ( mx>offset &&
				mx < (int)this_option->text.width + POPUP_OPTION_TEXT_LEFT_MARGIN &&
				my > this_option->computed_y_pos &&
				my < this_option->computed_y_pos + this_option->text.height ) {
    			if (this_popup->option_click_callback) {
					this_popup->option_click_callback( this_popup, this_option );
                    return 1; /* Don't process any more. */
				}
			}
		}
	}
	return 1;
}

/*!
 * \ingroup popup_window
 * \brief Set the text on top of the popup window
 * \param this_popup The popup structure you wish to set text
 * \param text The text to be displayed
 * \returns Nothing
 */
void popup_set_text( popup_t *this_popup, const char *text )
{
    int text_size = strlen(text);

	flowing_text_free( &this_popup->text );

	this_popup->text.str_size_allocated = text_size+POPUP_MAX_NEWLINES_IN_TEXT;
	this_popup->text.str = calloc( this_popup->text.str_size_allocated , sizeof (char));

	if (NULL==this_popup->text.str)
		return;

	safe_strncpy(this_popup->text.str, text, (text_size+1) * sizeof(char));
}

/*!
 * \ingroup popup_window
 * \brief Display handler, to be called by the window.
 *
 *  This method looks up the popup structure based on the window pointer, and
 *  calls the appropriate popup_display_object() method.
 *
 * \param window_info The pointer to window info structure
 * \returns popup_display_object() if popup found, 0 otherwise
 */
static int popup_display_handler(window_info *win)
{
	popup_node_t *the_popup_node = popup_node_find_by_window( win );
	POPUP_FUNC_ENTER;

	if (NULL!=the_popup_node) {
        return popup_display_object( POPUP_NODE(the_popup_node), win );
	}
	return 0; /* Should never get here */
}

/*!
 * \ingroup popup_window
 * \brief Click handler, to be called by the window.
 *
 *  This method looks up the popup structure based on the window pointer, and
 *  calls the appropriate popup_click_object() method.
 *
 * \param window_info The pointer to window info structure
 * \param mx Mouse X pointer
 * \param my Mouse Y pointer
 * \param flags The mouse flags
 * \returns popup_display_object() if popup found, 0 otherwise
 */
static int popup_click_handler(window_info *win,int mx, int my, Uint32 flags)
{
	popup_node_t *the_popup_node = popup_node_find_by_window( win );
	if (NULL!=the_popup_node) {
        return popup_click_object( POPUP_NODE(the_popup_node),win,mx,my,flags );
	}
    return 0; /* Should never get here */
}

/*!
 * \ingroup popup_window
 * \brief Close handler, to be called by the window when it is closed.
 *
 *  This method looks up the popup structure based on the window pointer, and
 *  calls the appropriate popup_close_object() method.
 *
 * \param window_info The pointer to window info structure
 * \returns popup_display_object() if popup found, 0 otherwise
 */
static int popup_close_handler(window_info *win)
{
	popup_node_t *the_popup_node = popup_node_find_by_window( win );
	if (NULL!=the_popup_node) {
		return popup_close_object( the_popup_node );
	}
    return 0;
}

/*!
 * \ingroup popup_window
 * \brief Set callback to be called when user clicks on an option
 *
 *  This method sets the callback which is to be called when user clicks on a specific
 *  option.
 *
 * \param popup_t Pointer to the popup_t structure
 * \param callback The callback to be associated with this popup
 * \returns Nothing
 */
void popup_set_callback( popup_t *popup, popup_callback_t callback )
{
	popup->option_click_callback = callback;
}

/*!
 * \ingroup popup_window
 * \brief Default callback to be called when user clicks on an option
 *
 *  This method is called when user clicks on an option by default
 *
 * \param popup_t Pointer to the popup_t structure
 * \param popup_option_t Pointer to the popup_option_t structure corresponding to the option clicked
 * \returns Nothing
 */
static void popup_click_callback( popup_t *popup, popup_option_t *opt )
{
	if (opt->type == OPTION_TYPE_RADIOOPTION ||
		opt->type == OPTION_TYPE_TEXTOPTION ) {

		/* Reset all entries in the group */
		popup_grouped_option_t *this_group = popup_find_or_create_group( popup, opt->group, opt->type );
		list_node_t *this_option_node;
		list_for_each_node( this_option_node, this_group->options) {
			POPUP_OPTION_NODE(this_option_node)->selected = 0;
		}
		opt->selected = 1;

		if ( ! popup->has_send_button) {
			popup_send_to_server( popup );
			popup_node_destroy( popup_node_find_by_popup( popup ) );
		}

	}
}

/*!
 * \ingroup popup_window
 * \brief Return the selected option in an option group
 *
 * \param popup_grouped_option_t Pointer to the popup_grouped_option_t structure we wish to analise
 * \returns A pointer to popup_option_t structure for the selected option, or NULL if no option is selected.
 */
static popup_option_t *popup_group_get_selected( popup_grouped_option_t *group )
{
	list_node_t *option_node;

	list_for_each_node( option_node, group->options ) {
		if ( POPUP_OPTION_NODE(option_node)->selected ) {
            return POPUP_OPTION_NODE(option_node);
		}
	}
    return NULL;
}

/*!
 * \ingroup popup_window
 * \brief Send popup response back to the server
 *
 * \param popup_t Pointer to the popup__t structure we want to send back data.
 * \returns Nothing
 */
static void popup_send_to_server( popup_t *popup )
{
    /* Max size hardcoded for now */
	unsigned char buffer[8192];
    unsigned char *bptr = buffer;
	list_node_t *group_node;
	popup_option_t *option;

	*bptr = POPUP_REPLY;
    bptr++;

	*((Uint16*)bptr) = SDL_SwapLE16( popup->id );
    bptr+=2;

	list_for_each_node( group_node, popup->grouped_options )
	{
		switch ( POPUP_GROUP_NODE(group_node)->type ) {
		case OPTION_TYPE_DISPLAYTEXT:
			break; /* No option to be sent back */
		case OPTION_TYPE_RADIOOPTION:
		case OPTION_TYPE_TEXTOPTION:
			option = popup_group_get_selected( POPUP_GROUP_NODE(group_node) );
			if (option) {
				*bptr = option->group;
				bptr++;
                *bptr = option->value.uint8;
				bptr++;
			}
			break;
		case OPTION_TYPE_TEXTENTRY:
            option =  POPUP_OPTION_NODE( POPUP_GROUP_NODE(group_node)->options );

			*bptr = option->group;
			bptr++;
			*bptr = 0;
			bptr++;

			*bptr = strlen( option->value.str );
			memcpy( bptr+1, option->value.str, *bptr );
            bptr += 1 + *bptr;
			break;
		default:
			/* Unknown */
            break;
		}
	}
	my_tcp_send(my_socket, buffer, bptr-buffer);

#if 0
	fprintf(stderr,"Sending packet: \n");
	{
		unsigned char *p = buffer;
		while (p<bptr) {
            fprintf(stderr,"%02x ",*p);
            p++;
		}
	}
    fprintf(stderr,"\n");
#endif

}

/*!
 * \ingroup popup_window
 * \brief Create a new popup based on network data received
 *
 * \param payload The network buffer
 * \param size The network buffer size
 * \returns Nothing
 */
void popup_create_from_network( const unsigned char *payload, size_t size )
{
	Uint16 popup_id;
	Uint16 size_hint;
	Uint8 flags;
	char title[256];
	char text[256];
	popup_t *new_popup;
	Uint8 option_type;
	popup_option_group_t option_group;
	popup_option_value_t value_id;

	FETCH_U16( popup_id );
	FETCH_U8( flags );
	FETCH_SIZESTRING( title );
	FETCH_U16( size_hint );
	FETCH_SIZESTRING( text );

	if (flags)
		LOG_ERROR("%s: flags=%d set but not yet supported\n", __FUNCTION__, flags );

	/* Ensure there is no popup with this ID */
	if ( popup_node_find_by_id( popup_id ) != NULL ) {
		return;
	}

	new_popup = popup_create( title, popup_id, 0 );

	POPUP_NETWORK_ASSERT( new_popup != NULL );

	popup_set_text( new_popup, text );
	popup_set_sizehint( new_popup, size_hint );

	/* Handle options. We reuse the title char array here. */

	while (size > 0) {
		FETCH_U8( option_type );
		FETCH_U8( option_group );

		switch ( option_type ) {
		case OPTION_TYPE_TEXTENTRY:
			FETCH_SIZESTRING( title );

            popup_add_option_textentry( new_popup, option_group, title );

			break;
		case OPTION_TYPE_DISPLAYTEXT:
			/* Not handled yet */
			FETCH_SIZESTRING( title );

			popup_add_option_displaytext( new_popup, option_group, title );

			break;
		case OPTION_TYPE_TEXTOPTION:
			FETCH_SIZESTRING( title );
			FETCH_U8( value_id.uint8 );

			popup_add_option( new_popup, option_group, title, value_id );

			break;
		case OPTION_TYPE_RADIOOPTION:
			FETCH_SIZESTRING( title );
			FETCH_U8( value_id.uint8 );

			popup_add_option_radio( new_popup, option_group, title, value_id );

			break;
		default:
			LOG_ERROR("CAUTION: invalid popup option type received (%d)\n", option_type );
			popup_free( new_popup );
			return;
		}
	}

	popup_set_callback( new_popup, &popup_click_callback );
	popup_finish_setup( new_popup );
}

/* EOF */
