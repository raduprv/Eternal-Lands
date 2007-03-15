//#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "elwindows.h"
#include "queue.h"
#include "actors.h"
#include "update.h"
#ifdef SFX
#include "special_effects.h"
#ifdef	EYE_CANDY
#include "eye_candy_wrapper.h"
#endif	//EYE_CANDY
#endif // SFX

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void get_updates();
 */

const char * web_update_address= "http://www.eternal-lands.com/index.php?content=update";
int icon_in_spellbar= -1;
int port= 2000;
unsigned char server_address[60];
TCPsocket my_socket= 0;
SDLNet_SocketSet set= 0;
#define MAX_TCP_BUFFER  8192
Uint8 tcp_in_data[MAX_TCP_BUFFER];
Uint8 tcp_out_data[MAX_TCP_BUFFER];
int tcp_out_loc= 0;
int previously_logged_in= 0;
time_t last_heart_beat;

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
actor *your_actor= NULL;

int last_sit= 0;

void move_to (short int x, short int y)
{
	Uint8 str[5];
				
	str[0]= MOVE_TO;
	*((short *)(str+1))= SDL_SwapLE16 (x);
	*((short *)(str+3))= SDL_SwapLE16 (y);
	my_tcp_send(my_socket, str, 5);
}

int on_the_move (const actor *act)
{
	if (act == NULL) return 0;
	return act->moving || (act->que[0] >= move_n && act->que[0] <= move_nw);
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


int my_tcp_send (TCPsocket my_socket, const Uint8 *str, int len)
{
	if(disconnected) {
		return 0;
	}

	if(tcp_out_loc > 0 && tcp_out_loc + len + 2 >= MAX_TCP_BUFFER){
		// wouldn't fit, send what we have
		my_tcp_flush(my_socket);
	}
	
	// LabRat's anti-bagspam code
	// Grum: Adapted. Converting every movement to a path caused too much
	// trouble. Instead we now check the current actor animation for
	// movement.
	if (str[0] == DROP_ITEM  && on_the_move (your_actor))
	{
		// I thought about having a bit of code here that counts attempts, and after say 5,
		// announces on #abuse something like "#abuse I attempted to bagspam, but was thwarted,
		// please teach me the error of my ways so I refrain from doing so in the future"
		/*if((spamcount++)>4)
		{
			Uint8 badstr[256];
			sprintf (badstr, "#abuse I attempted to bagspam %i bags, but was thwarted, please correct me", spamcount);
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
		if(str[0]==MOVE_TO || str[0]==RUN_TO || str[0]==SIT_DOWN || str[0]==HARVEST || str[0]==MANUFACTURE_THIS || str[0]==CAST_SPELL || str[0]==RESPOND_TO_NPC || str[0]==ATTACK_SOMEONE || str[0]==SEND_PM || str[0]==RAW_TEXT)
		{
			Uint32	time_limit= 600;

			if( str[0]==SEND_PM || str[0]==RAW_TEXT)time_limit=1500;
			if( str[0]==SIT_DOWN){
				if(last_sit+1500>cur_time) return 0;
				last_sit= cur_time;
			}
			//if too close together
			if(len == (int)tcp_cache_len && *str == *tcp_cache && cur_time < tcp_cache_time+time_limit)
				{
					//and the same packet
					if(!memcmp(str, tcp_cache, len))
						{
							//ignore this packet
							return 0;
						}
				}
			//memorize the data we are sending for next time
			memcpy(tcp_cache, str, len);
			tcp_cache_len= len;
			tcp_cache_time= cur_time;
		}
	}
	//update the heartbeat timer
	last_heart_beat= time(NULL);

	// check to see if the data would fit in the buffer
	if(len + 2 < MAX_TCP_BUFFER){
		// yes, buffer it for later processing
		tcp_out_data[tcp_out_loc]= str[0];	//copy the protocol byte
		*((short *)(tcp_out_data+tcp_out_loc+1))= SDL_SwapLE16((Uint16)len);//the data length
		// copy the rest of the data
		memcpy(&tcp_out_data[tcp_out_loc+3], &str[1], len-1);
		// adjust then buffer offset
		tcp_out_loc+= len+2;
		return(len+2);
	} else {
		// no, send it as is now
		Uint8 new_str[1024];	//should be enough

		new_str[0]= str[0];	//copy the protocol byte
		*((short *)(new_str+1))= SDL_SwapLE16((Uint16)len);//the data length
		// copy the rest of the data
		memcpy(&new_str[3], &str[1], len-1);
		//for(i=1; i<len; i++) {
		//	new_str[i+2]= str[i];
		//}
#ifdef	OLC
		return olc_tcp_send(my_socket, new_str, len+2);
#else	//OLC
		return SDLNet_TCP_Send(my_socket, new_str, len+2);
#endif	//OLC
	}
	// error, should never reach here
	return 0;
}

int my_tcp_flush (TCPsocket my_socket)
{
	int ret;
	
	if(disconnected || tcp_out_loc == 0) {
		return 0;
	}

	// if we are already sending data, lets see about sending a heartbeat a little bit early
	if(last_heart_beat+20 <= time(NULL)){
		send_heart_beat();
	}

	// send all the data in the buffer
#ifdef	OLC
	ret= olc_tcp_send(my_socket, tcp_out_data, tcp_out_loc);
	if(ret > 0){
		ret= olc_tcp_flush();
	}
#else	//OLC
	ret= SDLNet_TCP_Send(my_socket, tcp_out_data, tcp_out_loc);
#endif	//OLC

	// empty the buffer
	tcp_out_loc= 0;
	
	return(ret);
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
            log_error("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
			SDLNet_Quit();
			SDL_Quit();
			exit(4); //most of the time this is a major error, but do what you want.
        }

	if(SDLNet_ResolveHost(&ip,server_address,port)==-1)
		{
			LOG_TO_CONSOLE(c_red2,failed_resolve);
			return;
		}

	my_socket= SDLNet_TCP_Open(&ip);
	if(!my_socket)
		{
			LOG_TO_CONSOLE(c_red1,failed_connect);
			LOG_TO_CONSOLE(c_red1,reconnect_str);
			LOG_TO_CONSOLE(c_red1,alt_x_quit);
			return;
		}

	if(SDLNet_TCP_AddSocket(set,my_socket)==-1)
		{
			log_error("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
			SDLNet_Quit();
			SDL_Quit();
			exit(2);
		}
	disconnected= 0;

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
	last_heart_beat= time(NULL);
	send_heart_beat();	// prime the hearbeat to prevent some stray issues when there is lots of lag
	hide_window(trade_win);
	
	//BUDDY-FIXME: once server-side offline buddies are supported, the next 4 lines can go
	//For the buddy notifications
	if(time(NULL) > c_time) {
		time(&c_time);//note the current time
	}

	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}

void send_login_info()
{
	int i,j,len;
	unsigned char str[40];

	len= strlen(username_str);
	//check for the username length
	if(len<3)
		{
			sprintf(log_in_error_str,"%s: %s",reg_error_str,error_username_length);
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

	len= strlen(str);
	len++;//send the last 0 too
	if(my_tcp_send(my_socket, str, len)<len)
		{
			//we got a nasty error, log it
		}
	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}


void send_new_char(Uint8 * user_str, Uint8 * pass_str, char skin, char hair, char shirt, char pants, char boots,char head, char type)
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
	create_char_error_str[0]= 0;//no error
	my_tcp_flush(my_socket);    // make sure tcp output buffer is empty
}

// TEMP LOGAND [5/25/2004]
#ifndef NPC_SAY_OVERTEXT
 #define NPC_SAY_OVERTEXT 58 
#endif
//---

void process_message_from_server (const Uint8 *in_data, int data_length)
{
#ifdef EYE_CANDY	//Test code; delete when Eye Candy effects are well-in.
	ec_bounds bounds;
	ec_reference /* ref, */ ref1, ref2;
	ec_effects eff_list;
#endif //EYE_CANDY

	Uint8 text_buf[MAX_TCP_BUFFER];
	
	//see what kind of data we got
	switch (in_data[PROTOCOL])
		{
		case RAW_TEXT:
			{
				int len= data_length - 4;

				// extract the channel number
				if (data_length > 4) 
				{
					if (len > sizeof(text_buf) - 1)
					{
						len= sizeof(text_buf) - 1;
					}
					memcpy(text_buf, &in_data[4], len);
					text_buf[len] = '\0';
					
					// do filtering and ignoring
					len= filter_or_ignore_text(text_buf, len, sizeof (text_buf), in_data[3]);
					if (len > 0)
					{
						/* if from the server popup channel, and popup window usage enable */
						if (use_server_pop_win && (in_data[3] == server_pop_chan))
							display_server_popup_win(text_buf);
						/* else write to the chat window/console */
						else
							put_text_in_buffer(in_data[3], text_buf, len);
					}
				}
			}
			break;

		case ADD_NEW_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_actor_from_server(&in_data[3], data_length-3);
			}
			break;

		case ADD_NEW_ENHANCED_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_enhanced_actor_from_server(&in_data[3], data_length-3);
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
#ifdef EXTRA_DEBUG
	ERR();
#endif
				game_minute= SDL_SwapLE16(*((short *)(in_data+3)));
				new_minute();
#ifdef	EYE_CANDY
///////////////////////////////////////////////////////////////////////////////
// Special Effects Test Code #1/2 (Repeating effects)
// Uncomment an effect to test it; delete section when FX are integrated.
// NOTE: When actually calling functions for real, they should have their
//       returned references saved, and freed when no longer needed.
///////////////////////////////////////////////////////////////////////////////
//				ec_create_bag_pickup(49.0, 70.0, 0.0, 10);
//				ec_create_bag_drop(49.0, 70.0, 0.0, 10);
//				ec_create_breath_fire(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_breath_ice(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_breath_poison(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_breath_magic(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_breath_lightning(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_breath_wind(49.0, 70.0, 1.5, 52, 70, 0.2, NULL, 10, 2.0);
//				ec_create_harvesting_radon_pouch(49.0, 70.0, 0.0, 10);
//				ec_create_harvesting_cavern_wall(49.0, 70.0, 0.0, 10);
//				ec_create_harvesting_mother_nature(49.0, 70.0, 0.0, 10);
//				ec_create_harvesting_queen_of_nature(49.0, 70.0, 0.0, 10);
//				ec_create_harvesting_bees(49.0, 70.0, 0.3, 10);
//				ec_create_harvesting_bag_of_gold(48.5, 70.0, 0.3, 10);
//				ec_create_harvesting_rare_stone(48.5, 70.0, 0.3, 10);
//				ec_create_impact_magic_protection(49.0, 70.0, 0.5, -1.0, 0.0, 0.0, 10, 5.0);
//				ec_create_impact_shield(49.0, 70.0, 0.5, -1.0, 0.0, 0.0, 10, 5.0);
//				ec_create_impact_magic_immunity(49.0, 70.0, 0.5, -1.0, 0.0, 0.0, 10, 5.0);
//				ec_create_impact_poison(49.0, 70.0, 0.5, -1.0, 0.0, 0.0, 10, 5.0);
//				ec_create_impact_blood(49.0, 70.0, 0.5, -1.0, 0.0, 0.0, 10, 5.0);
//				ec_create_selfmagic_heal(49.0, 70.0, 0.1, 10);
//				ec_create_selfmagic_magic_protection(49.0, 70.0, 0.0, 10);
//				ec_create_selfmagic_shield(49.0, 70.0, 0.1, 10);
//				ec_create_selfmagic_restoration(49.0, 70.0, 0.1, 10);
//				ec_create_selfmagic_bones_to_gold(49.0, 70.0, 0.1, 10);
//				ec_create_selfmagic_teleport_to_the_portals_room(49.0, 70.0, 0.1, 10);
//				ec_create_selfmagic_magic_immunity(49.0, 70.0, 0.1, 10);
//				ec_create_alert(49.25, 70.25, 0.1, 10);
//				ec_create_summon_rabbit(49.0, 70.0, 0.0, 10);
//				ec_create_summon_rat(49.0, 70.0, 0.0, 10);
//				ec_create_summon_beaver(49.0, 70.0, 0.0, 10);
//				ec_create_summon_deer(49.0, 70.0, 0.0, 10);
//				ec_create_summon_green_snake(49.0, 70.0, 0.0, 10);
//				ec_create_summon_red_snake(49.0, 70.0, 0.0, 10);
//				ec_create_summon_brown_snake(49.0, 70.0, 0.0, 10);
//				ec_create_summon_fox(49.0, 70.0, 0.0, 10);
//				ec_create_summon_boar(49.0, 70.0, 0.0, 10);
//				ec_create_summon_wolf(49.0, 70.0, 0.0, 10);
//				ec_create_summon_puma(49.0, 70.0, 0.0, 10);
//				ec_create_summon_bear(49.0, 70.0, 0.0, 10);
//				ec_create_summon_skeleton(49.0, 70.0, 0.0, 10);
//				ec_create_summon_small_gargoyle(49.0, 70.0, 0.0, 10);
//				ec_create_summon_medium_gargoyle(49.0, 70.0, 0.0, 10);
//				ec_create_summon_large_gargoyle(49.0, 70.0, 0.0, 10);
//				ec_create_summon_fluffy(49.0, 70.0, 0.0, 10);
//				ec_create_summon_chimeran_wolf(49.0, 70.0, 0.0, 10);
//				ec_create_summon_yeti(49.0, 70.0, 0.0, 10);
//				ec_create_summon_arctic_chimeran(49.0, 70.0, 0.0, 10);
//				ec_create_summon_giant(49.0, 70.0, 0.0, 10);
//				ec_create_targetmagic_remote_heal(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
//				ec_create_targetmagic_poison(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
//				ec_create_targetmagic_teleport_to_range(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
//				ec_create_targetmagic_harm(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
//				ec_create_targetmagic_life_drain(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
//				ec_reference ref;
//				ref = ec_create_generic();
//				ec_add_target(ref, 52.0, 70.0, 0.5);
//				ec_add_target(ref, 50.0, 68.0, 0.5);
//				ec_add_target(ref, 50.0, 72.0, 0.5);
//				ec_launch_targetmagic_heal_summoned(ref, 49.0, 70.0, 0.5, NULL, 10);
//				ec_reference ref;
//				ref = ec_create_generic();
//				ec_add_target(ref, 52.0, 70.0, 0.5);
//				ec_add_target(ref, 50.0, 68.0, 0.5);
//				ec_add_target(ref, 50.0, 72.0, 0.5);
//				ec_launch_targetmagic_smite_summoned(ref, 49.0, 70.0, 0.5, NULL, 10);
//				ec_create_targetmagic_drain_mana(49.0, 70.0, 0.5, 52.0, 70.0, 0.5, NULL, 10);
///////////////////////////////////////////////////////////////////////////////
#endif	//EYE_CANDY
			}
			break;

		case LOG_IN_OK:
			{
				// login and/or new character windows are no longer needed
				if (login_root_win >= 0) destroy_window (login_root_win);
				login_root_win = -1;
				if (newchar_root_win >= 0) {
					destroy_window (newchar_root_win);
					hide_window( namepass_win );
					hide_window( color_race_win );
				}
				newchar_root_win = -1;
				show_window (game_root_win);

				load_quickspells();
				
#ifdef COUNTERS
				load_counters();
#endif
				
				previously_logged_in=1;

#ifdef	EYE_CANDY
///////////////////////////////////////////////////////////////////////////////
// Special Effects Test Code #2/2 (Lasting effects)
// Uncomment an effect to test it; delete section when FX are integrated.
// NOTE: When actually calling functions for real, they should have their
//       returned references saved, and freed when no longer needed.
///////////////////////////////////////////////////////////////////////////////
				bounds = ec_create_bounds_list();
				ec_add_polar_coords_bound(bounds, 0.0, 0.0, 8.0, 1.0);
				ec_add_polar_coords_bound(bounds, 2.0, 0.0, 3.0, 1.0);
				ec_add_polar_coords_bound(bounds, 5.0, 0.1, 1.5, 0.7);
//				ec_create_campfire(49.0, 70.0, 0.0, NULL, 10, 1.0);
//				ec_create_cloud(45.0, 65.0, 1.0, 1.0, bounds, 10);
//				ec_create_fireflies(49.0, 70.0, 0.0, NULL, 1.0, bounds);
//				ec_create_fountain(49.0, 70.0, 0.2, 0.0, 0, 1.0, 10);
//				ec_create_lamp(47.4, 69.6, 3.3, 10);
//				ec_create_ongoing_magic_protection(49.0, 70.0, 0.0, 10, 5.0);
//				ec_create_ongoing_shield(49.0, 70.0, 0.0, 10, 5.0);
//				ec_create_ongoing_magic_immunity(49.0, 70.0, 0.0, 10, 5.0);
//				ec_create_ongoing_poison(49.0, 70.0, 0.0, 10, 5.0);
//				ec_create_smoke(49.0, 70.0, 0.0, 1.0, 10);
//				// Sword effects won't be very visible because the swords won't be moving.
//				ec_create_sword_serpent(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_cutlass(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_emerald_claymore(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_sunbreaker(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_orc_slayer(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_eagle_wing(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_jagged_saber(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_of_fire(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_of_ice(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_sword_of_magic(49.0, 70.0, 0.5, 49.5, 70.0, 0.0, 10);
//				ec_create_teleporter(49.0, 70.0, 0.0, 10);
				ref1 = ec_create_wind_leaves(49.0, 70.0, 0.05, NULL, 1.0, bounds, 1.0, 0.0, 0.0);
				ref2 = ec_create_wind_leaves(59.0, 70.0, 0.05, NULL, 1.0, bounds, 1.0, 0.0, 0.0);
				eff_list = ec_create_effects_list();
				ec_add_effect(eff_list, ref2);
				ec_add_wind_effect_list(ref1, eff_list);
				ec_free_effects_list(eff_list);
//				eff_list = ec_create_effects_list();
//				ec_add_effect(eff_list, ref1);
//				ec_add_wind_effect_list(ref2, eff_list);
//				ec_free_effects_list(eff_list);
//				ec_create_wind_petals(49.0, 70.0, 0.05, NULL, 1.0, bounds, 1.0, 0.0, 0.0);
///////////////////////////////////////////////////////////////////////////////
#endif	//EYE_CANDY
			}
			break;

		case HERE_YOUR_STATS:
			{
				get_the_stats((Sint16 *)(in_data+3));
			}
			break;

		case SEND_PARTIAL_STAT:
			{
				// allow for multiple stats in a row
				while(data_length >= 8){
					get_partial_stat(*((Uint8 *)(in_data+3)),SDL_SwapLE32(*((Sint32 *)(in_data+4))));
					in_data+= 5;
					data_length-= 5;
				}
			}
			break;

		case GET_KNOWLEDGE_LIST:
			{
				get_knowledge_list(SDL_SwapLE16(*(Uint16 *)(in_data+1))-1, in_data+3);
			}
			break;

		case GET_NEW_KNOWLEDGE:
			{
				get_new_knowledge(SDL_SwapLE16(*(Uint16 *)(in_data+3)));
			}
			break;

		case HERE_YOUR_INVENTORY:
			{
				get_your_items(in_data+3);
			}
			break;

		case GET_NEW_INVENTORY_ITEM:
			{
				// allow for multiple packets in a row
				while(data_length >= 11){
					get_new_inventory_item(in_data+3);
					in_data+= 8;
					data_length-= 8;
				}
			}
			break;

		case REMOVE_ITEM_FROM_INVENTORY:
			{
				// allow for multiple packets in a row
				while(data_length >= 4){
					remove_item_from_inventory(*((Uint8 *)(in_data+3)));
					in_data+= 1;
					data_length-= 1;
				}
			}
			break;

		case INVENTORY_ITEM_TEXT:
			{
				put_small_text_in_box(&in_data[3],data_length-3,6*items_grid_size+100,items_string);
				if(!(get_show_window(items_win)||get_show_window(manufacture_win)||get_show_window(trade_win)))
					{
						put_text_in_buffer(CHAT_SERVER, &in_data[3], data_length-3);
					}
#ifdef COUNTERS
					if ( (data_length > 27) && (!strncmp(in_data+4, "You successfully created 1 ", 27)) ) {
						char *product = malloc(data_length-4-27+1);

						strncpy(product, in_data+4+27, data_length-4-27);
						product[data_length-4-27] = '\0';
						
						counters_set_product_name(product);
					}
#endif
			}
			break;
		case SPELL_ITEM_TEXT:
			{
				put_small_text_in_box(in_data+3,data_length-3,6*51+100,spell_text);
				if(sigil_win==-1||!windows_list.window[sigil_win].displayed)
					put_text_in_buffer (CHAT_SERVER, in_data+3, data_length-3);
				have_error_message=1;
			}
			break;

		case GET_KNOWLEDGE_TEXT:
			{
				put_small_text_in_box(&in_data[3],data_length-3,6*51+150,knowledge_string);
			}
			break;

		case CHANGE_MAP:
			{
				change_map(in_data+3);
			}
			break;

		case GET_TELEPORTERS_LIST:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_teleporters_from_list(&in_data[3]);
			}
			break;

		case PLAY_MUSIC:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if(music_on)play_music(SDL_SwapLE16(*((short *)(in_data+3))));
			}
			break;

		case PLAY_SOUND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
