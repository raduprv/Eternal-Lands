#include <string.h>
#include "global.h"

void build_help()
{
	int i=0;

	my_strcp(help_list[i].topic_name,"main");
	my_strcp(help_list[i].topic_info,"\nThere are the following topics available: [console] [keys]. Type: \"help topic\", for more information about that topic. Note that you can abbreviate the topic name, as long as it is not ambiguous.");
	i++;

	my_strcp(help_list[i].topic_name,"topic");
	my_strcp(help_list[i].topic_info,"\nWhen i said \"help [topic]\", I actually meant to type help followed by one of the colored words...");
	i++;

	my_strcp(help_list[i].topic_name,"keys");
	my_strcp(help_list[i].topic_info,"\n[keys] - The following keys are available in the game:\nAlt+x to [exit] the game.\nLeft/Right arrows to rotate the camera around.\nShift + Left/Right arrows to finely rotate the camera.\nPageUp/PageDown to zoom in and out\nIns/Del to rotate your character in place (works only while you don't move, and you stand).\nF2 to go to the last heard URL (works only if you configured the browser properly, in el.ini).\nHome to move one step forward\nEsc to clear the text line you are writing\nTab to bring on the map.\nAlt+h to toggle health bars.\nAlt+n to toggle names.\nCtrl+a to open stAts window.\nCtrl+i to open Inventrory window.\nCtrl+l to do Look action.\nCtrl+m to open Manufacture window.\nCtrl+o to open Options window.\nCtrl+u to do Use action.\nCtrl+s to open Spell window.\nCtrl+w to do Walk action.\n~ or F1 to switch to/from [console] mode.");
	i++;

	my_strcp(help_list[i].topic_name,"console");
	my_strcp(help_list[i].topic_info,"\n[console] - In the console mode you can access the following commands: [help] [ver] [ignore] [unignore] [ignores] [filter] [unfilter] [filters] [glinfo] [time] [date] [stats] [cls] [quit] [exit] [ping]. For more information on those commands, type: \"help command\", where command is one of the listed commands (without the quote marks). To enter or leave the console mode, you can press either F1, or the ~ key, as long as you are using an English keyboard. Anything that is not a valid command will be sent as a normal chat line.");
	i++;

	my_strcp(help_list[i].topic_name,"ping");
	my_strcp(help_list[i].topic_info,"\n[ping] - Prints the latency you have with the server. Please note that the latency is given in the client/server terms, not actual machine ping terms, so, generally, the ping command latency is bigger than if you would actually ping the machine.");
	i++;

	my_strcp(help_list[i].topic_name,"help");
	my_strcp(help_list[i].topic_info,"\n[help] - Displays a help menu (duh!).");
	i++;

	my_strcp(help_list[i].topic_name,"ver");
	my_strcp(help_list[i].topic_info,"\n[ver] - Displays some info about the current version of the client.");
	i++;

	my_strcp(help_list[i].topic_name,"glinfo");
	my_strcp(help_list[i].topic_info,"\n[glinfo] - Displays some info about your video card name, manufacturer, OpenGL version, and supported extensions.");
	i++;

	my_strcp(help_list[i].topic_name,"cls");
	my_strcp(help_list[i].topic_info,"\n[cls] - Clears the screen, killing all the history (however, the conversation log is not affected).");
	i++;

	my_strcp(help_list[i].topic_name,"date");
	my_strcp(help_list[i].topic_info,"\n[date] - Outputs the IN GAME date.");
	i++;

	my_strcp(help_list[i].topic_name,"time");
	my_strcp(help_list[i].topic_info,"\n[time] - Outputs the IN GAME time. Note that the IN GAME time is a cycle of 6 hours. From 0 to 1 is dawn, from 1 to 3 is full daylight, from 3 to 4 is evening, and from 4 to 0 is full night. Also, notice that a second in the game is a little longer than a real life second, so if you have an IN GAME meeting, or something, you might want not to rely on a real life clock.");
	i++;

	my_strcp(help_list[i].topic_name,"quit");
	my_strcp(help_list[i].topic_info,"\n[quit] - Quit the game, no questions asked. Notice that you can also use exit, or alt+x.");
	i++;

	my_strcp(help_list[i].topic_name,"exit");
	my_strcp(help_list[i].topic_info,"\n[exit] - Exit the game, no questions asked. Notice that you can also use quit, or alt+x.");
	i++;

	my_strcp(help_list[i].topic_name,"stats");
	my_strcp(help_list[i].topic_info,"\n[stats] - Gives some information about the server uptime, and how many players are logged in, etc.");
	i++;

	my_strcp(help_list[i].topic_name,"ignores");
	my_strcp(help_list[i].topic_info,"\n[ignores] - Display a list with the players that you currently ignore. Note that this list doesn't only include the names hold in local_ignores.txt and global_ignores.txt, it also includes the names you had chosen to ignore, but are not saved in one of those files (that is, when the #save_ignores directive, in el.ini is set on 0).");
	i++;

	my_strcp(help_list[i].topic_name,"ignore");
	my_strcp(help_list[i].topic_info,"\n[ignore] - Syntax:\"ignore name\". No quotemarks, of course. If you do that, you won't 'hear' any message from that player, whether public or private message. Depending on how the flag #save_ignores is set, the name will be added to the global_ignores.txt, which is loaded every time when you start the game, or not. Also, please not that you have to ignore the full name of that player, giving only the first few letters won't work as expected. Example: \"ignore Entropy\". If you try \"ignore entro\" instead, it won't work, it will ignore only the player entro. BTW, the ignore names are case insensitive. You can unignore players with the [unignore] command, and you can see a list of ignored people with the [ignores] command.");
	i++;

	my_strcp(help_list[i].topic_name,"unignore");
	my_strcp(help_list[i].topic_info,"\n[unigonre] - Syntax: \"unignore name\". Causes that name to be removed from the ignore list. However, it will NOT remove that name from the local_ignores.txt, you have to do it manually.");
	i++;

}

int get_help_topic(Uint8 *topic)
{
	int i,j,len,topic_id=0;
	int body_count=0;
	Uint8 ch1,ch2;

	len=strlen(topic);
	if(!len)return -1;
	for (i=0;i<MAX_HELP_ENTRIES;i++)
		{
        	{
				for(j=0;j<len;j++)
					{
						ch1=topic[j];
						ch2=help_list[i].topic_name[j];
						if(ch1>=65 && ch1<=90)ch1+=32;//make lowercase
						if(ch2>=65 && ch2<=90)ch2+=32;//make lowercase
						if(ch1!=ch2)break;
					}
				if(len==j)
					{
						if(help_list[i].topic_name[j]==0)
							{
								topic_id=i;
								return topic_id;
							}
						body_count++;
						topic_id=i;
					}
			}

		}
	if(!body_count)return -1;
	if(body_count>1)return -2;
	return topic_id;
}





void display_help_topic(Uint8 *topic)
{
	int result;
	Uint32	i;
	Uint8 str[1024];
	Uint8 ch;

	result=get_help_topic(topic);

	if(result==-1)
		{
			log_to_console(c_red1,"No such help topic!");
			return;
		}

	if(result==-2)
		{
			log_to_console(c_red1,"Ambiguous topic, please provide more letters!");
			return;
		}

	//if we are here, it means we did find the topic...

	str[0]=0;
	for(i=0;i<strlen(help_list[result].topic_info);i++)
		{
			ch=help_list[result].topic_info[i];
			if(ch=='[')ch=127+c_orange2;
			else
				if(ch==']')ch=127+c_grey1;
			str[i]=ch;
		}
	str[i+1]=0;
	log_to_console(c_grey1,str);
}

