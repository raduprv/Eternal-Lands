#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__


extern char our_name[20];
extern char our_password[20];

extern int port;
extern unsigned char server_address[60];

extern TCPsocket my_socket;
SDLNet_SocketSet set;

extern Uint8 in_data[8192];

extern int combat_mode;
extern int previously_logged_in;

extern char version_string[];
extern int version_major_digit;
extern int version_first_digit;
extern int version_second_digit;
extern int version_patch_digit;
extern int last_heart_beat;

int my_tcp_send(TCPsocket my_socket, Uint8 *str, int len);
void send_version_to_server();
void connect_to_server();
void send_login_info();
void send_new_char(Uint8 * user_str, Uint8 * pass_str, Uint8 * conf_pass_str, char skin, 
				   char hair, char shirt, char pants, char boots,char head, char type);
void process_message_from_server(unsigned char *in_data, int data_lenght);
int recvpacket();
void get_message_from_server();

#endif

