/*!
 * \file
 * \ingroup chat_window
 * \brief Declare the functions used to display the chat console.
 */
#ifndef __CHAT_H__
#define __CHAT_H__

#include "elwindows.h"
#include "queue.h"
#include "text.h"
#include "widgets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEXT_MESSAGE_LENGTH 160 /*!< The server will disconnect us when we send longer messages */

#define INPUT_MARGIN 4
#define INPUT_DEFAULT_FLAGS (TEXT_FIELD_EDITABLE|TEXT_FIELD_NO_KEYPRESS|WIDGET_CLICK_TRANSPARENT)

#define MAX_CHANNEL_COLORS 64

typedef struct
{
	Uint32 nr;
	int color;
} channelcolor;

typedef struct
{
	int value;
	const int lower;
	const int upper;
} max_chat_lines_def;

extern max_chat_lines_def max_chat_lines;

extern widget_list *input_widget;

extern int use_windowed_chat;		/*!< flag indicating whether we use the channel selection bar, the chat window, or neither */
//extern int highlight_tab_on_nick;	/*!< flag indicating whether we want to highligh chat tab on nick or not  */
extern int chat_win;				/*!< handler for the chat window */
extern int local_chat_separate;		/*!< if non-zero, show local chat in a separate tab */
extern int personal_chat_separate;	/*!< if non-zero, show PMs in a different tab */
extern int guild_chat_separate;		/*!< if non-zero, show GMs in a different tab */
extern int server_chat_separate;	/*!< if non-zero, show game messages in a different tab */
extern int mod_chat_separate;		/*!< for moderators and newbie helpers only: if non-zero, show mod chat in a different tab */
extern int tab_bar_win;			 /*!< handler for the tab bar window */
extern int enable_chat_show_hide;	/*!< config option to enable show/hide of the chat system */

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

/*!
 * \ingroup chat_window
 * \brief   Clears the text from chat history
 *
 *      Clears the text from chat history
 *
 * \callgraph
 */
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

// /*!
// * \brief   Highlights a channel in the channel selection tab
// *
// *	Highlights a channel's tab in red. Used when a message with player's name arrives.
// *
// * \param channel channel to highlight
// *
// * \retval int 1 if highlighted, 0 otherwise
// * \callgraph
// */
//int highlight_tab(const Uint8 channel);

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
int root_key_to_input_field (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);

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
 * \brief   Displays the chat window
 *
 *      Displays the chat window
 *
 * \callgraph
 */
void display_chat (void);

/*!
 * \ingroup chat_bar
 * \brief   Show the channel selection bar
 *
 *      Show the channel selection bar
 *
 * \callgraph
 */
void display_tab_bar (void);

/*!
 * \ingroup chat
 * \brief	Switch focused channel
 *
 *	Switch focus to channel using the start of the line for selection #@/ etc.
 *
 * \param line	text line to pass
 * \callgraph
 */
void change_to_channel_tab(const char *line);

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

/*!
 * \brief checks if message fits the filter.
 *
 * \param[in] msg message to test.
 * \param[in] filter filter.
 *
 * \return 1 if message doesnt fit filter (should be skipped), 0 otherwise.
 */
int skip_message (const text_message *msg, Uint8 filter);

int command_jlc(char * text, int len);
void update_chat_win_buffers(void);
void cleanup_chan_names(void);
int chat_input_key(widget_list *widget, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod);
void load_channel_colors();
void save_channel_colors();
int command_channel_colors(char * text, int len);
void next_channel_tab(void);
void prev_channel_tab(void);
void update_text_windows (text_message * pmsg);
void recolour_messages(text_message *msgs);
void recolour_message(text_message *msg);
void reset_tab_channel_colours(void);
void set_first_tab_channel(void);
const char * get_tab_channel_name(void);
void set_next_tab_channel(void);
int get_tab_bar_x(void);
int get_tab_bar_y(void);
int get_tabbed_chat_end_x(void);

int get_input_height();

void open_chat(void);
void toggle_chat(void);
void enable_chat_shown(void);
int is_chat_shown(void);


/*!
 * \ingroup chat_window
 *
 * \brief	Get the current number of chat lines shown.
 *
 * \retval	the number of lines
 *
 * \callgraph
 */
int get_lines_to_show(void);

/*!
 * \ingroup chat_window
 *
 * \brief	Decrement the number of chat lines shown.
 *
 * \callgraph
 */
void dec_lines_to_show(void);

/*!
 * \ingroup chat_window
 *
 * \brief	Set the number of chat lines shown to zero.
 *
 * \callgraph
 */
void clear_lines_to_show(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // def __CHAT_H__