#ifdef NEW_SOUND
				if(!sound_on)add_sound_object(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))),SDL_SwapLE16(*((short *)(in_data+7))));
#else
				if(!sound_on)add_sound_object(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))),SDL_SwapLE16(*((short *)(in_data+7))),SDL_SwapLE16(*((short *)(in_data+9))),SDL_SwapLE16(*((short *)(in_data+11))));
#endif
			}
			break;

		case TELEPORT_OUT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif

#ifdef	NEW_FRUSTUM
				add_particle_sys_at_tile("./particles/teleport_in.part", SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE16 (*((short *)(in_data+5))), 1);
#else
				add_particle_sys_at_tile ( "./particles/teleport_in.part", SDL_SwapLE16 ( *( (short *)(in_data+3) ) ), SDL_SwapLE16 ( *( (short *)(in_data+5) ) ) );
#endif
			}
			break;

		case TELEPORT_IN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif

#ifdef	NEW_FRUSTUM
				add_particle_sys_at_tile("./particles/teleport_in.part", SDL_SwapLE16(*((short *)(in_data+3))), SDL_SwapLE16(*((short *)(in_data+5))), 1);
#else
				add_particle_sys_at_tile ( "./particles/teleport_in.part", SDL_SwapLE16 ( *( (short *)(in_data+3) ) ), SDL_SwapLE16 ( *( (short *)(in_data+5) ) ) );
