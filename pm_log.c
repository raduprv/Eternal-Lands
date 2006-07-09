#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "pm_log.h"

int afk=0;
int last_action_time=0;
int afk_time=DEFAULT_AFK_MINUTES*60000;
int afk_time_conf=DEFAULT_AFK_MINUTES; //For elconfig window
char afk_message[MAX_TEXT_MESSAGE_LENGTH]={0};
char afk_title[101];
#ifdef AFK_FIX
int afk_local = 0;
#endif //AFK_FIX

struct pm_struct pm_log;

void print_return_message(void);    /* forward declaration */

void free_pm_log()
{
	int i;

	for(;--pm_log.ppl>=0;)                                           
		{                                                       
			pm_log.afk_msgs[pm_log.ppl].msgs=0;
			free(pm_log.afk_msgs[pm_log.ppl].name); 
			pm_log.afk_msgs[pm_log.ppl].name=NULL;
			for(i=pm_log.afk_msgs[pm_log.ppl].msgs;--i>=0;)
				{
					free(pm_log.afk_msgs[pm_log.ppl].messages[i]); 
					pm_log.afk_msgs[pm_log.ppl].messages[i]=NULL;
				}
		}
	free(pm_log.afk_msgs);
	pm_log.afk_msgs=NULL;
	pm_log.msgs=pm_log.ppl=0;
}

void go_afk()
{
	if(pm_log.ppl)free_pm_log();
	LOG_TO_CONSOLE(c_green1,going_afk);
	if(!you_sit) 
		{
			Uint8 str[4];
			str[0]=SIT_DOWN;
			str[1]=1;
			my_tcp_send(my_socket,str,2);
		}
	afk++;
}

void go_ifk()
{
	print_return_message();
	afk=0;
}

void print_title(char * no, char * name, char * messages)
{
	char * ptr = afk_title;
	
	memset(afk_title,' ',100);
	while(*no)*ptr++=*no++;
	*ptr=':';
	ptr=afk_title+12;
	while(*name)*ptr++=*name++;
	*ptr=':';
	ptr=afk_title+27;
	while(*messages)*ptr++=*messages++;
	*ptr++=':';
	*ptr=0;
}

void print_return_message()
{
	char str[65];
	int m=-1;

	LOG_TO_CONSOLE(c_green1,not_afk);
	if(pm_log.ppl && pm_log.msgs)
		{
			snprintf(str, sizeof(str), new_messages, pm_log.msgs);
			LOG_TO_CONSOLE(c_green2,str);
			print_title("#", afk_names, afk_messages);
			LOG_TO_CONSOLE(c_green2, afk_title);
			while(++m<pm_log.ppl)
				{
					char name[35];
					snprintf(name, sizeof(name), "%2d: %16s         %2d",m+1,pm_log.afk_msgs[m].name,pm_log.afk_msgs[m].msgs);
					LOG_TO_CONSOLE(c_green2,name);
				}
			LOG_TO_CONSOLE(c_green2,afk_print_help);
		}
}

void print_message(int no)
{
	int m=-1;

	while(++m<pm_log.afk_msgs[no].msgs)
		LOG_TO_CONSOLE(c_blue1,pm_log.afk_msgs[no].messages[m]);
}

int have_name(char *name, int len)
{
	int z=0;

	for (;z<pm_log.ppl;z++){
		if(pm_log.afk_msgs[z].name && !strncasecmp(pm_log.afk_msgs[z].name, name, len)){
			return z;
		}
	}
	return -1;
}

int add_name_to_pm_log(char *name, int len)
{
	int z= pm_log.ppl;

	pm_log.ppl++;
	pm_log.afk_msgs= (afk_struct *)realloc(pm_log.afk_msgs,pm_log.ppl*sizeof(afk_struct));
	pm_log.afk_msgs[z].msgs= 0;
	pm_log.afk_msgs[z].messages= NULL;
	pm_log.afk_msgs[z].name= (char*)calloc(len+1, sizeof(char));
	snprintf(pm_log.afk_msgs[z].name, len+1, "%s", name);
	return z;
}

