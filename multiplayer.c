#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void get_updates();
 */

const char * web_update_address="http://www.eternal-lands.com/index.php?content=update";

int port=2000;
unsigned char server_address[60];
TCPsocket my_socket=0;
SDLNet_SocketSet set=0;
Uint8 in_data[8192];
int previously_logged_in=0;
Uint32 last_heart_beat;

char our_name[20];
char our_password[20];
int log_conn_data=0;

int this_version_is_invalid=0;
int put_new_data_offset=0;
Uint8	tcp_cache[256];
Uint32	tcp_cache_len=0;
Uint32	tcp_cache_time=0;

//for the client/server sync
int server_time_stamp=0;
int client_time_stamp=0;
int client_server_delta_time=0;

int yourself=-1;

int last_sit=0;

int my_tcp_send(TCPsocket my_socket, Uint8 *str, int len)
{
	int i;
	Uint8 new_str[1024];//should be enough

	if(disconnected)return 0;
	//check to see if we have too many packets being sent of the same to reduce server flood
	if(len < 256)	// only if it fits
	if(str[0]==MOVE_TO || str[0]==RUN_TO || str[0]==SIT_DOWN || str[0]==HARVEST || str[0]==MANUFACTURE_THIS || str[0]==CAST_SPELL || str[0]==RESPOND_TO_NPC || str[0]==ATTACK_SOMEONE || str[0]==SEND_PM || str[0]==RAW_TEXT)
		{
			Uint32	time_limit=600;
			if( str[0]==SEND_PM || str[0]==RAW_TEXT)time_limit=1500;
			if( str[0]==SIT_DOWN){
				if(last_sit+1500>cur_time) return 0;
				last_sit=cur_time;
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
			tcp_cache_len = len;
			tcp_cache_time = cur_time;
		}
	//update the heartbeat timer
	last_heart_beat=cur_time;

	new_str[0]=str[0];//copy the protocol
	*((short *)(new_str+1))= SDL_SwapLE16((Uint16)len);//the data lenght
	//copy the rest of the data
	for(i=1;i<len;i++)new_str[i+2]=str[i];
	return SDLNet_TCP_Send(my_socket,new_str,len+2);
}

void send_version_to_server(IPaddress *ip)
{
	Uint8 str[20];

	str[0]=SEND_VERSION;
	*((short *)(str+1))=SDL_SwapLE16((short)version_first_digit);
	*((short *)(str+3))=SDL_SwapLE16((short)version_second_digit);
	str[5]=client_version_major;
	str[6]=client_version_minor;
	str[7]=client_version_release;
	str[8]=client_version_patch;
	// XXX (Grum): is there no better way then swapping host and port twice?
	#ifdef EL_BIG_ENDIAN
	ip->host = SDL_Swap32(ip->host);
	ip->port = SDL_Swap16(ip->port);
	#endif
	str[9]=ip->host&0xFF;
	str[10]=(ip->host >> 8)&0xFF;
	str[11]=(ip->host >> 16)&0xFF;
	str[10]=(ip->host >> 24)&0xFF;
	str[13]=ip->port&0xFF;
	str[14]=(ip->port >> 8)&0xFF;	
	#ifdef EL_BIG_ENDIAN
	ip->host = SDL_Swap32(ip->host);
	ip->port = SDL_Swap16(ip->port);
	#endif
	
	my_tcp_send(my_socket,str,15);
}

void connect_to_server()
{
	IPaddress ip;
	if(this_version_is_invalid)return;
	if(set)
		{
			SDLNet_FreeSocketSet(set);
			set=0;
		}
	if(my_socket)
		{
			SDLNet_TCP_Close(my_socket);
			my_socket=0;
		}

	LOG_TO_CONSOLE(c_red1,connect_to_server_str);
	draw_scene();	// update the screen
	set=SDLNet_AllocSocketSet(1);
	if(!set)
        {
            char str[120];
            sprintf(str,"SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
            log_error(str);
			SDLNet_Quit();
			SDL_Quit();
			exit(4); //most of the time this is a major error, but do what you want.

        }

	if(SDLNet_ResolveHost(&ip,server_address,port)==-1)
		{
			LOG_TO_CONSOLE(c_red2,failed_resolve);
			return;
		}

	my_socket=SDLNet_TCP_Open(&ip);
	if(!my_socket)
		{
			LOG_TO_CONSOLE(c_red1,failed_connect);
			LOG_TO_CONSOLE(c_red1,reconnect_str);
			LOG_TO_CONSOLE(c_red1,alt_x_quit);
            		return;
		}

	if(SDLNet_TCP_AddSocket(set,my_socket)==-1)
		{
            char str[120];
            sprintf(str,"SDLNet_TCP_AddSocket: %s\n",SDLNet_GetError());
            log_error(str);
			SDLNet_Quit();
			SDL_Quit();
			exit(2);
		}
	disconnected=0;
	//ask for the opening screen
	if(!previously_logged_in)
		{
			Uint8 str[1];
			str[0]=SEND_OPENING_SCREEN;
			my_tcp_send(my_socket,str,1);
		}
	else
		{
			yourself=-1;
			you_sit=0;
			destroy_all_actors();
			send_login_info();
		}

    //send the current version to the server
    send_version_to_server(&ip);
    last_heart_beat=cur_time;
	hide_window(trade_win);
}

void send_login_info()
{
	int i;
	int j;
	int len;
	unsigned char str[40];

	len=strlen(username_str);
	//check for the username lenght
	if(len<3)
		{
			sprintf(log_in_error_str,"%s: %s",reg_error_str,error_username_length);
			return;
		}

	//join the username and password, and send them to the server
	str[0]=LOG_IN;

	if(caps_filter && my_isupper(username_str, len)) my_tolower(username_str);
	for(i=0;i<len;i++)str[i+1]=username_str[i];
	str[i+1]=' ';
	i++;
	len=strlen(password_str);
	for(j=0;j<len;j++)str[i+j+1]=password_str[j];
	str[i+j+1]=0;

	//are we going to use this?
	//clear the password field, in order not to try to relogin again
	//password_str[0]=0;
	//display_password_str[0]=0;
	//password_text_lenght=0;


	len=strlen(str);
	len++;//send the last 0 too
	if(my_tcp_send(my_socket,str,len)<len)
		{
			//we got a nasty error, log it
		}
}


void send_new_char(Uint8 * user_str, Uint8 * pass_str, Uint8 * conf_pass_str, char skin,
				   char hair, char shirt, char pants, char boots,char head, char type)
{
	int i;
	int j;
	int len;
	unsigned char str[120];

	//join the user and pass, and send them to the server
	len=strlen(user_str);

	//check for the username lenght
	if(len<3)
		{
			sprintf(create_char_error_str,"%s: %s",reg_error_str,error_username_length);
			return;
		}
	//check if the password is >0
	if(strlen(pass_str)<4)
		{
			sprintf(create_char_error_str,"%s: %s",reg_error_str,error_password_length);
			return;
		}
	//check if the password coresponds
	if(strcmp(pass_str,conf_pass_str)!=0)
		{
			sprintf(create_char_error_str,"%s: %s",reg_error_str,error_pass_no_match);
			return;
		}

	str[0]=CREATE_CHAR;
	for(i=0;i<len;i++)str[i+1]=user_str[i];
	str[i+1]=' ';
	i++;
	len=strlen(pass_str);
	for(j=0;j<len;j++)str[i+j+1]=pass_str[j];
	str[i+j+1]=0;

	//put the colors and gender
	str[i+j+2]=skin;
	str[i+j+3]=hair;
	str[i+j+4]=shirt;
	str[i+j+5]=pants;
	str[i+j+6]=boots;
	str[i+j+7]=type;
	str[i+j+8]=head;


	len=i+j+9;
	if(my_tcp_send(my_socket,str,len)<len)
		{
			//we got a nasty error, log it
		}
	create_char_error_str[0]=0;//no error
}

// TEMP LOGAND [5/25/2004]
#ifndef NPC_SAY_OVERTEXT
#define NPC_SAY_OVERTEXT 58 
#endif
//---

void process_message_from_server(unsigned char *in_data, int data_lenght)
{
	//see what kind of data we got
	switch (in_data[PROTOCOL])
		{
		case RAW_TEXT:
			{
#ifdef MULTI_CHANNEL
				Uint32 channel;
				// extract the channel number
				if (data_lenght > 7) 
				{
					channel =  SDL_SwapLE32(*((Uint32*)(&in_data[3])));
					data_lenght = filter_or_ignore_text (&in_data[7], data_lenght-7) + 7;
					if (data_lenght > 7)
					{
						//how to display it
						if (get_show_window (opening_root_win) )
							put_text_in_buffer (&in_data[7], data_lenght-7, 54);
						else 
							put_text_in_buffer (&in_data[7], data_lenght - 7, 0);
						// let's log it
						write_to_log (&in_data[7], data_lenght - 7);
					}
				}
#else			
				// do filtering and ignoring
				data_lenght=filter_or_ignore_text(&in_data[3],data_lenght-3)+3;
				if(data_lenght > 3)
					{
						//how to display it
						if (get_show_window (opening_root_win) )
							put_text_in_buffer(&in_data[3],data_lenght-3,54);
						else put_text_in_buffer(&in_data[3],data_lenght-3,0);
						//lets log it
						write_to_log(&in_data[3],data_lenght-3);
					}
#endif
			}
			break;

		case ADD_NEW_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_actor_from_server(&in_data[3]);
			}
			break;

		case ADD_NEW_ENHANCED_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_enhanced_actor_from_server(&in_data[3]);
			}
			break;

		case ADD_ACTOR_COMMAND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_command_to_actor(SDL_SwapLE16(*((short *)(in_data+3))),in_data[5]);
			}
			break;

		case REMOVE_ACTOR:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				destroy_actor(SDL_SwapLE16(*((short *)(in_data+3))));
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
				game_minute=SDL_SwapLE16(*((short *)(in_data+3)));
				new_minute();
			}
			break;

		case LOG_IN_OK:
			{
				show_hud_windows ();
				// login and/or new character windows are no longer needed
				if (login_root_win >= 0) destroy_window (login_root_win);
				login_root_win = -1;
				if (newchar_root_win >= 0) destroy_window (newchar_root_win);
				newchar_root_win = -1;
				show_window (game_root_win);

				previously_logged_in=1;
			}
			break;

		case HERE_YOUR_STATS:
			{
				get_the_stats((Sint16 *)(in_data+3));
			}
			break;

		case SEND_PARTIAL_STAT:
			{
				get_partial_stat(*((Uint8 *)(in_data+3)),SDL_SwapLE32(*((Sint32 *)(in_data+4))));
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
				get_new_inventory_item(in_data+3);
			}
			break;

		case REMOVE_ITEM_FROM_INVENTORY:
			{
				remove_item_from_inventory(*((Uint8 *)(in_data+3)));
			}
			break;

		case INVENTORY_ITEM_TEXT:
			{
				put_small_text_in_box(&in_data[3],data_lenght-3,6*51+100,items_string);
				if(!(get_show_window(items_win)||get_show_window(manufacture_win)||get_show_window(trade_win)))
					{
						put_text_in_buffer(&in_data[3],data_lenght-3,0);
					}
			}
			break;

		case SPELL_ITEM_TEXT:
			{
				put_small_text_in_box(&in_data[3],data_lenght-3,6*51+100,spell_text);
				have_error_message=1;
			}
			break;

		case GET_KNOWLEDGE_TEXT:
			{
				put_small_text_in_box(&in_data[3],data_lenght-3,6*51+150,knowledge_string);
			}
			break;

		case CHANGE_MAP:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				object_under_mouse=-1;//to prevent a nasty crash, while looking for bags, when we change the map
				close_dialogue();	// close the dialogue window if open
				destroy_all_particles();
				load_map(&in_data[3]);
				kill_local_sounds();