#endif
			}
#ifndef	NEW_FRUSTUM
			regenerate_near_objects=1;//Regenerate the near 3d objects...
#endif
			break;
		case LOG_IN_NOT_OK:
			{
				set_login_error (&in_data[3], data_length - 3);
			}
			break;

		case REDEFINE_YOUR_COLORS:
			{
				strcpy(log_in_error_str,redefine_your_colours);
			}
			break;

		case YOU_DONT_EXIST:
			{
				sprintf(log_in_error_str,"%s: %s",reg_error_str,char_dont_exist);
			}
			break;


		case CREATE_CHAR_NOT_OK:
			{
				set_create_char_error (&in_data[3], data_length - 3);
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
				LOCK_ACTORS_LISTS();
				yourself= SDL_SwapLE16(*((short *)(in_data+3)));
				your_actor= get_actor_ptr_from_id(yourself);
				UNLOCK_ACTORS_LISTS();
			}
			break;

		case START_RAIN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				float severity;

				if (data_length > 4) {
					severity= 0.1f + 0.9f*(*((Uint8 *)(in_data+4))/255.0f);
				} else {
					severity= 1.0f;
				}
				start_weather(*((Uint8 *)(in_data+3)), severity);
			}
			break;

		case STOP_RAIN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				float severity;

				if (data_length > 4) {
					severity= 0.1f + 0.9f*(*((Uint8 *)(in_data+4))/255.0f);
				} else {
					severity= 1.0f;
				}
				stop_weather(*((Uint8 *)(in_data+3)), severity);
			}
			break;

		case THUNDER:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_thunder(rand()%5,*((Uint8 *)(in_data+3)));
			}
			break;


		case SYNC_CLOCK:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				server_time_stamp= SDL_SwapLE32(*((int *)(in_data+3)));
				client_time_stamp= SDL_GetTicks();
				client_server_delta_time= server_time_stamp-client_time_stamp;
			}
			break;

		case PONG:
			{
				Uint8 str[160];
				sprintf(str,"%s: %i ms",server_latency, SDL_GetTicks()-SDL_SwapLE32(*((Uint32 *)(in_data+3))));
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
				put_bag_on_ground(SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))),*((Uint8 *)(in_data+7)));
			}
			break;

		case GET_BAGS_LIST:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_bags_from_list(&in_data[3]);
			}
			break;

		case SPAWN_BAG_PARTICLES:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif

