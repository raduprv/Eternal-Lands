#include <string.h>
#include <time.h>
#include "multiplayer.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "asc.h"
#include "actors.h"
#include "actor_scripts.h"
#include "achievements.h"
#include "books.h"
#include "buddy.h"
#include "buffs.h"
#include "chat.h"
#include "console.h"
#include "consolewin.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "elwindows.h"
#include "errors.h"
#include "filter.h"
#include "gamewin.h"
#include "global.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "knowledge.h"
#include "lights.h"
#include "loginwin.h"
#include "manufacture.h"
#include "map.h"
#include "new_actors.h"
#include "new_character.h"
#include "particles.h"
#include "pathfinder.h"
#include "questlog.h"
#include "queue.h"
#include "rules.h"
#include "serverpopup.h"
#include "sound.h"
#include "spells.h"
#include "storage.h"
#include "trade.h"
#include "trade_log.h"
#include "translate.h"
#include "update.h"
#include "weather.h"
#include "counters.h"
#include "special_effects.h"
#include "eye_candy_wrapper.h"
#include "mines.h"
#include "sendvideoinfo.h"
#include "servers.h"
#include "popup.h"
#include "missiles.h"
#include "threads.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void get_updates();
 */
SDL_mutex* tcp_out_data_mutex = 0;

const char * web_update_address= "http://www.eternal-lands.com/index.php?content=update";
int icon_in_spellbar= -1;
int port= 2000;
unsigned char server_address[60];
TCPsocket my_socket= 0;
SDLNet_SocketSet set= 0;
#define MAX_TCP_BUFFER  8192
Uint8 tcp_in_data[MAX_TCP_BUFFER];
Uint8 tcp_out_data[MAX_TCP_BUFFER];
int in_data_used=0;
int tcp_out_loc= 0;
int previously_logged_in= 0;
time_t last_heart_beat;
time_t last_save_time;
int always_pathfinding = 0;
char inventory_item_string[300] = {0};
size_t inventory_item_string_id = 0;

char our_name[20];
char our_password[20];
int log_conn_data= 0;

int this_version_is_invalid= 0;
int put_new_data_offset= 0;
Uint8	tcp_cache[256];
Uint32	tcp_cache_len= 0;
Uint32	tcp_cache_time= 0;

//for the client/server sync
int server_time_stamp= 0;
int client_time_stamp= 0;
int client_server_delta_time= 0;

int yourself= -1;

int last_sit= 0;
int last_turn_around = 0;

Uint32 next_second_time = 0;
short real_game_minute = 0;
short real_game_second = 0;


/*
 *	Date handling code:
 * 
 * 		Maintain a string with the current date.  This gets invalidated
 * 	at the turn of the day but not immediately refreshed.  Rather, the
 * 	refresh (asking the server) is done next time the get string is
 * 	read.  The alternative would be for all clients to request the new date
 * 	just after the day changes.  This way lessens the load on the server.
 */
static char the_date[20] = "<unset>";	/* do not read directly, use get_date() */
static int need_new_date = 0;			/* the date string is not valid - needs a refresh*/
static int requested_date = 0;			/* a new date has been requested from the server */
/* if not NULL the function to pass a new date string - one should do... */
static void (*who_to_tell_callback)(const char *) = NULL;

int set_date(const char *the_string)
{
	safe_strncpy(the_date, the_string, sizeof(the_date));

	if (who_to_tell_callback != NULL)
	{
		(*who_to_tell_callback)(the_date);
		who_to_tell_callback = NULL;
	}

	if (!need_new_date)
		return 0;

	need_new_date = requested_date = 0;
	return 1;
}

const char *get_date(void (*callback)(const char *))
{
	if (!need_new_date)
	{
		if (callback != NULL)
			(*callback)(the_date);
		return the_date;
	}

	if (callback != NULL)
		who_to_tell_callback = callback;

	if (!requested_date)
	{
		unsigned char protocol_name = GET_DATE;
		my_tcp_send(my_socket, &protocol_name, 1);
		requested_date = 1;
	}

	return NULL;
}

static void invalidate_date(void)
{
	need_new_date = 1;
}

/*	End date handling code */


void create_tcp_out_mutex()
{
	tcp_out_data_mutex = SDL_CreateMutex();
}

void destroy_tcp_out_mutex()
{
	SDL_DestroyMutex(tcp_out_data_mutex);

	tcp_out_data_mutex = 0;
}

#ifdef DEBUG
void print_packet(const char *in_data, int len){
	unsigned char buf[200];
	int i;

	printf("PACKET (%i)\n",len);
	//make it printable
	for(i=0;i<len;i++)
		if(in_data[i]>=' '&&in_data[i]<='~') buf[i]=in_data[i];
		else buf[i]='.';

	for(i=0;i<len;i++) {
		printf("%3i) %c %2x %3i\n",i,buf[i],(unsigned int)in_data[i],(int)in_data[i]);

	}
	printf("\n\n");
}
#endif


void move_to (short int x, short int y, int try_pathfinder)
{
	Uint8 str[5];

	if (try_pathfinder && always_pathfinding)
	{
		actor *me = get_our_actor();
		/* check distance */
		if (me && (abs(me->x_tile_pos-x)+abs(me->y_tile_pos-y)) > 2)
			/* if path finder fails, try standard move */
			if (pf_find_path(x,y))
				return;
	}

	str[0]= MOVE_TO;
	*((short *)(str+1))= SDL_SwapLE16 (x);
	*((short *)(str+3))= SDL_SwapLE16 (y);
	my_tcp_send(my_socket, str, 5);
}

void send_heart_beat()
{
	Uint8 command[64];
	int	len;

	last_heart_beat= time(NULL);
	command[0]= HEART_BEAT;
	len= 1;
#ifdef	OLC
	len+= olc_heartbeat(command+len);
#endif	//OLC
	my_tcp_send(my_socket, command, len);
}

/*!
 * This function sends the tcp output buffer, but without locking
 */
static int my_locked_tcp_flush(TCPsocket my_socket)
{
	int ret;

	if (disconnected || tcp_out_loc == 0)
	{
		return 0;
	}

	// if we are already sending data, lets see about sending a heartbeat a little bit early
	if (last_heart_beat+20 <= time(NULL))
	{
		send_heart_beat();
	}

	// send all the data in the buffer
#ifdef	OLC
	ret= olc_tcp_send(my_socket, tcp_out_data, tcp_out_loc);
	if (ret > 0)
	{
		ret= olc_tcp_flush();
	}
#else	//OLC
	ret= SDLNet_TCP_Send(my_socket, tcp_out_data, tcp_out_loc);
#endif	//OLC

	// empty the buffer
	tcp_out_loc= 0;

	tcp_cache[0]=0;

	return ret;
}

