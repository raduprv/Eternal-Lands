/*!
 * \file
 * \ingroup network_actors
 * \brief multiplayer related functions
 */
#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__


/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in multiplayer.c, no need to declare them here.
 */
//extern const char * web_update_address; /*!< URL for the update page, currently not used */
//extern char our_name[20]; /*!< the name of our actor */
//extern char our_password[20]; /*!< the password of our actor */

extern char create_char_error_str[520]; /*!< buffer for messages that came from errors during the creation of a new character */
extern char log_in_error_str[520]; /*!< buffer for messagees that came from errors during login */

extern int port; /*!< the server port we use */
extern unsigned char server_address[60]; /*!< the server address we use */

extern TCPsocket my_socket; /*!< our TCP socket to communiate with the server */

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in multiplayer.c, no need to declare it here.
 */
//extern SDLNet_SocketSet set;
//extern Uint8 in_data[8192]; /*!< a buffer for input data from either the user or the server */

extern int combat_mode; /*!< a flag that indicates whether our actor is currently in combat mode, aka fighting or not */

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in multiplayer.c, no need to declare it here.
 */
//extern int previously_logged_in; /*!< true, if we got disconnected, but the client is still running */

/*! \name Version information 
 * @{ */
extern char version_string[]; /*!< a buffer for the complete version string */
extern int client_version_major; /*!< The clients Major version number */
extern int client_version_minor; /*!< The clients Minor version number */
extern int client_version_release; /*!< The clients Release version number */
extern int client_version_patch; /*!< The clients Patchlevel number */
extern int version_first_digit; /*!< the first digit of the version */
extern int version_second_digit; /*!< the second digit of the version */
/*! @} */

extern Uint32 last_heart_beat; /*!< a timestamp that inidicates when the last message was sent to the server */
extern Uint32 cur_time, last_time; /*!< timestamps to check whether we need to resync */

/*
 * OBSOLETE: Queued for removal from this file.
 * Used only in multiplayer.c, no need to declare them here.
 */
//extern int server_time_stamp; /*!< the current time on the server */
//extern int client_time_stamp; /*!< the current time on the client */
//extern int client_server_delta_time; /*!< the difference between the servers and the clients current time */

extern int log_conn_data; /*!< indicates whether we should log connection data or not */

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \param my_socket
 * \param str
 * \param len
 * \retval int
 */
int my_tcp_send(TCPsocket my_socket, Uint8 *str, int len);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \param ip
// *
// * \sa my_tcp_send
// */
//void send_version_to_server(IPaddress *ip);

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void connect_to_server();

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \sa my_tcp_send
 */
void send_login_info();

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \param user_str
 * \param pass_str
 * \param conf_pass_str
 * \param skin
 * \param hair
 * \param shirt
 * \param pants
 * \param boots
 * \param head
 * \param type
 *
 * \sa my_tcp_send
 */
void send_new_char(Uint8 * user_str, Uint8 * pass_str, Uint8 * conf_pass_str, char skin, 
				   char hair, char shirt, char pants, char boots,char head, char type);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \param in_data
// * \param data_lenght
// *
// * \callgraph
// */
//void process_message_from_server(unsigned char *in_data, int data_lenght);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \retval int
// */
//int recvpacket();

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 */
void get_message_from_server();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup network_actors
// * \brief
// *
// *      Detail
// *
// * \callgraph
// */
//void get_updates();
#endif