#ifndef	NO_MUSIC
				playing_music=0;
#endif	//NO_MUSIC
				get_map_playlist();
				have_a_map=1;
				//also, stop the rain
				seconds_till_rain_starts=-1;
				seconds_till_rain_stops=-1;
				is_raining=0;
				rain_sound=0;//kill local sounds also kills the rain sound
				weather_light_offset=0;
				rain_light_offset=0;
				if ( get_show_window (map_root_win) )
					switch_from_game_map ();
				{ 
					FILE * fp;
					char marks_file[256], text[600];
#ifndef WINDOWS
    				strcpy(marks_file, getenv("HOME"));
    				strcat(marks_file, "/.elc/");
					strcat(marks_file, strrchr(map_file_name,'/')+1);
#else
					strcpy(marks_file, strrchr(map_file_name,'/')+1);
#endif
					strcat(marks_file, ".txt");
					// don't use my_fopen here, not everyone uses map markers
					fp = fopen(marks_file, "r");
					max_mark = 0;
					if ( fp )
					{
						
						while ( fgets(text, 600,fp) )
						{
							if (strlen (text) > 1) //skip empty lines
							{
								sscanf (text, "%d %d", &marks[max_mark].x, &marks[max_mark].y);
								text[strlen(text)-1] = '\0'; //remove the newline
								strncpy (marks[max_mark].text, strstr(strstr(text, " ")+1, " ")+1, 500);
								max_mark++;
								if ( max_mark > 200 ) break;
							}
						}
						fclose(fp);
					}
				}

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
				if(!no_sound)play_music(SDL_SwapLE16(*((short *)(in_data+3))));
			}
			break;

		case PLAY_SOUND:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				if(!no_sound)add_sound_object(SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))),SDL_SwapLE16(*((short *)(in_data+7))),SDL_SwapLE16(*((char *)(in_data+9))),SDL_SwapLE16(*((short *)(in_data+10))));
			}
			break;

		case TELEPORT_OUT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_particle_sys_at_tile("./particles/teleport_in.part",SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))));
				if(!no_sound)add_sound_object(snd_tele_out,SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))),1,0);
			}
			break;

		case TELEPORT_IN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				add_particle_sys_at_tile("./particles/teleport_in.part",SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))));
				if(!no_sound)add_sound_object(snd_tele_out,SDL_SwapLE16(*((short *)(in_data+3))),SDL_SwapLE16(*((short *)(in_data+5))),1,0);
			}
			break;

		case LOG_IN_NOT_OK:
			{
				set_login_error (&in_data[3], data_lenght - 3);
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
				set_create_char_error (&in_data[3], data_lenght - 3);
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
				yourself=SDL_SwapLE16(*((short *)(in_data+3)));
			}
			break;

		case START_RAIN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				seconds_till_rain_starts=*((Uint8 *)(in_data+3));
				seconds_till_rain_stops=-1;
			}
			break;

		case STOP_RAIN:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				seconds_till_rain_stops=*((Uint8 *)(in_data+3));
				seconds_till_rain_starts=-1;
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
				server_time_stamp=SDL_SwapLE32(*((int *)(in_data+3)));
				client_time_stamp=SDL_GetTicks();
				client_server_delta_time=server_time_stamp-client_time_stamp;
			}
			break;

		case PONG:
			{
				Uint8 str[160];
				sprintf(str,"%s: %i MS",server_latency, SDL_GetTicks()-SDL_SwapLE32(*((Uint32 *)(in_data+3))));
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
	             add_particle_sys_at_tile("./particles/bag_in.part",SDL_SwapLE16(*((Uint16 *)(in_data+3))),SDL_SwapLE16(*((Uint16 *)(in_data+5))));
			}
			break;

		case FIRE_PARTICLES:
			{
#ifdef EXTRA_DEBUG
				ERR();
#endif
				switch(SDL_SwapLE16(*(Uint16 *)(in_data+7))) {
				case 2:
					add_particle_sys("./particles/fire_big.part",(float)(SDL_SwapLE16(*((Uint16 *)(in_data+3))))/2.0 +0.25,(float)(SDL_SwapLE16(*((Uint16 *)(in_data+5))))/2.0 + 0.25,0.0);
					break;
				case 1:
				default:
					add_particle_sys("./particles/fire_small.part",(float)(SDL_SwapLE16(*((Uint16 *)(in_data+3))))/2.0 +0.25,(float)(SDL_SwapLE16(*((Uint16 *)(in_data+5))))/2.0 + 0.25,0.0);
				}
			}
			break;

		case REMOVE_FIRE_AT:
			{
#ifdef EXTRA_DEBUG
	ERR();
#endif
				remove_fire_at((float)(SDL_SwapLE16(*((Uint16 *)(in_data+3))))/2.0 +0.25,(float)(SDL_SwapLE16(*((Uint16 *)(in_data+5))))/2.0 + 0.25);
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
				put_small_text_in_box(&in_data[3],data_lenght-3,dialogue_menu_x_len-70,dialogue_string);
				display_dialogue();
				if(in_data[3]>=127 && in_data[4]>=127)
					{
						add_questlog(&in_data[4],data_lenght-4);
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
				build_response_entries(&in_data[3],*((Uint16 *)(in_data+1)));
			}
			break;

		case GET_TRADE_ACCEPT:
			{
				if(!in_data[3])trade_you_accepted=1;
				else
					trade_other_accepted=1;
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
				get_sigils_we_have(SDL_SwapLE32(*((Uint32 *)(in_data+3))));
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
				// just send the pack back as it is
				my_tcp_send(my_socket,in_data,data_lenght);
			}
			break;

		case BUDDY_EVENT:
			{
				if(in_data[3]==1)
					add_buddy(&in_data[5],in_data[4],data_lenght-3);
				else if(in_data[3]==0)
					del_buddy(&in_data[4],data_lenght-2);
			}
			break;
			
		case DISPLAY_CLIENT_WINDOW:
			{
				switch(in_data[3]){
					case RULE_WIN: 
					case RULE_INTERFACE: 
						highlight_rule(in_data[3],in_data+4,data_lenght-4);
						break;
					case NEW_CHAR_INTERFACE:
						hide_all_root_windows ();
						hide_hud_windows ();
						create_newchar_root_window ();
						show_window (newchar_root_win);
						connect_to_server();
						break;
					default:
						break;
				}
			}

		case OPEN_BOOK:
			{
				open_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;

		case READ_BOOK:
			{
				read_network_book(in_data+3, data_lenght-3);
			}
			break;

		case CLOSE_BOOK:
			{
				close_book(SDL_SwapLE16(*((Uint16*)(in_data+3))));
			}
			break;

		default:
			{
				// Unknown data type??
                ;
			}
			break;
		}
}