int my_tcp_send (TCPsocket my_socket, const Uint8 *str, int len)
{
	Uint8 new_str[1024];	//should be enough

	CHECK_AND_LOCK_MUTEX(tcp_out_data_mutex);

	if (disconnected)
	{
		CHECK_AND_UNLOCK_MUTEX(tcp_out_data_mutex);

		return 0;
	}

	if (tcp_out_loc > 0 && tcp_out_loc + len + 2 >= MAX_TCP_BUFFER)
	{
		// wouldn't fit, send what we have
		my_locked_tcp_flush(my_socket);
	}

	CHECK_AND_UNLOCK_MUTEX(tcp_out_data_mutex);

	// LabRat's anti-bagspam code
	// Grum: Adapted. Converting every movement to a path caused too much
	// trouble. Instead we now check the current actor animation for
	// movement.
	if ((str[0] == TURN_LEFT || str[0] == TURN_RIGHT) && on_the_move (get_our_actor ()))
		return 0;
	if (str[0] == DROP_ITEM  && on_the_move (get_our_actor ()))
	{
		// I thought about having a bit of code here that counts attempts, and after say 5,
		// announces on #abuse something like "#abuse I attempted to bagspam, but was thwarted,
		// please teach me the error of my ways so I refrain from doing so in the future"
		/*if((spamcount++)>4)
		{
			Uint8 badstr[256];
			safe_snprintf (badstr, sizeof(badstr), "#abuse I attempted to bagspam %i bags, but was thwarted, please correct me", spamcount);
			send_input_text_line (badstr, strlen(badstr));
			spamcount = 0;  /reset spam count so the #abuse staff don't get swamped..
		}*/

		// The anti bagspam code in all its glory - don't allow us to drop a bag if following
		// a path - I tried coding every DROP_ALL part of the code, but it was longwinded and
		// this way, after a couple of hours break, seemed the more logical and straightforward
		// solution.
		// 1% of the produce from manufacturers may be donated to Labrat for this patch,
		// or for the bagspammers, sell the items you were going to spam and give the proceeds
		// to a noob on IP :)
		//
		return 1;
	}

	//check to see if we have too many packets being sent of the same to reduce server flood
	if(len < sizeof (tcp_cache))	// only if it fits
	{
		if(str[0]==MOVE_TO || str[0]==RUN_TO || str[0]==SIT_DOWN || str[0]==HARVEST || str[0]==MANUFACTURE_THIS || str[0]==CAST_SPELL || str[0]==RESPOND_TO_NPC || str[0]==ATTACK_SOMEONE || str[0]==SEND_PM || str[0]==RAW_TEXT || str[0]==TURN_LEFT || str[0]==TURN_RIGHT)
		{
			Uint32	time_limit= 600;

			if( str[0]==SEND_PM || str[0]==RAW_TEXT)time_limit=1500;
			if( str[0]==SIT_DOWN){
				if(last_sit+1500>cur_time) return 0;
				last_sit= cur_time;
			}
			if( str[0]==TURN_RIGHT || str[0]==TURN_LEFT )
			{
				if(last_turn_around + 600 > cur_time) return 0;
				last_turn_around = cur_time;
			}

			//if too close together
			if(len == (int)tcp_cache_len && *str == *tcp_cache && cur_time < tcp_cache_time+time_limit){
				//and the same packet
				if(!memcmp(str, tcp_cache, len)){
					//ignore this packet
					return 0;
				}
			}

			//turns do not interrupt queued moves
			if(tcp_cache[0] == MOVE_TO && (str[0]==TURN_LEFT || str[0]==TURN_RIGHT)) return 0;

			//memorize the data we are sending for next time
			memcpy(tcp_cache, str, len);
			tcp_cache_len= len;
			tcp_cache_time= cur_time;
		}
	}

	CHECK_AND_LOCK_MUTEX(tcp_out_data_mutex);

	//update the heartbeat timer
	last_heart_beat= time(NULL);

	// check to see if the data would fit in the buffer
	if (len + 2 < MAX_TCP_BUFFER)
	{
		// yes, buffer it for later processing
		tcp_out_data[tcp_out_loc] = str[0];	//copy the protocol byte
		*((short *)(tcp_out_data+tcp_out_loc+1)) = SDL_SwapLE16((Uint16)len);//the data length
		// copy the rest of the data
		memcpy(&tcp_out_data[tcp_out_loc+3], &str[1], len-1);
		// adjust then buffer offset
		tcp_out_loc += len + 2;

		CHECK_AND_UNLOCK_MUTEX(tcp_out_data_mutex);

		return len + 2;
	}

	// no, send it as is now

	CHECK_AND_UNLOCK_MUTEX(tcp_out_data_mutex);

	new_str[0] = str[0];	//copy the protocol byte
	*((short *)(new_str+1)) = SDL_SwapLE16((Uint16)len);//the data length
	// copy the rest of the data
	memcpy(&new_str[3], &str[1], len-1);
#ifdef	OLC
	return olc_tcp_send(my_socket, new_str, len+2);
#else	//OLC
	return SDLNet_TCP_Send(my_socket, new_str, len+2);
#endif	//OLC
}

int my_tcp_flush(TCPsocket my_socket)
{
	int result;

	CHECK_AND_LOCK_MUTEX(tcp_out_data_mutex);

	result = my_locked_tcp_flush(my_socket);

	CHECK_AND_UNLOCK_MUTEX(tcp_out_data_mutex);

	return result;
}


void send_version_to_server(IPaddress *ip)
{
	Uint8 str[64];
	int	len;

	str[0]= SEND_VERSION;
	*((short *)(str+1))= SDL_SwapLE16((short)version_first_digit);
	*((short *)(str+3))= SDL_SwapLE16((short)version_second_digit);
	str[5]= client_version_major;
	str[6]= client_version_minor;
	str[7]= client_version_release;
	str[8]= client_version_patch;

#ifdef  EL_BIG_ENDIAN
	// byte swapping needed for Macs despite what the docs say
	str[9]= (ip->host >> 24)&0xFF;
	str[10]= (ip->host >> 16)&0xFF;
	str[11]= (ip->host >> 8)&0xFF;
	str[12]= ip->host&0xFF;
	str[13]= (ip->port >> 8)&0xFF;
	str[14]= ip->port&0xFF;
#else   //EL_BIG_ENDIAN
	str[9]= ip->host&0xFF;
	str[10]= (ip->host >> 8)&0xFF;
	str[11]= (ip->host >> 16)&0xFF;
	str[12]= (ip->host >> 24)&0xFF;
	str[13]= ip->port&0xFF;
	str[14]= (ip->port >> 8)&0xFF;
#endif	//EL_BIG_ENDIAN
	len= 15;

#ifdef	OLC
	len+= olc_version(str+len);
#endif	//OLC
	my_tcp_send(my_socket, str, len);
}

