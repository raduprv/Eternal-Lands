#include <stdio.h>
#include <string.h>
#include "global.h"
#include "pm_log.h"

int afk=0;
int last_action_time=0;
int afk_time=0;
char afk_message[160]={0};
char afk_title[100];

struct pm_struct pm_log;

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
	log_to_console(c_green1,going_afk);
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

	log_to_console(c_green1,not_afk);
	if(pm_log.ppl && pm_log.msgs)
		{
			snprintf(str,60,new_messages,pm_log.msgs);
			log_to_console(c_green2,str);
			print_title("#",afk_names,afk_messages);
			log_to_console(c_green2,afk_title);
			while(++m<pm_log.ppl)
				{
					char name[35];
					sprintf(name,"%2d: %16s         %2d",m+1,pm_log.afk_msgs[m].name,pm_log.afk_msgs[m].msgs);
					log_to_console(c_green2,name);
				}
			log_to_console(c_green2,afk_print_help);
		}
}

void print_message(int no)
{
	int m=-1;

	while(++m<pm_log.afk_msgs[no].msgs)
		log_to_console(c_blue1,pm_log.afk_msgs[no].messages[m]);
}

int have_name(char *name, int len)
{
	int z=0;

	for (;z<pm_log.ppl;z++) if(!strncasecmp(pm_log.afk_msgs[z].name,name,len)) return z;
	return -1;
}
void add_name_to_pm_log(char *name, int len)
{
	int z=pm_log.ppl;

	pm_log.ppl++;
	pm_log.afk_msgs=(afk_struct *)realloc(pm_log.afk_msgs,pm_log.ppl*sizeof(afk_struct));
	pm_log.afk_msgs[z].msgs=0;
	pm_log.afk_msgs[z].messages=NULL;
	pm_log.afk_msgs[z].name=(char*)calloc(len+1,sizeof(char));
	memcpy(pm_log.afk_msgs[z].name,name,len);
}

void add_message_to_pm_log(char * message, int len)
{
	int l, last_pm_len=strlen(last_pm_from);
	int z=have_name(last_pm_from,last_pm_len);

	message[len]=0;
	message+=11+last_pm_len;
	l=strlen(message)-1;
	if(z<0)
		{
			send_afk_message(NULL,1);
			z=pm_log.ppl;
			add_name_to_pm_log(last_pm_from,last_pm_len);
		}
	pm_log.afk_msgs[z].messages=(char**)realloc(pm_log.afk_msgs[z].messages,(pm_log.afk_msgs[z].msgs+1)*sizeof(char *));
	//time name message
	pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs]=(char*)calloc(184,sizeof(char));
	sprintf(pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs],"<%1d:%02d> %s: ",game_minute/60, game_minute%60,last_pm_from);
	strncat(pm_log.afk_msgs[z].messages[pm_log.afk_msgs[z].msgs],message,l);
	pm_log.afk_msgs[z].msgs++;
	pm_log.msgs++;
}

int is_talking_about_me(Uint8 *server_msg, int len)
{
	int a=0,l=strlen(username_str);

	if(*server_msg=='['||*server_msg=='#') return 0;//Only do local chat
	while(*server_msg++!=':' && --len); 
	server_msg+=2; len-=2;//We don't need the name of the actor...
	//We do need the name of ourselves...
	while((strncasecmp(server_msg+a,username_str,l))!=0) 
		if((len-a++)<=l)return 0;//We're at the end
	return 1;
}

void send_afk_message(char * server_msg, int type)
{
	Uint8 sendtext[160]={0};
	if(!afk_message[0]) return;
	if(type) sprintf(sendtext,"%c%s %s",2,last_pm_from,afk_message);
	else 
		{
			int i=0;
			char * name=(char*)calloc(20,sizeof(char));
			
			while((*(name+i++)=*server_msg++)!=':');
			*(name+i-1)=0;
			if(have_name(name,i-1)<0)
				{
					sprintf(sendtext,"%c%s %s",2,name,afk_message);
			
					add_name_to_pm_log(name, i-1);
				}
		}
	if(sendtext[1])my_tcp_send(my_socket,sendtext,strlen(&sendtext[1])+1);
}