int in_data_used=0;
static void process_data_from_server()
{
	/* enough data present for the length field ? */
	if (3 <= in_data_used) {
		Uint8   *pData  = in_data;
		Uint16   size;
		
		do { /* while (3 <= in_data_used) (enough data present for the length field) */
			size = SDL_SwapLE16(*((short*)(pData+1)));
			
			if (sizeof (in_data) - 3 >= size) { /* buffer big enough ? */
				size += 2; /* add length field size */
				
				if (size <= in_data_used) { /* do we have a complete message ? */
					process_message_from_server(pData, size);

					if (log_conn_data)
						log_conn(pData, size);
		
					/* advance to next message */
					pData         += size;
					in_data_used  -= size;
				}
				else
					break;
			}
			else { /* sizeof (in_data) - 3 < size */
				LOG_TO_CONSOLE(c_red2, packet_overrun);
	    
				LOG_TO_CONSOLE(c_red2, disconnected_from_server);
				LOG_TO_CONSOLE(c_red2, alt_x_quit);
				in_data_used = 0;
				disconnected = 1;
			}
		} while (3 <= in_data_used);

		/* move the remaining data to the start of the buffer...(not tested...never happened to me) */
		if (in_data_used && pData != in_data)
			memmove(in_data, pData, in_data_used);
	}
}