void connect_to_server()
{
	IPaddress	ip;

	tcp_out_loc= 0; // clear the tcp output buffer
	if(this_version_is_invalid) return;
	if(set)
		{
			SDLNet_FreeSocketSet(set);
			set= 0;
		}
	if(my_socket)
		{
			SDLNet_TCP_Close(my_socket);
			my_socket= 0;
		}
	//clear the buddy list so we don't get multiple entries
	clear_buddy();

	LOG_TO_CONSOLE(c_red1,connect_to_server_str);
	draw_scene();	// update the screen
	set= SDLNet_AllocSocketSet(1);
	if(!set)
        {
            LOG_ERROR("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
            do_error_sound();
			SDLNet_Quit();
			SDL_Quit();
			exit(4); //most of the time this is a major error, but do what you want.
        }

	if(SDLNet_ResolveHost(&ip,(char*)server_address,port)==-1)
		{
			LOG_TO_CONSOLE(c_red2,failed_resolve);
			do_disconnect_sound();
			return;
		}

	my_socket= SDLNet_TCP_Open(&ip);
	if(!my_socket)
		{
			LOG_TO_CONSOLE(c_red1,failed_connect);
			LOG_TO_CONSOLE(c_red1,reconnect_str);
			LOG_TO_CONSOLE(c_red1,alt_x_quit);
			do_disconnect_sound();
			return;
		}

	if(SDLNet_TCP_AddSocket(set,my_socket)==-1)
		{
			LOG_ERROR("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
			SDLNet_Quit();
			SDL_Quit();
			exit(2);
		}
	disconnected= 0;
	have_storage_list = 0;  //With a reconnect, our cached copy of what's in storage may no longer be accurate

	//send the current version to the server
	send_version_to_server(&ip);

	//ask for the opening screen
	if(!previously_logged_in)
		{
			Uint8 str[1];

			str[0]= SEND_OPENING_SCREEN;
			my_tcp_send(my_socket, str, 1);
		}
	else
		{
			yourself= -1;
			you_sit= 0;
			destroy_all_actors();
			send_login_info();
		}

	//clear out info
	clear_waiting_for_questlog_entry();
	harvesting = 0;
	last_heart_beat= time(NULL);
	send_heart_beat();	// prime the hearbeat to prevent some stray issues when there is lots of lag
	hide_window(trade_win);
	do_connect_sound();

	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}

void send_login_info()
{
	int i,j,len;
	unsigned char str[40];

	len= strlen(username_str);
	//check for the username length
	if (len < 3)
	{
		set_login_error (error_username_length, strlen (error_username_length), 1);
		return;
	}

	//join the username and password, and send them to the server
	str[0]= LOG_IN;

	if(caps_filter && my_isupper(username_str, len)) my_tolower(username_str);
	for(i=0; i<len; i++) str[i+1]= username_str[i];
	str[i+1]= ' ';
	i++;
	len= strlen(password_str);
	for(j=0; j<len; j++) str[i+j+1]= password_str[j];
	str[i+j+1]= 0;

	len = strlen((char*)str);
	len++;//send the last 0 too
	if(my_tcp_send(my_socket, str, len)<len)
		{
			//we got a nasty error, log it
		}

	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}


void send_new_char(char * user_str, char * pass_str, char skin, char hair, char shirt, char pants, char boots,char head, char type)
{
	int i,j,len;
	unsigned char str[120];

	len= strlen(user_str);
	str[0]= CREATE_CHAR;
	for(i=0; i<len; i++) str[i+1]= user_str[i];
	str[i+1]= ' ';
	i++;
	len= strlen(pass_str);
	for(j=0; j<len; j++) str[i+j+1]= pass_str[j];
	str[i+j+1]= 0;
	//put the colors and gender
	str[i+j+2]= skin;
	str[i+j+3]= hair;
	str[i+j+4]= shirt;
	str[i+j+5]= pants;
	str[i+j+6]= boots;
	str[i+j+7]= type;
	str[i+j+8]= head;

	len= i+j+9;
	if(my_tcp_send(my_socket,str,len)<len) {
		//we got a nasty error, log it
	}

	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}

// TEMP LOGAND [5/25/2004]
#ifndef NPC_SAY_OVERTEXT
#define NPC_SAY_OVERTEXT 58
#endif
//---

void process_message_from_server (const Uint8 *in_data, int data_length)
{
	Uint8 text_buf[MAX_TCP_BUFFER];

	if (data_length <= 2)
	{
		LOG_WARNING("CAUTION: Possibly forged packet received.\n");
		return;
	}

	//see what kind of data we got
	switch (in_data[PROTOCOL])
		{
		case RAW_TEXT:
			{
				int len;
				
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged RAW_TEXT packet received.\n");
				  break;
				}

				safe_strncpy2((char*)text_buf, (char*)&in_data[4], sizeof(text_buf), data_length - 4);
				len = strlen((char*)text_buf);

				// if from the server popup channel
				if (in_data[3] == server_pop_chan)
				{
					if (use_server_pop_win)
						display_server_popup_win((char*)text_buf);
					else
						put_text_in_buffer (in_data[3], text_buf, len);
					// if we're expecting a quest entry, this will be it
					if (waiting_for_questlog_entry())
					{
						char *cur_npc_name = (char *)malloc(sizeof(npc_name));
						safe_strncpy2(cur_npc_name, (char *)npc_name, sizeof(npc_name), sizeof(npc_name));
						safe_strncpy((char *)npc_name, "<None>", sizeof(npc_name));
						add_questlog((char*)text_buf, len);
						safe_strncpy2((char *)npc_name, cur_npc_name, sizeof(npc_name), sizeof(npc_name));
						free(cur_npc_name);
					}
				}

				// for all other messages do filtering, ignoring and counters checking etc
				else
				{
					len= filter_or_ignore_text((char*)text_buf, len, sizeof (text_buf), in_data[3]);
					if (len > 0)
						put_text_in_buffer (in_data[3], text_buf, len);
				}
			}
			break;

		case ADD_NEW_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 17)
				{
				  LOG_WARNING("CAUTION: Possibly forged ADD_NEW_ACTOR packet received.\n");
				  break;
				}
				add_actor_from_server((char*)&in_data[3], data_length-3);
			}
			break;

		case ADD_NEW_ENHANCED_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				//print_packet(in_data,data_length);
				if (data_length <= 32)
				{
				  LOG_WARNING("CAUTION: Possibly forged ADD_ENHANCED_ACTOR packet received.\n");
				  break;
				}
				add_enhanced_actor_from_server((char*)&in_data[3], data_length-3);
			}
			break;

		case ADD_ACTOR_COMMAND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				// allow for multiple packets in a row
				while(data_length >= 6){
					add_command_to_actor(SDL_SwapLE16(*((short *)(in_data+3))), in_data[5]);
					in_data+= 3;
					data_length-= 3;
				}
			}
			break;
		case ADD_ACTOR_ANIMATION:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				// allow for multiple packets in a row
				while(data_length >= 7){
					add_emote_to_actor(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))));
					in_data+= 4;
					data_length-= 4;
				}
			}
			break;
		case REMOVE_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				// allow for multiple packets in a row
				while(data_length >= 5){
					destroy_actor(SDL_SwapLE16(*((short *)(in_data+3))));
					in_data+= 2;
					data_length-= 2;
				}
			}
			break;

		case KILL_ALL_ACTORS:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				destroy_all_actors();
			}
			break;

		case NEW_MINUTE:
			{
				static short last_real_game_minute = -1;
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef DEBUG_TIME
				return;
#endif
				if (data_length <= 4)
				{
					LOG_WARNING("CAUTION: Possibly forged NEW_MINUTE packet received.\n");
					break;
				}
				real_game_minute= SDL_SwapLE16(*((short *)(in_data+3)));
				real_game_minute %= 360;
				real_game_second = 0;
				next_second_time = cur_time+1000;
				if (real_game_minute < last_real_game_minute)
					invalidate_date();
				last_real_game_minute = real_game_minute;
				new_minute();
				new_minute_console();
			}
			break;

		case LOG_IN_OK:
			{
				char str[256];
				// login and/or new character windows are no longer needed
				if (login_root_win >= 0) destroy_window (login_root_win);
				login_root_win = -1;
				if (newchar_root_win >= 0) {
					destroy_window (newchar_root_win);
					hide_window( namepass_win );
					hide_window( color_race_win );
					hide_window( newchar_advice_win );
				}
				newchar_root_win = -1;
				if (!get_show_window(console_root_win))
					show_window (game_root_win);

				safe_snprintf(str,sizeof(str),"(%s on %s) %s",username_str,get_server_name(),win_principal);
				SDL_WM_SetCaption(str, "eternallands" );

#if defined NEW_SOUND
				// Try to turn on the music as it isn't needed up until now
				if (music_on)
					turn_music_on();
#endif // NEW_SOUND

				load_quickspells();
				load_recipes();
				load_server_markings();
				load_questlog();
				load_counters();
				load_channel_colors();
				send_video_info();
				previously_logged_in=1;
				last_save_time= time(NULL);

				// Print the game date cos its pretty (its also needed for SKY_FPV to set moons for signs, wonders, times and seasons)
				command_date("", 0);
				// print the game time in order to get the seconds for the SKY_FPV feature
				command_time("", 0);
				// print the invading monster count
				safe_snprintf(str, sizeof(str), "%c#il", RAW_TEXT);
				my_tcp_send(my_socket, (Uint8*)str, strlen(str+1)+1);
				break;
			}

		case HERE_YOUR_STATS:
			{
				if (data_length <= 167)
				{
				  LOG_WARNING("CAUTION: Possibly forged HERE_YOUR_STATS packet received.\n");
				  break;
				}
				get_the_stats((Sint16 *)(in_data+3), data_length-3);
				update_research_rate();
			}
			break;

		case SEND_PARTIAL_STAT:
			{
				// allow for multiple stats in a row
				while (data_length >= 8)
				{
					get_partial_stat (in_data[3], SDL_SwapLE32(*((Sint32 *)(in_data+4))));
					in_data+= 5;
					data_length-= 5;
				}
				update_research_rate();
			}
			break;

		case GET_KNOWLEDGE_LIST:
			{
				Uint16 size;
				if (data_length <= 1)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_KNOWLEDGE_LIST packet received.\n");
				  break;
				}
				size = SDL_SwapLE16(*(Uint16 *)(in_data+1))-1;
				if (data_length <= size + 2)
				{
				  LOG_WARNING("CAUTION(2): Possibly forged GET_KNOWLEDGE_LIST packet received.\n");
				  break;
				}
				get_knowledge_list(size, (char*)in_data+3);
			}
			break;

		case GET_NEW_KNOWLEDGE:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_NEW_KNOWLEDGE packet received.\n");
				  break;
				}
				get_new_knowledge(SDL_SwapLE16(*(Uint16 *)(in_data+3)));
			}
			break;

		case HERE_YOUR_INVENTORY:
			{
				int items;
				int plen;
		
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged HERE_YOUR_INVENTORY packet received.\n");
				  break;
				}
				items = in_data[3];
				if (data_length - 4  == items * 8 )
				{
					item_uid_enabled = 0;
					plen = 8;
				}
				else if (data_length - 4  == items * 10 )
				{
					item_uid_enabled = 1;
					plen = 10;
				}
				else
				plen = 8;

				if (data_length - 4 != items * plen)
				{
				  LOG_WARNING("CAUTION(2): Possibly forged HERE_YOUR_INVENTORY packet received.\n");
				  break;
				}
				inventory_item_string[0]=0;
				inventory_item_string_id=0;
				get_your_items(in_data+3);
				trade_post_inventory();
			}
			break;

		case GET_NEW_INVENTORY_ITEM:
			{
				int plen;
				if (item_uid_enabled)
					plen=10;
				else
					plen=8;
				
				// allow for multiple packets in a row
				while(data_length >= 3+plen){
					get_new_inventory_item(in_data+3);
					in_data+= plen;
					data_length-= plen;
				}
			}
			break;

		case REMOVE_ITEM_FROM_INVENTORY:
			{
				// allow for multiple packets in a row
				while(data_length >= 4)
				{
					remove_item_from_inventory (in_data[3]);
					in_data+= 1;
					data_length-= 1;
				}
			}
			break;

		case INVENTORY_ITEM_TEXT:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged INVENTORY_ITEM_TEXT packet received.\n");
				  break;
				}
				safe_strncpy2(inventory_item_string, (const char *)&in_data[3], sizeof(inventory_item_string)-1, data_length - 3);
				inventory_item_string[sizeof(inventory_item_string)-1] = 0;
				inventory_item_string_id++;
				if(!(get_show_window(items_win)||get_show_window(manufacture_win)||get_show_window(trade_win)))
					{
						put_text_in_buffer(CHAT_SERVER, &in_data[3], data_length-3);
					}
				// Start a new block, since C doesn't like variables declared in the middle of a block.
				{
					char *teststring = "You successfully created ";
					int testlen = strlen(teststring);
					if ( (data_length > testlen+4) && (!strncmp((char*)in_data+4, teststring, testlen)) )
					{
						char *restofstring = malloc(data_length - 4 - testlen + 1);
						safe_strncpy(restofstring, (char*)in_data + 4 + testlen, data_length - 4 - testlen + 1);
						if (strlen(restofstring) > 0)
						{
							int product_count = atoi(restofstring);
							char *product = restofstring;
							while (*product!='\0' && *product!= ' ')
								product++;
							if (strlen(product)>1)
							{
								counters_set_product_info(product+1, product_count);
								check_for_recipe_name(product+1);
							}
						}
						free(restofstring);
					}
					// if we don't get the product name, make sure we don't just count it as the last item.
					else
						counters_set_product_info("",0);
				}  // End successs counters block
				/* You failed to create a[n] ..., and lost the ingredients */
				if (my_strncompare(inventory_item_string+1, "You failed to create a[n] ", 26))
				{
					size_t item_name_len = 0;
					char item_name[128];
					const char *item_string = &inventory_item_string[27];

					/* look for the ending, if found use it to locate the item name */
					char *located = strstr(item_string, ", and lost the ingredients");
					if (located) item_name_len = (size_t)((located - item_string)/sizeof(char));

					/* if there was no match then its not a crit fail string */
					if (item_name_len)
					{
						safe_strncpy2(item_name, item_string, sizeof(item_name), item_name_len);
						increment_critfail_counter(item_name);
					}
				}  // End critfail counters block
			}
			break;
		case SPELL_ITEM_TEXT:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged SPELL_ITEM_TEXT packet received.\n");
				  break;
				}
				put_small_text_in_box(in_data+3, data_length-3, 6*51+100, (char*)spell_text);
				if(sigil_win==-1||!windows_list.window[sigil_win].displayed)
					put_text_in_buffer (CHAT_SERVER, in_data+3, data_length-3);
				have_error_message=1;
			}
			break;

		case GET_KNOWLEDGE_TEXT:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_KNOWLEDGE_TEXT packet received.\n");
				  break;
				}
				put_small_text_in_box(&in_data[3],data_length-3,6*51+150,knowledge_string);
			}
			break;

		case CHANGE_MAP:
			{
			        char mapname[1024];
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged CHANGE_MAP packet received.\n");
				  break;
				}
				if(in_data[3] == '.' && in_data[4] == '/')
				{
					safe_strncpy2(mapname, (char*)in_data + 3, sizeof(mapname), data_length - 3);
				} else 
				{
					safe_snprintf(mapname, sizeof(mapname), "./%s", (char*)in_data + 3);
				}
				change_map(mapname);
			}
			break;

		case GET_TELEPORTERS_LIST:
			{
				Uint16 teleporters_no;
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 4)
				{
					LOG_WARNING("CAUTION: Possibly forged GET_TELEPORTERS_LIST packet received.\n");
					break;
				}
				teleporters_no = SDL_SwapLE16 (*((Uint16 *)(in_data + 3)));
				if (data_length <= teleporters_no * 5 + 4)
				{
					LOG_WARNING("CAUTION(2): Possibly forged GET_TELEPORTERS_LIST packet received.\n");
					break;
				}
				add_teleporters_from_list (in_data+3);
			}
			break;

		case PLAY_MUSIC:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef NEW_SOUND
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged PLAY_MUSIC packet received.\n");
				  break;
				}
				if(music_on)play_music(SDL_SwapLE16(*((short *)(in_data+3))));