#ifndef AFK_FIX
void add_message_to_pm_log (const char *message, int len)
#else
void add_message_to_pm_log (char *message, int len, Uint8 channel)
#endif //AFK_FIX
{
	char buf[512];
#ifndef AFK_FIX
	int last_pm_len = strlen(last_pm_from);
	int z = have_name (last_pm_from, last_pm_len);
#else
	char last_msg_from[32];
	int last_msg_len;
	int z;
	char mymsg[512]; //, *msg_pointer;

	strcpy(mymsg, message);
	if (channel == CHAT_LOCAL) {
        char *msg_pointer;
        
		msg_pointer = strstr(mymsg, " ") - 1;
		*msg_pointer = 0;
		strncpy(last_msg_from, mymsg, sizeof(last_msg_from));
		strncpy(mymsg, msg_pointer+2, sizeof(mymsg));
	} else {
		//*mymsg = message;
		strncpy(last_msg_from, last_pm_from, sizeof(last_msg_from));
	}
	last_msg_len = strlen(last_msg_from);
	z = have_name (last_msg_from, last_msg_len);
#endif //AFK_FIX
	if (z < 0)
	{
#ifndef AFK_FIX
		send_afk_message (NULL, 0, CHAT_PERSONAL);
		z = add_name_to_pm_log (last_pm_from, last_pm_len);
#else
		if (channel == CHAT_LOCAL) {
			send_afk_message (message, strlen(message), channel);
		} else {
			send_afk_message (NULL, 0, channel);
		}
		z = add_name_to_pm_log (last_msg_from, last_msg_len);
#endif //AFK_FIX
	}
	
	pm_log.afk_msgs[z].messages = realloc (pm_log.afk_msgs[z].messages, (pm_log.afk_msgs[z].msgs+1) * sizeof (char *));
	// time name message
#ifndef AFK_FIX
	snprintf (buf, sizeof(buf), "<%1d:%02d> %s: %.*s", game_minute/60, game_minute%60, last_pm_from, len, message);
#else
	snprintf (buf, sizeof(buf), "<%1d:%02d> %s: %.*s", game_minute/60, game_minute%60, last_msg_from, strlen(mymsg), mymsg);
#endif //AFK_FIX
	pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs] = calloc (strlen (buf) + 1, sizeof (char));
	strcpy (pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs], buf);
	pm_log.afk_msgs[z].msgs++;
	pm_log.msgs++;
}

int my_namecmp(char *check)
{
	int i=0;
	char username[32];
	strcpy(username,username_str);
	my_tolower(username);
	
	for(;i<20 && username[i] && check[i]==username[i];i++);
	if(check[i]==username[i]||((check[i]==' '||!isalpha(check[i])) && !username[i])) return 0;
	return 1;
}

int is_talking_about_me (const Uint8 *server_msg, int len, char everywhere)
{
	int a=0;
	unsigned char msg[200];
	if(len > 198)
		return 0;
	if (!everywhere && (server_msg[0] == '[' || server_msg[0] == '#'))
	{
		return 0; //Only do local chat
	}

	snprintf (msg, sizeof(msg), "%.*s", len, server_msg);
	my_tolower (msg);

	while (msg[a] && msg[a] != ':' && (msg[a] < 127+c_red1 || msg[a] > 127+c_grey4))
		a++;
	//We do need the name of ourselves...
	while (a < 199 && msg[a] != '\0')
	{
		if((msg[a]==' '||(msg[a]>127+c_red1 && msg[a]<127+c_grey4)) && !my_namecmp(msg+1+a))
			return 1;
		else
			a++;
	}
	return 0;
}

void send_afk_message (const Uint8 *server_msg, int len, Uint8 channel)
{
	Uint8 sendtext[MAX_TEXT_MESSAGE_LENGTH]={0};
	
	if (afk_message[0] == '\0') return;
	
	if (channel == CHAT_PERSONAL || channel == CHAT_MODPM)
	{
		snprintf (sendtext, sizeof(sendtext), "%c%s %s", SEND_PM, last_pm_from, afk_message);
	}
	else 
	{
		int i, j;
		//char *name = (char *)calloc(32, sizeof (char));
		char	name[32];
		
		// Copy the name. This ought to work for both local chat and
		// trade attempts
		i = j = 0;
		while (j < len && (server_msg[j] < 127+c_red1 || server_msg[j] > 127+c_grey4) )
		{
			name[i++] = server_msg[j];
			if (server_msg[j] == ':' || server_msg[j] == ' ') break;
			j++;
		}		
		name[i-1] = '\0';
		
		if (have_name(name, i-1) < 0)
		{
			snprintf (sendtext, sizeof(sendtext), "%c%s %s", SEND_PM, name, afk_message);
#ifndef AFK_FIX
			add_name_to_pm_log (name, i-1);
#endif //AFK_FIX
		}
		else
		{
			sendtext[0]= '\0';
			sendtext[1]= '\0';
		}
	}
	if (sendtext[1] != '\0') 
	{
		my_tcp_send (my_socket, sendtext, strlen (&sendtext[1]) + 1);
	}
}
