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
			snprintf(str, 60, new_messages, pm_log.msgs);
			LOG_TO_CONSOLE(c_green2,str);
			print_title("#", afk_names, afk_messages);
			LOG_TO_CONSOLE(c_green2, afk_title);
			while(++m<pm_log.ppl)
				{
					char name[35];
					sprintf(name,"%2d: %16s         %2d",m+1,pm_log.afk_msgs[m].name,pm_log.afk_msgs[m].msgs);
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

void add_name_to_pm_log(char *name, int len)
{
	int z= pm_log.ppl;

	pm_log.ppl++;
	pm_log.afk_msgs= (afk_struct *)realloc(pm_log.afk_msgs,pm_log.ppl*sizeof(afk_struct));
	pm_log.afk_msgs[z].msgs= 0;
	pm_log.afk_msgs[z].messages= NULL;
	pm_log.afk_msgs[z].name= (char*)calloc(len+1, sizeof(char));
	memcpy(pm_log.afk_msgs[z].name,name,len+1);
}

void add_message_to_pm_log(char * message, int len)
{
	int last_pm_len= strlen(last_pm_from);
	int z= have_name(last_pm_from, last_pm_len);
	char	buf[512];

	if(z<0)
		{
			send_afk_message(NULL, 1);
			z= pm_log.ppl;
			add_name_to_pm_log(last_pm_from, last_pm_len);
		}
	pm_log.afk_msgs[z].messages= (char**)realloc(pm_log.afk_msgs[z].messages, (pm_log.afk_msgs[z].msgs+1)*sizeof(char *));
	//time name message
	snprintf(buf, 500, "<%1d:%02d> %s: %.*s", game_minute/60, game_minute%60, last_pm_from, len, message);
	pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs]= (char*)calloc(strlen(buf+1), sizeof(char));
	strcpy(pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs], buf);
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

int is_talking_about_me (Uint8 *server_msg, int len, char everywhere)
{
	int a=0;
	unsigned char msg[200];
	if(len > 198)
		return 0;
	if (!everywhere && (*server_msg == '[' || *server_msg == '#'))
	{
		return 0; //Only do local chat
	}

	strncpy(msg,server_msg,len);
	msg[len]=0;
	my_tolower(msg);

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

void send_afk_message (Uint8 *server_msg, int type)
{
	Uint8 sendtext[MAX_TEXT_MESSAGE_LENGTH+60]={0};
	
	if (afk_message[0] == '\0') return;
	
	if (type)
	{
		sprintf (sendtext, "%c%s %s", SEND_PM, last_pm_from, afk_message);
	}
	else 
	{
		int i=0;
		//char *name = (char *)calloc(32, sizeof (char));
		char	name[32];
		
		// Copy the name. This ought to work for both local chat and
		// trade attempts
		while (*server_msg < 127+c_red1 || *server_msg > 127+c_grey4)
		{
			name[i++] = *server_msg;
			if (*server_msg == ':' || *server_msg == ' ') break;
			server_msg++;
		}		
		name[i-1] = '\0';
		
		if (have_name(name, i-1) < 0)
		{
			sprintf (sendtext, "%c%s %s", SEND_PM, name, afk_message);
			add_name_to_pm_log (name, i-1);
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