#endif // NEW_SOUND
			}
			break;

		case PLAY_SOUND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef NEW_SOUND
				if (data_length <= 8)
				{
				  LOG_WARNING("CAUTION: Possibly forged PLAY_SOUND packet received.\n");
				  break;
				}
				if (sound_on) add_server_sound(SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE16(*((short *)(in_data+5))), SDL_SwapLE16(*((short *)(in_data+7))), 1.0f);
#endif // NEW_SOUND
			}
			break;

		case TELEPORT_OUT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif

				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged TELEPORT_OUT packet received.\n");
				  break;
				}
				add_particle_sys_at_tile("./particles/teleport_out.part", SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE16 (*((short *)(in_data+5))), 1);
			}
			break;

		case TELEPORT_IN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif

				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged TELEPORT_IN packet received.\n");
				  break;
				}
				add_particle_sys_at_tile("./particles/teleport_in.part", SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE16(*((short *)(in_data+5))), 1);
			}
			break;
		case LOG_IN_NOT_OK:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged LOG_IN_NOT_OK packet received.\n");
				  break;
				}
				set_login_error ((char*)&in_data[3], data_length - 3, 1);
			}
			break;

		case REDEFINE_YOUR_COLORS:
			{
				set_login_error (redefine_your_colours, strlen (redefine_your_colours), 0);
			}
			break;

		case YOU_DONT_EXIST:
			{
				set_login_error (char_dont_exist, strlen (char_dont_exist), 1);
			}
			break;


		case CREATE_CHAR_NOT_OK:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged CREATE_CHAR_NOT_OKAY packet received.\n");
				  break;
				}
				set_create_char_error ((char*)&in_data[3], data_length - 3);
				return;
			}
			break;


		case CREATE_CHAR_OK:
			{
				login_from_new_char();
			}
			break;

		case YOU_ARE:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged YOU_ARE packet received.\n");
				  break;
				}
				LOCK_ACTORS_LISTS();
				yourself= SDL_SwapLE16(*((short *)(in_data+3)));
				set_our_actor (get_actor_ptr_from_id (yourself));
				UNLOCK_ACTORS_LISTS();
			}
			break;

		case START_RAIN:
			{
				float severity;
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged START_RAIN packet received.\n");
					break;
				}

				if (data_length > 4) {
					severity= 0.1f + 0.9f * (in_data[4] / 255.0f);
				} else {
					severity= 1.0f;
				}
				if (show_weather)
				{
					weather_set_area(0, tile_map_size_x*1.5, tile_map_size_y*1.5, 100000.0, 1, severity, in_data[3]);
				}
			}
			break;

		case STOP_RAIN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged STOP_RAIN packet received.\n");
					break;
				}

				weather_set_area(0, tile_map_size_x*1.5, tile_map_size_y*1.5, 100000.0, 1, 0.0, in_data[3]);
			}
			break;

		case THUNDER:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged THUNDER packet received.\n");
					break;
				}
				if (show_weather)
				{
					weather_add_lightning(rand()%5,
                                      		-camera_x + (50.0 + rand()%101)*(rand()%2 ? 1.0 : -1.0),
                                      		-camera_y + (50.0 + rand()%101)*(rand()%2 ? 1.0 : -1.0));
				}
			}
			break;


		case SEND_WEATHER:
			{
				// nothing for the moment
			}
			break;


		case SYNC_CLOCK:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged SYNC_CLOCK packet received.\n");
				  break;
				}
				server_time_stamp= SDL_SwapLE32(*((int *)(in_data+3)));
				client_time_stamp= SDL_GetTicks();
				client_server_delta_time= server_time_stamp-client_time_stamp;
			}
			break;

		case PONG:
			{
				char str[160];
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged SYNC_CLOCK packet received.\n");
				  break;
				}
				safe_snprintf(str, sizeof(str), "%s: %i ms",server_latency, SDL_GetTicks()-SDL_SwapLE32(*((Uint32 *)(in_data+3))));
				LOG_TO_CONSOLE(c_green1,str);
			}
			break;

		case UPGRADE_NEW_VERSION:
			{
				LOG_TO_CONSOLE(c_red1,update_your_client);
				LOG_TO_CONSOLE(c_red1,(char*)web_update_address);
			}
			break;

		case UPGRADE_TOO_OLD:
			{
				LOG_TO_CONSOLE(c_red1,client_ver_not_supported);
				LOG_TO_CONSOLE(c_red1,(char*)web_update_address);
				this_version_is_invalid=1;
			}
			break;

		case GET_NEW_BAG:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 7)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_NEW_BAG packet received.\n");
				  break;
				}
				put_bag_on_ground(SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))), in_data[7]);
			}
			break;

		case GET_BAGS_LIST:
			{
				Uint16 bags_no;
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged GET_BAGS_LIST packet received.\n");
					break;
				}
				bags_no = in_data[3];
				if (data_length <= bags_no * 5 + 3)
				{
					LOG_WARNING("CAUTION(2): Possibly forged GET_BAGS_LIST packet received.\n");
					break;
				}
				add_bags_from_list(&in_data[3]);
			}
			break;

		case SPAWN_BAG_PARTICLES:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged SPAWN_BAG_PARTICLES packet received.\n");
				  break;
				}

			}
			break;

		case FIRE_PARTICLES:
			{
#ifdef EXTRA_DEBUG
				ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged FIRE_PARTICLES packet received.\n");
				  break;
				}
				add_fire_at_tile (SDL_SwapLE16(*(Uint16 *)(in_data+7)), SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))));
			}
			break;

		case REMOVE_FIRE_AT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged REMOVE_FIRE_AT packet received.\n");
				  break;
				}
				remove_fire_at_tile (SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16 *)(in_data+5))));
			}
			break;

		case GET_NEW_GROUND_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_NEW_GROUND_ITEM packet received.\n");
				  break;
				}
				get_bag_item(in_data+3);
			}
			break;

		case HERE_YOUR_GROUND_ITEMS:
			{
				int bags_no;
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged HERE_YOUR_GROUND_ITEMS packet received.\n");
					break;
				}
				bags_no = in_data[3];
				if (data_length <= bags_no * 7 + 3)
				{
					LOG_WARNING("CAUTION(2): Possibly forged HERE_YOUR_GROUND_ITEMS packet received.\n");
					break;
				}
				get_bags_items_list(&in_data[3]);
			}
			break;

		case CLOSE_BAG:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				hide_window(ground_items_win);
			}
			break;

		case REMOVE_ITEM_FROM_GROUND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged REMOVE_ITEM_FROM_GROUND packet received.\n");
				  break;
				}
				remove_item_from_ground(in_data[3]);
			}
			break;

		case DESTROY_BAG:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				remove_bag(in_data[3]);
			}
			break;

		case NPC_TEXT:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged NPC_TEXT packet received.\n");
				  break;
				}
				put_small_text_in_box(&in_data[3], data_length-3, dialogue_menu_x_len-70, (char*)dialogue_string);
				display_dialogue();
				if (is_color (in_data[3]) && is_color (in_data[4]))
				{
					// double color code, this text
					// should be added to the quest log
					safe_strncpy2((char*)text_buf, (char*)&in_data[4], sizeof(text_buf), data_length - 4);
					add_questlog ((char*)text_buf, strlen((char*)text_buf));
				}
				// if we're expecting a quest entry, this will be it
				else if (waiting_for_questlog_entry())
				{
					safe_strncpy2((char*)text_buf, (char*)&in_data[3], sizeof(text_buf), data_length - 3);
					add_questlog ((char*)text_buf, strlen((char*)text_buf));
				}
			}
			break;

		case SEND_NPC_INFO:
			{
				if (data_length <= 23)
				{
				  LOG_WARNING("CAUTION: Possibly forged NPC_INFO packet received.\n");
				  break;
				}
				safe_strncpy2((char*)npc_name, (char*)&in_data[3], sizeof(npc_name), 20);
				cur_portrait=in_data[23];
			}
			break;

		case NPC_OPTIONS_LIST:
			{
				// NOTE: an empty response list (data_length == 3) is valid,
				// and simply means that the response list should be cleared.
				// Just take care not to try to use the data argument in
				// build_response_entries ().
				build_response_entries (in_data+3, SDL_SwapLE16 (*((Uint16 *)(in_data+1))));
			}
			break;

		case GET_TRADE_ACCEPT:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_TRADE_ACCEPT packet received.\n");
				  break;
				}
				if(!in_data[3])
					trade_you_accepted++;
				else
					trade_other_accepted++;
			}
			break;

		case GET_TRADE_REJECT:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_TRADE_REJECT packet received.\n");
				  break;
				}
				if(!in_data[3])trade_you_accepted=0;
				else
					trade_other_accepted=0;
			}
			break;

		case GET_TRADE_EXIT:
			{
				hide_window(trade_win);
				trade_exit();
			}
			break;

		case GET_YOUR_TRADEOBJECTS:
			{
				int items;
				int plen;
				if (item_uid_enabled)
					plen=10;
				else
					plen=8;

				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_YOUR_TRADEOBJECTS packet received.\n");
				  break;
				}
				items = in_data[3];
				if (data_length <= items * plen + 3)
				{
				  LOG_WARNING("CAUTION(2): Possibly forged GET_YOUR_TRADEOBJECTS packet received.\n");
				  break;
				}
				get_your_trade_objects(in_data+3);
			}
			break;

		case GET_TRADE_OBJECT:
			{
				int plen;
				if (item_uid_enabled)
					plen=10;
				else
					plen=8;		
			
				if (data_length <= 3+plen)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_TRADE_OBJECT packet received.\n");
				  break;
				}
				put_item_on_trade(in_data+3);
			}
			break;

		case REMOVE_TRADE_OBJECT:
			{
				if (data_length <= 8)
				{
				  LOG_WARNING("CAUTION: Possibly forged REMOVE_TRADE_OBJECT packet received.\n");
				  break;
				}
				remove_item_from_trade(in_data+3);
			}
			break;

		case GET_TRADE_PARTNER_NAME:
			{
				get_trade_partner_name(&in_data[3],SDL_SwapLE16(*((Uint16 *)(in_data+1)))-1);
			}
			break;

		case GET_YOUR_SIGILS:
			{
				// future support for more sigils
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_YOUR_SIGILS packet received.\n");
				  break;
				}
				if(data_length < 11){
					get_sigils_we_have(SDL_SwapLE32(*((Uint32 *)(in_data+3))), 0);
				} else {
					get_sigils_we_have(SDL_SwapLE32(*((Uint32 *)(in_data+3))), SDL_SwapLE32(*((Uint32 *)(in_data+7))));
				}
			}
			break;

		case GET_ACTIVE_SPELL:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_ACTIVE_SPELL packet received.\n");
				  break;
				}
				get_active_spell(in_data[3],in_data[4]);
			}
			break;

		case REMOVE_ACTIVE_SPELL:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged REMOVE_ACTIVE_SPELL packet received.\n");
				  break;
				}
				remove_active_spell(in_data[3]);
			}
			break;

		case GET_ACTIVE_SPELL_LIST:
			{
				if (data_length <= 2+NUM_ACTIVE_SPELLS)
				{
					LOG_WARNING("CAUTION: Possibly forged GET_ACTIVE_SPELL_LIST packet received.\n");
					break;
				}
				get_active_spell_list (in_data+3);
			}
			break;

		case GET_ACTOR_HEALTH:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_ACTOR_HEALTH packet received.\n");
				  break;
				}
				get_actor_health(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
			}
			break;

		case GET_ACTOR_DAMAGE:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef BANDWIDTH_SAVINGS
				// allow for multiple packets in a row
				while (data_length >= 7)
				{
					get_actor_damage(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
					data_length -= 4;
					in_data += 4;
				}
#else
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_ACTOR_DAMAGE packet received.\n");
				  break;
				}
				get_actor_damage(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
#endif
			}
			break;

		case GET_ACTOR_HEAL:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef BANDWIDTH_SAVINGS
				// allow for multiple packets in a row
				while (data_length >= 7)
				{
					get_actor_heal(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
					data_length -= 4;
					in_data += 4;
				}
#else
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_ACTOR_HEAL packet received.\n");
				  break;
				}
				get_actor_heal(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
#endif
			}
			break;

		case ACTOR_UNWEAR_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 5)
				{
				  LOG_WARNING("CAUTION: Possibly forged ACTOR_UNWEAR_ITEM packet received.\n");
				  break;
				}
				unwear_item_from_actor(SDL_SwapLE16(*((Uint16 *)(in_data+3))),in_data[5]);
			}
			break;

		case ACTOR_WEAR_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 6)
				{
				  LOG_WARNING("CAUTION: Possibly forged ACTOR_WEAR_ITEM packet received.\n");
				  break;
				}
				actor_wear_item(SDL_SwapLE16(*((Uint16 *)(in_data+3))),in_data[5],in_data[6]);
			}
			break;

		case NPC_SAY_OVERTEXT:
			{
				char buf[1024];
				if (data_length <= 5)
				{
				  LOG_WARNING("CAUTION: Possibly forged NPC_SAY_OVERTEXT packet received.\n");
				  break;
				}
				safe_strncpy2(buf, (char*)in_data + 5, sizeof(buf), data_length - 5);
				add_displayed_text_to_actor(
					get_actor_ptr_from_id( SDL_SwapLE16(*((Uint16 *)(in_data+3))) ), buf);
			}
			break;

		case PING_REQUEST:
			{
#ifdef	OLC
				// add in the status information
				char	buf[1024];
				int	len= data_length;
				memcpy(buf, in_data, data_length);
				len+= olc_ping_request(buf+len);
				my_tcp_send(my_socket, buf, len);
#else	//OLC
				// just send the pack back as it is
				my_tcp_send(my_socket, in_data, data_length);
#endif	//OLC
			}
			break;

		case BUDDY_EVENT:
			{
				if (data_length <= 5)
				{
				  LOG_WARNING("CAUTION: Possibly forged BUDDY_EVENT packet received.\n");
				  break;
				}
				if(in_data[3]==1)
					add_buddy((char*)&in_data[5],in_data[4],data_length-5);
				else if(in_data[3]==0)
					del_buddy((char*)&in_data[4],data_length-4);
			}
			break;

		case DISPLAY_CLIENT_WINDOW:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged DISPLAY_CLIENT_WINDOW packet received.\n");
				  break;
				}
				switch(in_data[3]){
					case RULE_WIN:
					case RULE_INTERFACE:
						highlight_rule(in_data[3],in_data+4,data_length-4);
						break;
					case NEW_CHAR_INTERFACE:
						hide_all_root_windows ();
						hide_hud_windows ();
						countdown=0;
						have_a_map=0;
						create_newchar_root_window ();
						show_window (newchar_root_win);
						connect_to_server();
						break;
					default:
						break;
				}
			}
			break;
		case OPEN_BOOK:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged OPEN_BOOK packet received.\n");
				  break;
				}
				open_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;

		case READ_BOOK:
			{
				if (data_length <= 7)
				{
				  LOG_WARNING("CAUTION: Possibly forged READ_BOOK packet received.\n");
				  break;
				}
				read_network_book((char*)in_data+3, data_length-3);
			}
			break;

		case CLOSE_BOOK:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged CLOSE_BOOK packet received.\n");
				  break;
				}
				close_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;
		case STORAGE_LIST:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged STORAGE_LIST packet received.\n");
				  break;
				}
				get_storage_categories((char*)in_data+3, data_length-3);
			}
			break;

		case STORAGE_ITEMS:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged STORAGE_ITEMS packet received.\n");
				  break;
				}
				get_storage_items(in_data+3, data_length-3);
				trade_post_storage();
			}
			break;

		case STORAGE_TEXT:
			{
				if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged STORAGE_TEXT packet received.\n");
				  break;
				}
				get_storage_text(in_data+3, data_length-3);
			}
			break;
		case SPELL_CAST:
			{
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged SPELL_CAST packet received.\n");
				  break;
				}
				if (((in_data[3] == S_SUCCES) || (in_data[3] == S_NAME)) && (data_length <= 4))
				{
				  LOG_WARNING("CAUTION(2): Possibly forged SPELL_CAST packet received.\n");
				  break;
				}
				process_network_spell((char*)in_data+3, data_length-3);
				if (in_data[3] == S_SUCCES) {
					// increment the spell counter
					increment_spell_counter(in_data[4]);
				}
       		}
			break;
		case GET_ACTIVE_CHANNELS:
			if (data_length <= 3)
			{
			  LOG_WARNING("CAUTION: Possibly forged GET_ACTIVE_CHANNELS packet received.\n");
			  break;
			}
			set_active_channels (in_data[3], (Uint32*)(in_data+4), (data_length-2)/4);
			break;

		case GET_3D_OBJ_LIST:
			if (data_length <= 3)
			{
			  LOG_WARNING("CAUTION: Possibly forged GET_3D_OBJ_LIST packet received.\n");
			  break;
			}
			get_3d_objects_from_server (in_data[3], &in_data[4], data_length - 4);
			break;

		case GET_3D_OBJ:
			get_3d_objects_from_server (1, &in_data[3], data_length - 3);
			break;

		case REMOVE_3D_OBJ:
			if (data_length <= 4)
			{
			  LOG_WARNING("CAUTION: Possibly forged REMOVE_3D_OBJ packet received.\n");
			  break;
			}
			remove_3d_object_from_server (SDL_SwapLE16 (*((Uint16 *)(&in_data[3]))));
			break;

		// for use by 1.0.3 server and higher
		case MAP_SET_OBJECTS:
			if (data_length <= 4)
			{
			  LOG_WARNING("CAUTION: Possibly forged MAP_SET_OBJECTS packet received.\n");
			  break;
			}
			switch(in_data[3]){
				case	0:	//2D
					set_2d_object(in_data[4], in_data+5, data_length-3);
					break;
				case	1:	//3D
					set_3d_object(in_data[4], in_data+5, data_length-3);
					break;
			}
			break;

		// for future expansion
		case MAP_STATE_OBJECTS:
			if (data_length <= 8)
			{
			  LOG_WARNING("CAUTION: Possibly forged MAP_STATE_OBJECTS packet received.\n");
			  break;
			}
			switch(in_data[3]){
				case	0:	//2D
					state_2d_object(in_data[4], in_data+5, data_length-3);
					break;
				case	1:	//3D
					state_3d_object(in_data[4], in_data+5, data_length-3);
					break;
			}
			break;

		case MAP_FLAGS:
			if (data_length <= 6)
			{
			  LOG_WARNING("CAUTION: Possibly forged MAP_FLAGS packet received.\n");
			  break;
			}
			map_flags=SDL_SwapLE32(*((Uint32 *)(in_data+3)));
			break;

		case GET_ITEMS_COOLDOWN:
				// make sure we interpret the incoming octets as unsigned
				// in case the function signature changes
			if (data_length <= 3)
			{
			  LOG_WARNING("CAUTION: Possibly forged GET_ITEMS_COOLDOWN packet received.\n");
			  break;
			}
			get_items_cooldown (&in_data[3], data_length - 3);
			break;

		case SEND_BUFFS:
#ifdef EXTRA_DEBUG
	ERR();
#endif
			if (data_length < 9)
			{
			  LOG_WARNING("CAUTION: Possibly forged SEND_BUFFS packet received.\n");
			  break;
			}
#ifdef BUFF_DEBUG
			{
				int actor_id = SDL_SwapLE16(*((short *)(in_data+3)));
				actor *act = get_actor_ptr_from_id(actor_id);
				if(act){
					printf("SEND_BUFFS received for actor %s\n", act->actor_name);
				}
				else {
					printf("SEND_BUFFS received for actor ID %i\n", actor_id);
				}
			}
#endif // BUFF_DEBUG
			update_actor_buffs(SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE32(*((Uint32 *)(in_data+5))));
			break;

		case SEND_SPECIAL_EFFECT:
			if (data_length <= 5)
			{
			  LOG_WARNING("CAUTION: Possibly forged SEND_SPECIAL_EFFECT packet received.\n");
			  break;
			}
			if (
			    (in_data[3] == SPECIAL_EFFECT_POISON) ||
			    (in_data[3] == SPECIAL_EFFECT_REMOTE_HEAL) ||
			    (in_data[3] == SPECIAL_EFFECT_HARM) ||
			    (in_data[3] == SPECIAL_EFFECT_MANA_DRAIN) ||
			    (in_data[3] == SPECIAL_EFFECT_INVASION_BEAMING) ||
			    (in_data[3] == SPECIAL_EFFECT_TELEPORT_TO_RANGE)
			   )
			{
				if (data_length <= 7)
				{
				  LOG_WARNING("CAUTION(2): Possibly forged SEND_SPECIAL_EFFECT packet received.\n");
				  break;
				}
			}
				if (special_effects){
					parse_special_effect(in_data[3], (const Uint16 *) &in_data[4]);
				}
			break;

		case REMOVE_MINE:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				remove_mine(in_data[3]);
			}
			break;

		case GET_NEW_MINE:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 8)
				{
					LOG_WARNING("CAUTION: Possibly forged GET_NEW_MINE packet received.\n");
					break;
				}
				put_mine_on_ground(SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))), in_data[8], in_data[7]);
			}
			break;

		case GET_MINES_LIST:
			{
				Uint16 mines_no;
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if (data_length <= 3)
				{
				  LOG_WARNING("CAUTION: Possibly forged GET_MINES_LIST packet received.\n");
				  break;
				}
				mines_no = in_data[3];
				if (data_length <= mines_no * 6 + 3)
				{
				  LOG_WARNING("CAUTION(2): Possibly forged GET_MINES_LIST packet received.\n");
				  break;
				}
				add_mines_from_list(&in_data[3]);
			}
			break;
		case DISPLAY_POPUP:
			{
				if (data_length <= 8) /* At least one char title and one char text */
				{
					LOG_WARNING("CAUTION: Possibly forged DISPLAY_POPUP packet received.\n");
					break;
				}
				popup_create_from_network(&in_data[3], data_length - 3);
			}
			break;
		case MISSILE_AIM_A_AT_B:
			if (data_length >= 7)
			{
				missiles_aim_at_b(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))));
			}
			break;
		case MISSILE_AIM_A_AT_XYZ:
			if (data_length >= 17)
			{
				float target[3];
				target[0] = SwapLEFloat(*((float*)(in_data+5)));
				target[1] = SwapLEFloat(*((float*)(in_data+9)));
				target[2] = SwapLEFloat(*((float*)(in_data+13)));
				missiles_aim_at_xyz(SDL_SwapLE16(*((short *)(in_data+3))),target);
			}
			break;
		case MISSILE_FIRE_A_TO_B:
			if (data_length >= 7)
			{
				missiles_fire_a_to_b(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))));
			}
			break;
		case MISSILE_FIRE_A_TO_XYZ:
			if (data_length >= 17)
			{
				float target[3];
				target[0] = SwapLEFloat(*((float*)(in_data+5)));
				target[1] = SwapLEFloat(*((float*)(in_data+9)));
				target[2] = SwapLEFloat(*((float*)(in_data+13)));
				missiles_fire_a_to_xyz(SDL_SwapLE16(*((short *)(in_data+3))),target);
			}
			break;
		case MISSILE_FIRE_XYZ_TO_B:
			if (data_length >= 17)
			{
				float source[3];
				source[0] = SwapLEFloat(*((float*)(in_data+5)));
				source[1] = SwapLEFloat(*((float*)(in_data+9)));
				source[2] = SwapLEFloat(*((float*)(in_data+13)));
				missiles_fire_xyz_to_b(source,SDL_SwapLE16(*((short *)(in_data+3))));
			}
			break;
		case SEND_MAP_MARKER:
			{
			//in_data[3]=id
			//in_data[5]=x;
			//in_data[7]=y;
			//map_name\0text\0
			int i,fl=0,k=0;
			server_mark *sm;

			if (data_length <= 10)
				{
				  LOG_WARNING("CAUTION: Possibly forged SEND_MAP_MARKER packet received.\n");
				  break;
				}
			sm = calloc(1,sizeof(server_mark));//memory is set to zero
			sm->x=SDL_SwapLE16(*((short *)(in_data+5)));
			sm->y=SDL_SwapLE16(*((short *)(in_data+7)));
			sm->id=SDL_SwapLE16(*((short *)(in_data+3)));
			for(i=9;i<data_length;i++){
				if(in_data[i]==0) {fl=1; k=0; continue;}
				if(!fl) sm->map_name[k++]=in_data[i]; //reading map name
				else sm->text[k++]=in_data[i];//reading mark text
			}
			//printf("ADD MARKER: %i %i %i %s %s\n",sm->id,sm->x,sm->y,sm->map_name,sm->text);
			if(!server_marks) init_server_markers();
			hash_delete(server_marks,(NULL+sm->id)); //remove old marker if present
			hash_add(server_marks,(NULL+sm->id),(void*)sm);
			save_server_markings();
			load_map_marks();//load again, so the new marker is added correctly.
			break;
			}
		case REMOVE_MAP_MARKER:
			{
			int id;
			if (data_length <= 4)
				{
				  LOG_WARNING("CAUTION: Possibly forged REMOVE_MAP_MARKER packet received.\n");
				  break;
				}
			id=SDL_SwapLE16(*((short *)(in_data+3)));
			hash_delete(server_marks,(NULL+id)); //remove marker if present
			save_server_markings();			
			load_map_marks();//load again, so the new marker is removed correctly.
			break;
			}
		case NEXT_NPC_MESSAGE_IS_QUEST:
			{
				if (data_length <= 4)
				{
					LOG_WARNING("CAUTION: Possibly forged NEXT_NPC_MESSAGE_IS_QUEST packet received.\n");
					break;
				}
				set_next_quest_entry_id(SDL_SwapLE16(*((short *)(in_data+3))));
				break;
			}
		case HERE_IS_QUEST_ID:
			{
				if (data_length <= 3)
				{
					LOG_WARNING("CAUTION: Possibly forged HERE_IS_QUEST_ID packet received.\n");
					break;
				}
				set_quest_title((const char *)&in_data[3], data_length - 3);
				break;
			}
		case QUEST_FINISHED:
			{
				if (data_length <= 4)
				{
					LOG_WARNING("CAUTION: Possibly forged QUEST_FINISHED packet received.\n");
					break;
				}
				set_quest_finished(SDL_SwapLE16(*((short *)(in_data+3))));
				break;
			}
		case SEND_ACHIEVEMENTS:
			{
				Uint32 *achievement_data = NULL;
				size_t word_count = (data_length-3) / sizeof(Uint32);
				size_t i;
				if ((word_count < 1) && (word_count < MAX_ACHIEVEMENTS/32))
				{
					LOG_WARNING("CAUTION: Possibly forged SEND_ACHIEVEMENTS packet received.\n");
					break;
				}
				achievement_data = (Uint32 *)calloc(word_count, sizeof(Uint32));
				for (i=0; i<word_count; ++i)
					achievement_data[i] = SDL_SwapLE32(*((Uint32 *)(in_data+3+i*sizeof(Uint32))));
				achievements_data(achievement_data, word_count);
				free(achievement_data);
			}
			break;
		default:
			{
				// Unknown packet type??
#ifdef	OLC
				// do OL specific packet handling
				olc_packet_handler(in_data, data_length);
#endif	//OLC
			}
			break;
		}
}