#ifdef	NEW_FRUSTUM
//				add_particle_sys_at_tile("./particles/bag_in.part", SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))), 1);
#else
//				add_particle_sys_at_tile ( "./particles/bag_in.part", SDL_SwapLE16 ( *( (Uint16 *)(in_data+3) ) ), SDL_SwapLE16 ( *( (Uint16 *)(in_data+5) ) ) );
#endif
			}
			break;

		case FIRE_PARTICLES:
			{
#ifdef EXTRA_DEBUG
				ERR();
#endif
				add_fire_at_tile (SDL_SwapLE16(*(Uint16 *)(in_data+7)), SDL_SwapLE16(*((Uint16 *)(in_data+3))), SDL_SwapLE16(*((Uint16 *)(in_data+5))));
			}
			break;

		case REMOVE_FIRE_AT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				remove_fire_at_tile (SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16 *)(in_data+5))));
			}
			break;

		case GET_NEW_GROUND_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				get_bag_item(in_data+3);
			}
			break;

		case HERE_YOUR_GROUND_ITEMS:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
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
				put_small_text_in_box(&in_data[3],data_length-3,dialogue_menu_x_len-70,dialogue_string);
				display_dialogue();
				if (in_data[3] >= 127 && in_data[4] >= 127)
				{
					int len = data_length - 4;
					if (len > sizeof (text_buf) - 1)
						len = sizeof (text_buf) - 1;
					memcpy (text_buf, &in_data[4], len);
					text_buf[len] = '\0';
					add_questlog (text_buf, len);
				}
			}
			break;

		case SEND_NPC_INFO:
			{
				my_strcp(npc_name,&in_data[3]);
				cur_portrait=in_data[23];
			}
			break;

		case NPC_OPTIONS_LIST:
			{
				build_response_entries(&in_data[3],SDL_SwapLE16(*((Uint16 *)(in_data+1))));
			}
			break;

		case GET_TRADE_ACCEPT:
			{
				if(!in_data[3])
					trade_you_accepted++;
				else
					trade_other_accepted++;
			}
			break;

		case GET_TRADE_REJECT:
			{
				if(!in_data[3])trade_you_accepted=0;
				else
					trade_other_accepted=0;
			}
			break;

		case GET_TRADE_EXIT:
			{
				hide_window(trade_win);
			}
			break;

		case GET_YOUR_TRADEOBJECTS:
			{
				get_your_trade_objects(in_data+3);
			}
			break;

		case GET_TRADE_OBJECT:
			{
				put_item_on_trade(in_data+3);
			}
			break;

		case REMOVE_TRADE_OBJECT:
			{
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
				if(data_length < 11){
					get_sigils_we_have(SDL_SwapLE32(*((Uint32 *)(in_data+3))), 0);
				} else {
					get_sigils_we_have(SDL_SwapLE32(*((Uint32 *)(in_data+3))), SDL_SwapLE32(*((Uint32 *)(in_data+7))));
				}
			}
			break;

		case GET_ACTIVE_SPELL:
			{
				get_active_spell(in_data[3],in_data[4]);
			}
			break;

		case REMOVE_ACTIVE_SPELL:
			{
				remove_active_spell(in_data[3]);
			}
			break;

		case GET_ACTIVE_SPELL_LIST:
			{
				get_active_spell_list(&in_data[3]);
			}
			break;

		case GET_ACTOR_HEALTH:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				get_actor_health(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
			}
			break;

		case GET_ACTOR_DAMAGE:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				get_actor_damage(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
			}
			break;

		case GET_ACTOR_HEAL:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				get_actor_heal(SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16*)(in_data+5))));
			}
			break;

		case ACTOR_UNWEAR_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				unwear_item_from_actor(SDL_SwapLE16(*((Uint16 *)(in_data+3))),in_data[5]);
			}
			break;

		case ACTOR_WEAR_ITEM:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				actor_wear_item(SDL_SwapLE16(*((Uint16 *)(in_data+3))),in_data[5],in_data[6]);
			}
			break;

		case NPC_SAY_OVERTEXT:
			{
				add_displayed_text_to_actor(
					get_actor_ptr_from_id( SDL_SwapLE16(*((Uint16 *)(in_data+3))) ), in_data+5 );
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
				if(in_data[3]==1)
					add_buddy(&in_data[5],in_data[4],data_length-5);
				else if(in_data[3]==0)
					del_buddy(&in_data[4],data_length-4);
			}
			break;

		case DISPLAY_CLIENT_WINDOW:
			{
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
				open_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;

		case READ_BOOK:
			{
				read_network_book(in_data+3, data_length-3);
			}
			break;

		case CLOSE_BOOK:
			{
				close_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;
		case STORAGE_LIST:
			{
				get_storage_categories(in_data+3, data_length-3);
			}
			break;

		case STORAGE_ITEMS:
			{
				get_storage_items(in_data+3, data_length-3);
			}
			break;

		case STORAGE_TEXT:
			{
				get_storage_text(in_data+3, data_length-3);
			}
			break;
    		case SPELL_CAST:
    			{
				process_network_spell(in_data+3, data_length-3);
#ifdef COUNTERS
					if (in_data[3] == S_SUCCES) {
						// increment the spell counter
						increment_spell_counter(in_data[4]);
					}
#endif
        		}
			break;
		case GET_ACTIVE_CHANNELS:
			set_active_channels (in_data[3], (Uint32*)(in_data+4), (data_length-2)/4);
			break;

		case GET_3D_OBJ_LIST:
			if (data_length > 3)
				get_3d_objects_from_server (in_data[3], &in_data[4], data_length - 4);
			break;

		case GET_3D_OBJ:
			get_3d_objects_from_server (1, &in_data[3], data_length - 3);
			break;
		
		case REMOVE_3D_OBJ:
			if (data_length == 5)
				remove_3d_object_from_server (SDL_SwapLE16 (*((Uint16 *)(&in_data[3]))));
			break;

		// for use by 1.0.3 server and higher
		case MAP_SET_OBJECTS:
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
			{
				map_flags=SDL_SwapLE32(*((Uint32 *)(in_data+3)));
			}
			break;

		case GET_ITEMS_COOLDOWN: 
			{
				// make sure we interpret the incoming octets as unsigned
				// in case the function signature changes
				get_items_cooldown (&in_data[3], data_length - 3);
			}
			break;

		case SEND_BUFFS:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				update_actor_buffs(SDL_SwapLE16(*((short *)(in_data+3))), in_data[5]);
			}
			break;
			
		case SEND_SPECIAL_EFFECT:
			{
#ifdef SFX
				if (special_effects){
					parse_special_effect(in_data[3], (const Uint16 *) &in_data[4]);
				}
#endif
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


int in_data_used=0;
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
			if (received) {
				LOG_TO_CONSOLE(c_red2, SDLNet_GetError()); //XXX: SDL[Net]_GetError used by timer thread ? i bet its not reentrant...
			}
		 
			LOG_TO_CONSOLE(c_red2, disconnected_from_server);
			LOG_TO_CONSOLE(c_red2, alt_x_quit);
			in_data_used = 0;
			disconnected = 1;
		}
	}
	return 1;
}
