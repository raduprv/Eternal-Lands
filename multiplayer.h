/*!
 * \file
 * \ingroup network_actors
 * \brief multiplayer related functions
 */
#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__


extern const char * web_update_address;

extern char our_name[20];
extern char our_password[20];

extern char create_char_error_str[520];
extern char log_in_error_str[520];

extern int port;
extern unsigned char server_address[60];

extern TCPsocket my_socket;
extern SDLNet_SocketSet set;

extern Uint8 in_data[8192];

extern int combat_mode;
extern int previously_logged_in;

extern char version_string[];
extern int client_version_major;
extern int client_version_minor;
extern int client_version_release;
extern int client_version_patch;
extern int version_first_digit;
extern int version_second_digit;
extern Uint32 last_heart_beat;

extern Uint32 cur_time, last_time;
extern int server_time_stamp;
extern int client_time_stamp;
extern int client_server_delta_time;

extern int log_conn_data;

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

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \param ip
 *
 * \sa my_tcp_send
 */
void send_version_to_server(IPaddress *ip);

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

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \param in_data
 * \param data_lenght
 *
 * \callgraph
 */
void process_message_from_server(unsigned char *in_data, int data_lenght);

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \retval int
 */
int recvpacket();

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 */
void get_message_from_server();

/*!
 * \ingroup network_actors
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void get_updates();
#endif