static void process_data_from_server(queue_t *queue)
{
	/* enough data present for the length field ? */
	if (3 <= in_data_used) {
		Uint8   *pData  = tcp_in_data;
		Uint16   size;

		do { /* while (3 <= in_data_used) (enough data present for the length field) */
			size = SDL_SwapLE16(*((short*)(pData+1)));
			size += 2; /* add length field size */

			if (sizeof (tcp_in_data) - 3 >= size) { /* buffer big enough ? */

				if (size <= in_data_used) { /* do we have a complete message ? */
					message_t *message = malloc(sizeof *message);
					message->data = malloc(size*sizeof(unsigned char));
					message->length = size;
					memcpy(message->data, pData, size);
					queue_push(queue, message);

					if (log_conn_data){
						log_conn(pData, size);
					}

					/* advance to next message */
					pData         += size;
					in_data_used  -= size;
				} else {
					break;
				}
			}
			else { /* sizeof (tcp_in_data) - 3 < size */
				LOG_TO_CONSOLE(c_red2, packet_overrun);

				LOG_TO_CONSOLE(c_red2, disconnected_from_server);
				LOG_TO_CONSOLE(c_red2, alt_x_quit);
				LOG_ERROR ("Packet overrun, protocol = %d, size = %u\n", pData[0], size);
				in_data_used = 0;
				disconnected = 1;
#ifdef NEW_SOUND
				stop_all_sounds();
				do_disconnect_sound();
#endif // NEW_SOUND
				disconnect_time = SDL_GetTicks();
			}
		} while (3 <= in_data_used);

		/* move the remaining data to the start of the buffer...(not tested...never happened to me) */
		if (in_data_used > 0 && pData != tcp_in_data){
			memmove(tcp_in_data, pData, in_data_used);
		}
	}
}

