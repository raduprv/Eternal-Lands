/*!
 * \file
 * \ingroup chat_window
 * \brief Declare the functions used to display the chat console.
 */
#ifndef __CHAT_H__
#define __CHAT_H__

#include "elwindows.h"
#include "queue.h"
#include "widgets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEXT_MESSAGE_LENGTH 160 /*!< The server will disconnect us when we send longer messages */
#define MAX_CHAT_TABS		12	/*!< Size of the \see channels array */
#define MAX_ACTIVE_CHANNELS	10	/*!< Maximum number of channels in use */

extern Uint32 active_channels[MAX_ACTIVE_CHANNELS];
extern Uint8 current_channel;

#define INPUT_MARGIN 4
#define INPUT_HEIGHT (DEFAULT_FONT_Y_LEN + 2*INPUT_MARGIN) /* 1 line, 2 margins at 4px*/
#define INPUT_DEFAULT_FLAGS (TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS|WIDGET_CLICK_TRANSPARENT)

extern widget_list *input_widget;

extern queue_t *chan_name_queue;

/*
 * \brief   Returns the current chat input widget
 *
 *      Returns a pointer to the current chat input widget.
 *
 * \callgraph
 *

widget_list *get_input_widget(void);*/

/*!
 * \brief   Moves the chat input widget to a different window
 *
 *	Moves the chat input widget to a different window
 *
 * \param window_id ID of the window to move the widget to.
 * \callgraph
 */
void input_widget_move_to_win(int window_id);

/*!
 * \ingroup chat_window
 * \brief   Sets the channels that the player is currently subscribed to
 *
 *      Sets the channels that the player is currently subscribed to
 *
 * \param active The index of the currently active channel
 * \param channels The channel numbers
 * \param nchan The number of channels
 *
 * \callgraph
 */
void set_active_channels (Uint8 active, const Uint32 *channels, int nchan);

/*!
 * \ingroup chat_window
 * \brief   give the channel number of channel \a idx
 *
 *	give the channel number of channel \a idx
 *
 * \param idx The channel index
 *
 * \retval Uint32 The channel number if \a idx is a valid channel index, 0 otherwise
 * \callgraph
 */
Uint32 get_active_channel (Uint8 idx);

/*! Structure to hold infos for a chat window tab  */
typedef struct
{
	int tab_id;
	int out_id;
	Uint8 chan_nr;
	int nr_lines;
	char open, newchan, highlighted;
} chat_channel;

extern chat_channel channels[MAX_CHAT_TABS]; /*!< Infos about a chat window tabs  */

extern int use_windowed_chat; /*!< flag indicating whether we use the channel selection bar, the chat window, or neither */
extern int highlight_tab_on_nick; /*!< flag indicating whether we want to highligh chat tab on nick or not  */

extern int chat_win; /*!< handler for the chat window */

extern int chat_win_text_width; /*!< width of the chat window */

extern int active_tab; /*!< active chat window tab */

extern int chat_tabcollection_id;

void clear_chat_wins (void);

/*!
 * \ingroup chat_window
 * \brief   Parse text as console input
 *
 *      A common routine to parse input.  Input can be local chat, 
 * 	#commands, %options channel or personal chat.
 *
 * \param data       the input text
 * \param len		 the length of the input text
 * \callgraph
 */
void parse_input(char *data, int len);

/*!
 * \brief   Highlights a channel in the channel selection tab
 *
 *	Highlights a channel's tab in red. Used when a message with player's name arrives.
 *
 * \param channel channel to highlight
 *
 * \retval int 1 if highlighted, 0 otherwise
 * \callgraph
 */
int highlight_tab(const Uint8 channel);

/*!
 * \ingroup chat_window
 * \brief   Initializes the structures for the chat channels
 *
 *      Initializes the structures for the chat channels.
 *
 * \callgraph
 */
void init_chat_channels (void);

/*!
 * \ingroup chat_window
 * \brief   clear_input_line
 *
 *      Detail
 *
 * \callgraph
 */
void clear_input_line (void);

/*!
 * \ingroup chat_window
 * \brief   Updates the chat window
 *
 *      Updates the chat window.
 *
 * \param msg       The new message that arrived
 * \param highlight Flag indicating whether the destination tab should be highlighted
 * \callgraph
 */
void update_chat_window (text_message * msg, char highlight);

