/*!
 * \file popup.h
 * \ingroup popup_window
 * \brief Interface for a popup widget with entries
 */

#include "list.h"
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif


	/* Option types */
#define OPTION_TYPE_TEXTENTRY   0x00
#define OPTION_TYPE_DISPLAYTEXT 0x01
#define OPTION_TYPE_TEXTOPTION  0x08
#define OPTION_TYPE_RADIOOPTION 0x09

/* Typedefs for used structures */

typedef list_node_t    popup_node_t;
typedef list_node_t    popup_option_node_t;
typedef Uint8          popup_option_type_t;
typedef Uint8          popup_option_group_t;
typedef Uint16         popup_id_t;

/* Union for values to be sent back to server */

typedef union {
	Uint8 uint8;                       /* U8 for radio buttons and one-click text */
	char *str;                         /* string for text entries */
} popup_option_value_t;

/* Flowing text structure. Used to save width and number of lines */

typedef struct flowing_text_t
{
	char *str;                         /* String to be displayed */
	int str_size_allocated;            /* Size allocated for the string (needed to add newlines) */
	int lines;                         /* Number of lines after performing flow */
	int width;                         /* Max width after performing flow */
	int height;                        /* Height after performing flow */
} flowing_text_t;


/* Popup option structure. Allocated for each popup option beying displayed */

typedef struct popup_option_t {
	flowing_text_t       text;        /* Text to be displayed */
	popup_option_value_t value;       /* Value to be sent back to the server */
	popup_option_type_t  type;        /* Type of this popup option. See OPTION_ macros */
	popup_option_group_t group;       /* Group ID for this option */
	int selected:1;                   /* To be used by radio buttons and one-click. True if selected by user */
	int computed_y_pos;               /* computed Y pos relative to window */
	int widget_id;                    /* Widget id, if used (like text entries) */
} popup_option_t;

/* List node dereferencing */

#define POPUP_OPTION_NODE(x) \
    ((popup_option_t*)((x)->data))


struct popup_t;

/* Callback type for clicking on an option */

typedef void (*popup_callback_t)(struct popup_t *popup, popup_option_t *);

/* Grouped options structure */

typedef struct popup_grouped_option_t
{
	popup_option_group_t group_id;    /* Group ID for this group */
	popup_option_type_t  type;        /* Type of all options in this group */
	list_node_t          *options;    /* Linked list with the options */
} popup_grouped_option_t;

/* List node dereferencing */
#define POPUP_GROUP_NODE(x) \
	((popup_grouped_option_t*)((x)->data))

/* Popup structure allocated for each popup beying displayed */

typedef struct popup_t {
	flowing_text_t text;              /* Text of the popup */
	int win;                          /* Window ID */
	int width;                        /* Width for this popup. */
	int height;                       /* Height of this popup. Valid only after realize() */
	char *title;                      /* Title for the popup window */
	popup_id_t id;                    /* ID of this popup */
	list_node_t *grouped_options;     /* Grouped options for this popup */
	unsigned int realized:1;          /* Set to one if we're realized (have a window) */
	unsigned int is_persistent:1;     /* Set to one if we are persistent (not yet used) */
	unsigned int has_send_button:1;   /* Set to one if we have a Send button */
	popup_callback_t option_click_callback; /* Generic callback for when someone clicks on an option */
	int button_widget_id;             /* Send button widget ID (if applicable) */
} popup_t;

/* List node dereferencing */
#define POPUP_NODE(x) \
    ((popup_t*)((x)->data))

/*!
 * \ingroup popup_window
 * \brief	Initialize the popup structures
 */
void popup_init();
void popup_display();

/*!
 * \ingroup popup_window
 * \brief	Create a new popup window with a title
 * \param title The popup window title
 * \returns The newly created popup window.
 */

popup_t *popup_create( const char *title, popup_id_t popup_id, int persistent );
void popup_set_text( popup_t *this_popup, const char *text );
void popup_finish_setup( popup_t *this_popup );
void popup_set_callback( popup_t *popup, popup_callback_t callback );
void popup_create_from_network( const unsigned char *payload, size_t size );

/*
 Server to Client Protocol definition:

 Sizes are in bytes. Byteorder is preserved.

 Variable sizes are depicted by capital letters.

 Main header:
 +------+--------+------------+--------------------------------------+
 | Size | Type   | Name       | Description                          |
 +------+--------+------------+--------------------------------------+
 | 2    | Uint16 | PopupID    | Unique ID for this popup.            |
 | 1    | Uint8  | Flags      | Popup flags. Reserved for expansion. |
 | 1    | Uint8  | Title Size | Size (in chars) of popup title (*)   |
 | A    | char[] | Title      | Popup title.                         |
 | 2    | Uint16 | SizeHint   | Hint of the desired popup width      |
 | 1    | Uint8  | Text Size  | Text size in the popup (*)           |
 | B    | char[] | Text       | Text to be displayed by the popup.   |
 +------+--------+------------+--------------------------------------+

 After the main header comes the options. Any number of options
 can be present.

 Option Header

 +------+--------+------------------+------------------------------+
 | Size | Type   | Name             | Description                  |
 +------+--------+------------------+------------------------------+
 | 1    | Uint8  | Option Type (**) | The option type.             |
 | 1    | Uint8  | Group ID         | Group ID for this option     |
 | 1    | Uint8  | Text size        | Size of text (*)             |
 | C    | char[] | Text             | Option text                  |
 +------+--------+------------------+------------------------------+

 For value options, follows this after each option header:
 +------+--------+------------------+------------------------------+
 | Size | Type   | Name             | Description                  |
 +------+--------+------------------+------------------------------+
 | 1    | Uint8  | Value ID         | Value to be sent back (***)  |
 +------+--------+------------------+------------------------------+

 Text options do not include anything more than the option header.

 (*) Text do not include terminating \0.
 (**) Option types <8 are text types, all others are value types.
     Defined types:
	 TYPE_TEXTENTRY    0x00
	 TYPE_DISPLAYTEXT  0x01
	 TYPE_TEXTOPTION   0x08
	 TYPE_RADIOOPTION  0x09

 (***) Value IDs range from 1-255. Value ID 0 is reserved.


 Client to Server protocol:

 Main header:
 +------+--------+------------+--------------------------------------+
 | Size | Type   | Name       | Description                          |
 +------+--------+------------+--------------------------------------+
 | 2    | Uint16 | PopupID    | Unique ID for this popup.            |
 +------+--------+------------+--------------------------------------+

 Follows one or more entries:

 +------+--------+------------+--------------------------------------+
 | Size | Type   | Name       | Description                          |
 +------+--------+------------+--------------------------------------+
 | 1    | Uint8  | group      | Group ID                             |
 | 1    | Uint8  | Value ID   | Value ID for this option             |
 +------+--------+------------+--------------------------------------+

 If value ID == 0, then this is a textual entry option, so the text follows:

 +------+--------+------------+--------------------------------------+
 | Size | Type   | Name       | Description                          |
 +------+--------+------------+--------------------------------------+
 | 1    | Uint8  | Text size  | Size of text entered (*)             |
 | D    | char[] | Text       | Option text                          |
 +------+--------+------------+--------------------------------------+

 */
#ifdef __cplusplus
}
#endif