int get_message_from_server(void *thread_args)
{
	int received;
	void *queue = ((void **) thread_args)[0];
	int *done = ((void **) thread_args)[1];

	init_thread_log("server_message");

	while(!*done)
	{
		// Sleep while disconnected
		if(disconnected){
			SDL_Delay(100);	// 10 times per second should be often enough
			continue; //Continue to make the main loop check int done.
		} else if(SDLNet_CheckSockets(set, 100) <= 0 || !SDLNet_SocketReady(my_socket)) {
			//if no data, loop back and check again, the delay is in SDLNet_CheckSockets()
			continue; //Continue to make the main loop check int done.
		}

		if ((received = SDLNet_TCP_Recv(my_socket, &tcp_in_data[in_data_used], sizeof (tcp_in_data) - in_data_used)) > 0) {
			in_data_used += received;
			process_data_from_server(queue);
		}
		else { /* 0 >= received (EOF or some error) */
			char str[256];
			short tgm = real_game_minute;
			if (received)
				safe_snprintf(str, sizeof(str), "<%1d:%02d>: %s: [%s]", tgm/60, tgm%60, disconnected_from_server, SDLNet_GetError());
		 	else
				safe_snprintf(str, sizeof(str), "<%1d:%02d>: %s", tgm/60, tgm%60, disconnected_from_server);
			LOG_TO_CONSOLE(c_red2, str);
			LOG_TO_CONSOLE(c_red2, alt_x_quit);
			in_data_used = 0;
			disconnected = 1;
#ifdef NEW_SOUND
			stop_all_sounds();
			do_disconnect_sound();
#endif // NEW_SOUND
			disconnect_time = SDL_GetTicks();
		}
	}

	return 1;
}
