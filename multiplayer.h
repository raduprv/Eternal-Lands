/*!
 * \file
 * \ingroup network_actors
 * \brief multiplayer related functions
 */
#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include <SDL_net.h>

#ifdef __cplusplus
extern "C" {
#endif


extern int port; /*!< the server port we use */
extern unsigned char server_address[60]; /*!< the server address we use */
extern volatile int disconnected; /*!< indicates whether we are currently connected or not */

extern TCPsocket my_socket; /*!< our TCP socket to communiate with the server */

/*! \name Version information
 * @{ */
extern int always_pathfinding; /*!< use pathfinding for walk click on far visible tiles of the 3d map */
extern int mixed_message_filter; /*!< If true, do not display console messages for mixed items when other windows are closed */
/*! @} */


extern Uint32 next_second_time; /*!< the time of the next second */
extern short real_game_minute; /*!< the real game minute */
extern short real_game_second; /*!< the real game second */

/*!
 * \brief Get the Eternal Lands version number
 *
 *	Print the Eternal Lands version in string \a buf.
 *
 * \param buf	the character buffer in which the string is placed
 * \param len	the size of the buffer
 * \callgraph
 */
void get_version_string (char *buf, size_t len);

/*!
 * \brief	check validity of game seconds
 *
 * \retval	true if we have set seconds.
*/
int is_real_game_second_valid(void);

/*!
 * \brief	Set game seconds as valid.
 *
*/
void set_real_game_second_valid(void);

/*!
 * \brief	Get the current game time.
 *
 * \retval	game time in seconds.
*/
Uint32 get_game_time_sec(void);

/*!
 * \brief	Get the time difference from current game time.
 *
 * \param	ref_time the relative time to compare
 *
 * \return	the time difference in seconds, wrapped appropriately
*/
Uint32 diff_game_time_sec(Uint32 ref_time);

/*!
 * \brief	Set the state to disconnected from the server, showing messages and recording time.
 *
 * \param	message A message string, or NULL
 *
*/
void enter_disconnected_state(const char *message);

/*!
 * \brief	Close connection and call enter_disconnected_state().
 *
 * \param	message A message string, or NULL
 *
*/
void force_server_disconnect(const char *message);

/*!
 * \brief	Called from the main thread 500 ms timer, check if testing server connection.
 *
*/
void check_if_testing_server_connection(void);

/*!
 * \brief	Initiates a test for server connection, the client will enter the disconnected state if needed.
 *
*/
void start_testing_server_connection(void);

extern time_t last_heart_beat; /*!< a timestamp that indicates when the last message was sent to the server */

extern time_t last_save_time; /*!< a timestamp inidicating the last \#save */

extern int log_conn_data; /*!< indicates whether we should log connection data or not */

extern char inventory_item_string[300]; /*!< the last inventory text string */
extern size_t inventory_item_string_id; /*!< incremented each time we get a new string so users notice */

/*!
 * \ingroup network_actors
 *
 *      Creates the mutex for the tcp output buffer.
 *
 */
void create_tcp_out_mutex();

/*!
 * \ingroup network_actors
 *
 *      Destroys the mutex for the tcp output buffer, the socket and the socket set
 *
 */
void cleanup_tcp();

	/*!
 * \ingroup network_actors
 * \brief Move the actor to a new location
 *
 *	Move the actor to a nearby location by calling the pathfinder.
 *
 * \param x the x-coordinate of the new position
 * \param y the y-coordinate of the new position
 * \param try_pathfinder if true, and other conditions permit, use the pathfinder
 *
 * \pre pathfinder will be used if try_pathfinder and always_pathfinding true and distance over the threshold
 */
void move_to (short int x, short int y, int try_pathfinder);

/*!
 * \ingroup network_actors
 * \brief   Sends the given message \a str using the socket \a my_socket to the server.
 *
 *      The message \a str will be sent to the server using the socket \a my_socket. The max. length of \a str is given in \a len.
 *
 * \param my_socket the socket used to communicate with the server
 * \param str       the message to sent
 * \param len       the length of \a str
 * \retval int      0, if the client is not connected, or if the actor has been sitting for a specific amount of time, or if the packet is already stored in the \ref tcp_cache,
 *                  else the return value of SDLNet_TCP_Send will be returned.
 * \callgraph
 *
 * \pre If the client is \ref disconnected, this function will return 0, without performing any actions.
 * \pre If the actor is already sitting this function will return 0, when the \ref SIT_DOWN command is sent.
 * \pre If the message given in \a str was already sent during a specific amount of time, meaning it is still in the \ref tcp_cache, this function will return 0.
 */
int my_tcp_send (TCPsocket my_socket, const Uint8 *str, int len);

int my_tcp_flush (TCPsocket my_socket);


/*!
 * \ingroup network_actors
 * \brief   Tries to connect to the server.
 *
 *      Connects the client to the server.
 *
 * \callgraph
 *
 * \pre If the client version is too old, this function will return without performing any action.
 */
void connect_to_server();

/*!
 * \ingroup network_actors
 * \brief   Sends the login information from the username and password input fields to the server.
 *
 *      Sends the login information from the username and password input fields together with a \ref LOG_IN command to the server.
 *
 * \sa my_tcp_send
 */
void send_login_info();

/*!
 * \ingroup network_actors
 * \brief   Sends a \ref CREATE_CHAR command to the server.
 *
 *      Sends the \ref CREATE_CHAR command to the server. The data sent, will be combined from the given parameters.
 *
 * \param user_str          the username for the char
 * \param pass_str          the password for the char
 * \param skin              the skin id used by the char
 * \param hair              the hair id used by the char
 * \param eyes              the eyes id used by the char
 * \param shirt             the shirt id used by the char
 * \param pants             the pants id used by the char
 * \param boots             the boots id used by the char
 * \param head              the head id used by the char
 * \param type              the actor type id used by the char.
 *
 * \sa my_tcp_send
 *
 * \pre If the length of \a user_str is less than 3, this function will create an error and returns.
 * \pre If the length of \a pass_str is less than 4, this function will create an error and returns.
 * \pre If the \a conf_pass_str doesn't match the \a pass_str, this function will create an error and returns.
 */
void send_new_char(char * user_str, char * pass_str, char skin, char hair, char eyes, char shirt, char pants, char boots,char head, char type);

/*!
 * \ingroup network_actors
 * \brief   Checks for new server messages.
 *
 *      Checks whether there are new messages from the server waiting and processes them where necessary.
 *
 * \pre If the client is disconnected, this function won't perform any actions.
 * \pre If the socket set is invalid, this function won't perform any actions.
 * \pre If the socket is not ready, this function won't perform any actions.
 */
int get_message_from_server(void *thread_args);

void process_message_from_server(const Uint8 *in_data, int data_length);

void send_heart_beat();

/*!
 * \brief	Store a new game date.
 *
 * 		If a callback is registered, send it the date too.
 *
 * \param	the_string	the new data string
 *
 * \retval	int	1 if the date was requested from a get_date() call, otherwise 0
*/
int set_date(const char *the_string);

/*!
 * \brief	Get the latest game date.
 *
 * 		If there is a valid date string, a pointer to it is returned and any supplied
 * callback function called.  Otherwise the server is requested to send the new date and
 * save any callback function for when we have the new date.
 *
 * \param	callback	if not NULL a function to be passed the string when we have it
 *
 * \retval	string pointer	NULL is no date ready
*/
const char *get_date(void (*callback)(const char *));

#ifdef __cplusplus
} // extern "C"
#endif

#endif