void get_message_from_server()
{
	/* data available for reading ? */
	if (!disconnected && SDLNet_CheckSockets(set, 0) && SDLNet_SocketReady(my_socket)) {
		int received;

		if (0 < (received = SDLNet_TCP_Recv(my_socket, &in_data[in_data_used], sizeof (in_data) - in_data_used))) {
			in_data_used += received;
      
			process_data_from_server();
		}
		else { /* 0 >= received (EOF or some error) */
			if (received)
				LOG_TO_CONSOLE(c_red2, SDLNet_GetError()); //XXX: SDL[Net]_GetError used by timer thread ? i bet its not reentrant...
		 
			LOG_TO_CONSOLE(c_red2, disconnected_from_server);
			LOG_TO_CONSOLE(c_red2, alt_x_quit);
			in_data_used = 0;
			disconnected = 1;
		}
	}
}

/* currently UNUSED
void get_updates()
{
	char servername[80];
	char filepath_on_server[80];
	char local_filepath[80];
	FILE *fp;
	strcpy(servername, "no-exit.org");
	strcpy(filepath_on_server, "/el/files/testfile");
	strcpy(local_filepath, "testfile");
	fp = my_fopen(local_filepath, "w");
	http_get_file(servername, filepath_on_server, fp);
	fclose(fp);
}
*/
