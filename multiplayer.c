#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

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
	*((short *)(new_str+1))=(Uint16)len;//the data lenght
	//copy the rest of the data
	for(i=1;i<len;i++)new_str[i+2]=str[i];
	return SDLNet_TCP_Send(my_socket,new_str,len+2);
}

void send_version_to_server(IPaddress *ip)
{
	Uint8 str[20];

	str[0]=SEND_VERSION;
	*((short *)(str+1))=(short)version_first_digit;
	*((short *)(str+3))=(short)version_second_digit;
	str[5]=client_version_major;
	str[6]=client_version_minor;
	str[7]=client_version_release;
	str[8]=client_version_patch;
	str[9]=ip->host&0xFF;
	str[10]=(ip->host >> 8)&0xFF;
	str[11]=(ip->host >> 16)&0xFF;
	str[10]=(ip->host >> 24)&0xFF;
	str[13]=ip->port&0xFF;
	str[14]=(ip->port >> 8)&0xFF;
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

	log_to_console(c_red1,connect_to_server_str);
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
			log_to_console(c_red2,failed_resolve);
			return;
		}

	my_socket=SDLNet_TCP_Open(&ip);
	if(!my_socket)
		{
			//check to see if the player is a moron...
			if(server_address[0]=='1' && server_address[1]=='9' && server_address[2]=='2'
			   && server_address[3]=='.' && server_address[4]=='1' && server_address[5]=='6'
			   && server_address[6]=='8')
			  	{
			   		log_to_console(c_red1,license_check);
					log_to_console(c_red1,alt_x_quit);
				}
			else
				{
					log_to_console(c_red1,failed_connect);
					log_to_console(c_red1,reconnect_str);
					log_to_console(c_red1,alt_x_quit);
				}
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
				// do filtering and ignoring
				data_lenght=filter_or_ignore_text(&in_data[3],data_lenght-3)+3;
				if(data_lenght > 3)
					{
						//how to display it
						if(interface_mode!=interface_opening)
							put_text_in_buffer(&in_data[3],data_lenght-3,0);
						else put_text_in_buffer(&in_data[3],data_lenght-3,54);
						//lets log it
						write_to_log(&in_data[3],data_lenght-3);
					}
			}
			break;

		case ADD_NEW_ACTOR:
			{
				add_actor_from_server(&in_data[3]);
			}
			break;

		case ADD_NEW_ENHANCED_ACTOR:
			{
				add_enhanced_actor_from_server(&in_data[3]);
			}
			break;

		case ADD_ACTOR_COMMAND:
			{
				add_command_to_actor(*((short *)(in_data+3)),in_data[5]);
			}
			break;

		case REMOVE_ACTOR:
			{
				lock_actors_lists();	//lock it to avoid timing issues
				destroy_actor(*((short *)(in_data+3)));
				unlock_actors_lists();	//unlock it
			}
			break;

		case KILL_ALL_ACTORS:
			{
				destroy_all_actors();
			}
			break;

		case NEW_MINUTE:
			{
				game_minute=*((short *)(in_data+3));
				new_minute();
			}
			break;

		case LOG_IN_OK:
			{
				interface_mode=interface_game;
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
				get_partial_stat(*((Uint8 *)(in_data+3)),*((Sint32 *)(in_data+4)));
			}
			break;

		case GET_KNOWLEDGE_LIST:
			{
				get_knowledge_list(*(Uint16 *)(in_data+1)-1, in_data+3);
			}
			break;

		case GET_NEW_KNOWLEDGE:
			{
				get_new_knowledge(*(Uint16 *)(in_data+3));
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
			}
			break;

		case GET_TELEPORTERS_LIST:
			{
				add_teleporters_from_list(&in_data[3]);
			}
			break;

		case PLAY_MUSIC:
			{
				if(!no_sound)play_music(*((short *)(in_data+3)));
			}
			break;

		case PLAY_SOUND:
			{
				if(!no_sound)add_sound_object(*((short *)(in_data+3)),*((short *)(in_data+5)),*((short *)(in_data+7)),*((char *)(in_data+9)),*((short *)(in_data+10)));
			}
			break;

		case TELEPORT_OUT:
			{
				add_particle_sys_at_tile("./particles/teleport_in.part",*((short *)(in_data+3)),*((short *)(in_data+5)));
				if(!no_sound)add_sound_object(snd_tele_out,*((short *)(in_data+3)),*((short *)(in_data+5)),1,0);
			}
			break;

		case TELEPORT_IN:
			{
				add_particle_sys_at_tile("./particles/teleport_in.part",*((short *)(in_data+3)),*((short *)(in_data+5)));
				if(!no_sound)add_sound_object(snd_tele_in,*((short *)(in_data+3)),*((short *)(in_data+5)),1,0);
			}
			break;

		case LOG_IN_NOT_OK:
			{
				sprintf(log_in_error_str,"%s: %s",reg_error_str,invalid_pass);
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
				sprintf(create_char_error_str,"%s: %s",reg_error_str,char_name_in_use);
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
				yourself=*((short *)(in_data+3));
			}
			break;

		case START_RAIN:
			{
				seconds_till_rain_starts=*((Uint8 *)(in_data+3));
				seconds_till_rain_stops=-1;
			}
			break;

		case STOP_RAIN:
			{
				seconds_till_rain_stops=*((Uint8 *)(in_data+3));
				seconds_till_rain_starts=-1;
			}
			break;

		case THUNDER:
			{
				add_thunder(rand()%5,*((Uint8 *)(in_data+3)));
			}
			break;


		case SYNC_CLOCK:
			{
				server_time_stamp=*((int *)(in_data+3));
				client_time_stamp=SDL_GetTicks();
				client_server_delta_time=server_time_stamp-client_time_stamp;
			}
			break;

		case PONG:
			{
				Uint8 str[160];
				sprintf(str,"%s: %i MS",server_latency, SDL_GetTicks()-*((Uint32 *)(in_data+3)));
				log_to_console(c_green1,str);
			}
			break;

		case UPGRADE_NEW_VERSION:
			{
				log_to_console(c_red1,update_your_client);
				log_to_console(c_red1,(char*)web_update_address);
			}
			break;

		case UPGRADE_TOO_OLD:
			{
				log_to_console(c_red1,client_ver_not_supported);
				log_to_console(c_red1,(char*)web_update_address);
				this_version_is_invalid=1;
			}
			break;

		case GET_NEW_BAG:
			{
				put_bag_on_ground(*((Uint16 *)(in_data+3)),*((Uint16 *)(in_data+5)),*((Uint8 *)(in_data+7)));
			}
			break;

		case GET_BAGS_LIST:
			{
				add_bags_from_list(&in_data[3]);
			}
			break;

		case SPAWN_BAG_PARTICLES:
			{
			  add_particle_sys_at_tile("./particles/bag_in.part",*((Uint16 *)(in_data+3)),*((Uint16 *)(in_data+5)));
			}
			break;

		case GET_NEW_GROUND_ITEM:
			{
				get_bag_item(in_data+3);
			}
			break;

		case HERE_YOUR_GROUND_ITEMS:
			{
				get_bags_items_list(&in_data[3]);
			}
			break;

		case CLOSE_BAG:
			{
				hide_window(ground_items_win);
			}
			break;

		case REMOVE_ITEM_FROM_GROUND:
			{
				remove_item_from_ground(in_data[3]);
			}
			break;

		case DESTROY_BAG:
			{
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
				get_trade_partner_name(&in_data[3],*((Uint16 *)(in_data+1))-1);
			}
			break;

		case GET_YOUR_SIGILS:
			{
				get_sigils_we_have(*((Uint32 *)(in_data+3)));
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
				get_actor_damage(*((Uint16 *)(in_data+3)),in_data[5]);
			}
			break;

		case GET_ACTOR_HEAL:
			{
				get_actor_heal(*((Uint16 *)(in_data+3)),in_data[5]);
			}
			break;

		case ACTOR_UNWEAR_ITEM:
			{
				unwear_item_from_actor(*((Uint16 *)(in_data+3)),in_data[5]);
			}
			break;

		case ACTOR_WEAR_ITEM:
			{
				actor_wear_item(*((Uint16 *)(in_data+3)),in_data[5],in_data[6]);
			}
			break;

		case NPC_SAY_OVERTEXT:
			{
				add_displayed_text_to_actor(
					get_actor_ptr_from_id( *((Uint16 *)(in_data+3)) ), in_data+5 );
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

		default:
			{
				/* Unknown data type?? */;
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
			static const int foo = 1; /* used for run-time byteorder check */
			
			/* make a copy of the length field...watch alignment/byteorder */
			if (*(char *)&foo) { /* little-endian ? */
				((Uint8 *)&size)[0] = pData[1];
				((Uint8 *)&size)[1] = pData[2];
			}
			else { /* big-endian */
				((Uint8 *)&size)[0] = pData[2];
				((Uint8 *)&size)[1] = pData[1];
			}
			
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
				log_to_console(c_red2, packet_overrun);
	    
				log_to_console(c_red2, disconnected_from_server);
				log_to_console(c_red2, alt_x_quit);
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
				log_to_console(c_red2, SDLNet_GetError()); //XXX: SDL[Net]_GetError used by timer thread ? i bet its not reentrant...
		 
			log_to_console(c_red2, disconnected_from_server);
			log_to_console(c_red2, alt_x_quit);
			in_data_used = 0;
			disconnected = 1;
		}
	}
}

void get_updates()
{
	char servername[80];
	char filepath_on_server[80];
	char local_filepath[80];
	FILE *fp;
	strcpy(servername, "no-exit.org");
	strcpy(filepath_on_server, "/el/files/testfile");
	strcpy(local_filepath, "testfile");
	fp = fopen(local_filepath, "w");
	http_get_file(servername, filepath_on_server, fp);
	fclose(fp);
}
