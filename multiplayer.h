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

extern int version_first_digit;
extern int version_second_digit;

extern int last_heart_beat;


void send_login_info();

#endif