/*!
 * \ingroup chat_window
 * \brief   Switches to the tab with id id
 *
 *      Switches to the tab with id id.
 *
 * \param id Id of the tab to switch to
 * \param click Indicates if we clicked on the tab with the mouse or not
 * \callgraph
 */
void switch_to_chat_tab(int id, char click);
/*!
 * \ingroup chat_window
 * \brief   Handle a keypress of the root window 
 *
 *      Handles a keypress in the root window as if it were pressed in the chat window input field. 
 *
 * \param key
 * \param unikey
 *
 * \retval int 1 if handled, 0 otherwise
 * \callgraph
 */
int root_key_to_input_field (Uint32 key, Uint32 unikey);

/*!
 * \ingroup chat_window
 * \brief   Paste a text into the input field
 *
 *      Pastes a text line at the current cursor position in the input field
 *
 * \param text the text to paste
 *
 * \callgraph
 */
void paste_in_input_field (const Uint8 *text);

/*!
 * \ingroup chat_window
 * \brief   Creates the chat window
 *
 *      Creates the chat window
 *
 * \callgraph
 */
void create_chat_window (void);

/*!
 * \ingroup chat_window
 * \brief   Displays the chat window
 *
 *      Displays the chat window
 *
 * \callgraph
 */
void display_chat (void);

/*!
 * \ingroup chat_window
 * \brief   Updates the chat window text zoom
 *
 *      Updates the chat window text zoom
 *
 * \callgraph
 */
void chat_win_update_zoom (void);

///////////////////////////////////////////////////////////////////////

typedef struct 
{
	Uint8 channel;
	int button;
	char highlighted;
	char * description;
} chat_tab;

#define SPEC_CHANS 12 //11 are currently in use. read channels.xml for the list

typedef struct
{
	Uint32 channel;
	char * name;
	char * description;
} chan_name;

extern chat_tab tabs[MAX_CHAT_TABS]; /*!< info about chat tabs */
extern int local_chat_separate;		/*!< if non-zero, show local chat in a separate tab */
extern int personal_chat_separate;	/*!< if non-zero, show PMs in a different tab */
extern int guild_chat_separate;		/*!< if non-zero, show GMs in a different tab */
extern int server_chat_separate;	/*!< if non-zero, show game messages in a different tab */
extern int mod_chat_separate;		/*!< for moderators and newbie helpers only: if non-zero, show mod chat in a different tab */
extern int tabs_in_use;
extern int current_tab;

extern int tab_bar_win;			 /*!< handler for the tab bar window */

/*!
 * \ingroup chat_bar
 * \brief   Show the channel selection bar
 *
 *      Show the channel selection bar
 *
 * \callgraph
 */
void display_tab_bar (void);

void switch_to_tab(int id);

void change_to_current_tab(const char *input);

void change_to_current_chat_tab(const char *input);
/*!
 * \ingroup chat_bar
 * \brief   Highlight a channel in the channel selection bar
 *
 *      Highlight a channel in the channel selection bar when a message arrives in that channel.
 *
 * \param msg the message that has arrived
 * \callgraph
 */
void update_tab_bar (text_message * msg);

/*!
 * \ingroup chat_window
 * \brief	Convert from chat window to tab bar, or vice versa
 *
 *	Convert the chat window's tabs into tab bar buttons, or vice versa
 *
 * \param new_wc	the new value of use_windowed_chat
 * \callgraph
 */
void convert_tabs (int new_wc);

/*!
 * \brief	Read channel names from file
 *
 *	Open and parse file of names and descriptions for each chat channel
 *
 * \callgraph
 */
void init_channel_names (void);

/*!
 * \ingroup chat_window
 * \brief   Put a text line in the input field
 *
 *      Removes the content of the input field and inserts a new text line
 *
 * \param text the text to insert
 *
 * \callgraph
 */
void put_string_in_input_field(const Uint8 *text);

int command_jlc(char * text, int len);

void update_chat_win_buffers(void);

void cleanup_chan_names(void);

int chat_input_key(widget_list *widget, int mx, int my, Uint32 key, Uint32 unikey);

int resize_chat_handler(window_info *win, int width, int height);

/*
 * \ingroup channel_colors
 *
 *      Channel color stuff
 *
 * \callgraph
 *
 */
#define MAX_CHANNEL_COLORS 64

typedef struct
{
	Uint32 nr;
	int color;
} channelcolor;

extern channelcolor channel_colors[MAX_CHANNEL_COLORS];

void load_channel_colors();

void save_channel_colors();

int command_channel_colors(char * text, int len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __CHAT_H__
