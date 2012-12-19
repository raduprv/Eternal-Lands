#include "asc.h"
#include "actors.h"
#include "elwindows.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "interface.h"
#include "multiplayer.h"
#include "new_character.h"
#include "icon_window.h"
#include "init.h"
#include "textures.h"
#include "translate.h"
#include "sound.h"


static int	display_icons_handler(window_info *win);
static int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags);
static int	mouseover_icons_handler(window_info *win, int mx, int my);
static void add_icon(int icon_id, int coloured_icon_id, const char * help_name, const char * icon_type, void * data);
static void * get_action_mode(const char *name);


/*!
 * structure to store the data and event handlers for an icon
 */
typedef struct
{
	int state; /*!< the state of the icon, some icons are toggable */
	/*!
	 * \name icon image position
	 */
	/*! @{ */
	float u[2];
	float v[2];
	/*! @} */

	const char * help_message; /*!< icon help message */
    
	/*!
	 * \name Function pointer and data
	 */
	/*! @{ */
	int (*func)(void*, int);
	void * data;
	/*! @} */

	char data_type; /*!< data type indicator for \a data */
	char free_data; /*!< inidicator whether to free the data after use or not */
	Uint32 flashing;  /*!< if non-zero, the number times left to flash */
	Uint32 last_flash_change; /*!< if flashing, the time the state last changed */
} icon_struct;

int	icons_win= -1;

static icon_struct * icon_list[30]={NULL};
static int icons_no=0;
static int	icon_cursor_x; // to help highlight the proper icon

#define DATA_NONE -1
#define DATA_WINDOW 0
#define DATA_ACTIONMODE 1
#define DATA_MODE 2
#define DATA_KEY 3

#define STATE(I) (icon_list[I]->state&0x0F)
#define PRESSED (1|(1<<31))
#define NOT_ACTIVE 0


static float get_u_start(int icon_id)
{
#ifdef	NEW_TEXTURES
	float tmp = 32.0 * (float)(icon_id % 8)/256.0;
#else
	float tmp =  1.0 - (32.0 * (float)(icon_id % 8))/256.0;
#endif
	//printf("%s: %d %.2f\n", __FUNCTION__, icon_id, tmp);
	return tmp;
}

static float get_v_start(int icon_id)
{
	float tmp = 32.0 * (float)(icon_id >> 3)/256.0;
	//printf("%s: %d %.2f\n", __FUNCTION__, icon_id, tmp);
	return tmp;
}


static void do_keypress(const char *key_name, int id)
{
	if ((key_name != NULL) && *key_name)
	{
		Uint32 value = get_key_value(key_name);
		if (value)
			keypress_root_common(value, 0);
	}
}


void init_newchar_icons(void)
{
	/* wait until we have the root window to avoid hiding this one */
	if (newchar_root_win < 0)
		return;

	//create the icon window
	if(icons_win < 0)
		{
			icons_win= create_window("Icons", -1, 0, 0, window_height-32, window_width-hud_x, 32, ELW_TITLE_NONE|ELW_SHOW_LAST);
			set_window_handler(icons_win, ELW_HANDLER_DISPLAY, &display_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_CLICK, &click_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_MOUSEOVER, &mouseover_icons_handler);
		}
	else
		{
			move_window(icons_win, -1, 0, 0, window_height-32);
		}

	if(icons_no) return;

#ifndef NEW_NEW_CHAR_WINDOW
	add_icon(0, 18, "name_pass", "view_window", get_winid("name_pass") );
	add_icon(2, 20, "customize", "view_window", get_winid("customize") );
#endif
	add_icon(39, 38, "help", "view_window", get_winid("help") );
	add_icon(14, 34, "opts", "view_window", get_winid("opts") );
}

void init_peace_icons(void)
{
	//create the icon window
	if(icons_win < 0)
		{
			icons_win= create_window("Icons", -1, 0, 0, window_height-32, window_width-hud_x, 32, ELW_TITLE_NONE|ELW_SHOW_LAST);
			set_window_handler(icons_win, ELW_HANDLER_DISPLAY, &display_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_CLICK, &click_icons_handler);
			set_window_handler(icons_win, ELW_HANDLER_MOUSEOVER, &mouseover_icons_handler);
		}
	else
		{
			move_window(icons_win, -1, 0, 0, window_height-32);
		}

	if(icons_no) return;

	add_icon(0, 18, "walk", "switch_action_mode", get_action_mode("walk") );

	if(you_sit)
		add_icon(8, 26, "stand", "do_keypress", "#K_SIT" );
	else
		add_icon(7, 25, "sit", "do_keypress", "#K_SIT" );

	add_icon(2, 20, "look", "switch_action_mode", get_action_mode("look") );
	add_icon(15, 35, "use", "switch_action_mode", get_action_mode("use") );
	add_icon(47, 46, "use_witem", "switch_action_mode", get_action_mode("use_witem") );
	add_icon(4, 22, "trade", "switch_action_mode", get_action_mode("trade") );
	add_icon(5, 23, "attack", "switch_action_mode", get_action_mode("attack") );

	add_icon(11, 29, "invent", "view_window", get_winid("invent") );
	add_icon(9, 27, "spell", "view_window", get_winid("spell") );
	add_icon(12, 32, "manu", "view_window", get_winid("manu") );
	add_icon(45, 44, "emotewin", "view_window", get_winid("emotewin") );
	add_icon(19, 21, "quest", "view_window", get_winid("quest") );
	add_icon(36, 37, "map", "view_window", get_winid("map") );
	add_icon(3, 6, "info", "view_window", get_winid("info") );
	add_icon(10, 24, "buddy", "view_window", get_winid("buddy") );
	add_icon(13, 33, "stats", "view_window", get_winid("stats") );
	add_icon(1, 28, "console", "view_window", get_winid("console") );
	add_icon(39, 38, "help", "view_window", get_winid("help") );
	add_icon(14, 34, "opts", "view_window", get_winid("opts") );

	add_icon(16, 17, "XXX", "do_keypress", "#K_RANGINGWIN" );
	add_icon(16, 17, "range", "view_window", get_winid("range") );

	add_icon(16, 17, NULL, NULL, NULL );

	resize_window(icons_win, 32*icons_no, 32);
}

typedef struct
{
	char name[20];
	void *func;
	int datatype;
} icon_type;


static icon_type icon_types[] = {
	{ "switch_action_mode", switch_action_mode, DATA_ACTIONMODE },
	{ "do_keypress", do_keypress, DATA_KEY },
	{ "view_window", view_window, DATA_WINDOW } };

static void *get_func(const char *name)
{
	size_t i;
	if (name!=NULL)
		for (i=0; i<sizeof(icon_types)/sizeof(icon_type); i++)
			if (strcmp(name, icon_types[i].name) == 0)
				return icon_types[i].func;
	return NULL;
}

static int get_datatype(const char *name)
{
	size_t i;
	if (name!=NULL)
		for (i=0; i<sizeof(icon_types)/sizeof(icon_type); i++)
			if (strcmp(name, icon_types[i].name) == 0)
				return icon_types[i].datatype;
	return DATA_NONE;
}

typedef struct
{
	char name[20];
	void *mode;
} icon_action_mode;

static icon_action_mode icon_action_modes[] = {
	{ "walk", (void *)ACTION_WALK },
	{ "look", (void *)ACTION_LOOK },
	{ "use", (void *)ACTION_USE },
	{ "use_witem", (void *)ACTION_USE_WITEM },
	{ "trade", (void *)ACTION_TRADE },
	{ "attack", (void *)ACTION_ATTACK } };

static void * get_action_mode(const char *name)
{
	size_t i;
	if (name!=NULL)
		for (i=0; i<sizeof(icon_action_modes)/sizeof(icon_action_mode); i++)
			if (strcmp(name, icon_action_modes[i].name) == 0)
				return icon_action_modes[i].mode;
	return (void *)ACTION_WALK;
}


static void add_icon(int icon_id, int coloured_icon_id, const char * help_name, const char * icon_type, void * data)
{
	int no=icons_no++;

	icon_list[no]=(icon_struct*)calloc(1,sizeof(icon_struct));
	if(no == 0)
		icon_list[no]->state=PRESSED;
	else
		icon_list[no]->state=0;
	icon_list[no]->u[0]=get_u_start(icon_id);
	icon_list[no]->u[1]=get_u_start(coloured_icon_id);
	icon_list[no]->v[0]=get_v_start(icon_id);
	icon_list[no]->v[1]=get_v_start(coloured_icon_id);
	icon_list[no]->func=get_func(icon_type);
	icon_list[no]->help_message=get_named_string("tooltips", help_name);
	icon_list[no]->free_data=0;
	icon_list[no]->flashing=0;
	icon_list[no]->data_type=get_datatype(icon_type);
	switch(icon_list[no]->data_type)
		{
		case DATA_ACTIONMODE:
			icon_list[no]->data=(int*)calloc(1,sizeof(int));
			*(int *)icon_list[no]->data=(point)data;
			icon_list[no]->free_data=1;
			break;
		case DATA_MODE:	
		case DATA_WINDOW:
			icon_list[no]->data=data;
			break;
		case DATA_NONE:
			icon_list[no]->data=NULL;
			break;
		case DATA_KEY:
			{
				size_t len = strlen((char *)data) + 1;
				icon_list[no]->data=(char*)calloc(len,sizeof(char));
				safe_strncpy((char *)icon_list[no]->data, (char *)data, len);
				icon_list[no]->free_data=1;
			}
			break;
		}
}

void free_icons()
{
	int i;
	for(i=0;i<icons_no;i++) {
		if(icon_list[i]->free_data)
			free(icon_list[i]->data);
		free(icon_list[i]);
	}
	icons_no=0;
}

void flash_icon(const char* name, Uint32 seconds)
{
	int i;
	for(i=0;i<icons_no;i++)
		if (strcmp(name, icon_list[i]->help_message) == 0)
		{
			icon_list[i]->flashing = 4*seconds;
			break;
		}
}

static int	mouseover_icons_handler(window_info *win, int mx, int my)
{
	icon_cursor_x= mx;	// just memorize for later

	return 0;
}

static int	display_icons_handler(window_info *win)
{
	int i, state=-1, *z;

	for(i=0;i<icons_no;i++)
		{
			z = (int*)icon_list[i]->data;
			switch(icon_list[i]->data_type) {
			case DATA_WINDOW:
				if ( z!=NULL && *z >= 0 && (windows_list.window[*z].displayed || windows_list.window[*z].reinstate) )
					icon_list[i]->state = 1;
				else
					icon_list[i]->state = 0;
				break;
			case DATA_MODE:
				icon_list[i]->state=get_show_window (*z);
				break;
			case DATA_ACTIONMODE:
				icon_list[i]->state=(action_mode==*z);
				break;
			default:
				icon_list[i]->state=0;
				break;
			}
		}
	
	if(mouse_in_window(win->window_id, mouse_x, mouse_y))
		{
			state=icon_cursor_x/32;//Icons must be 32 pixels wide...
			if(state<icons_no)icon_list[state]->state|=0x01;//Set the state to be mouseover
			else state=-1;
		}
	
#ifdef	NEW_TEXTURES
	bind_texture(icons_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(icons_text);
#endif	/* NEW_TEXTURES */
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	
	for(i=0;i<icons_no;i++)
		{
			size_t index = STATE(i);
			if (icon_list[i]->flashing)
			{
				if (abs(SDL_GetTicks() - icon_list[i]->last_flash_change) > 250)
				{
					icon_list[i]->last_flash_change = SDL_GetTicks();
					icon_list[i]->flashing--;
				}
				index = icon_list[i]->flashing & 1;
			}
			draw_2d_thing(
				icon_list[i]->u[index],
				icon_list[i]->v[index], 
				icon_list[i]->u[index]+(float)31/256,
#ifdef	NEW_TEXTURES
				icon_list[i]->v[index] + (float)31/256,
#else	/* NEW_TEXTURES */
				icon_list[i]->v[index]-(float)31/256,
#endif	/* NEW_TEXTURES */
				i*32,0,i*32+31,32
				);
			if(!(icon_list[i]->state>>31))icon_list[i]->state=0;//Else we pressed the button and it should still be pressed
		}
	glEnd();

	if(state>=0 && show_help_text) show_help(icon_list[state]->help_message, 32*(state+1)+2, 10);//Show the help message

	return 1;
}

static int	click_icons_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int id=mx/32;//Icons are always 32 bit wide
	
	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(combat_mode)return 0;

	if(id<icons_no)
		{
			switch(icon_list[id]->data_type)
				{
				case DATA_MODE:
				case DATA_ACTIONMODE:
				case DATA_WINDOW:
				case DATA_KEY:
					{
						int * data=(int *)icon_list[id]->data;
						if (icon_list[id]->func != NULL)
							icon_list[id]->func(data, id);
						break;
					}
				default:
					{
						if (icon_list[id]->func != NULL)
							icon_list[id]->func(0, id);
						break;
					}
				}
			do_icon_click_sound();
			// cancel any flashing icon now user has responded
			icon_list[id]->flashing = 0;
		}
	return 1;
}

void sit_button_pressed(void * none, int id)
{
	if(you_sit)
		{
			Uint8 str[4];
			//Send message to server...	
			str[0]=SIT_DOWN;
			str[1]=0;
			my_tcp_send(my_socket,str,2);
		}
	else
		{
			Uint8 str[4];
			//Send message to server...
			str[0]=SIT_DOWN;
			str[1]=1;
			my_tcp_send(my_socket,str,2);
		}
}

void you_sit_down()
{
	you_sit=1;
	if(!icon_list[1])return;
	icon_list[1]->u[0]=get_u_start(8);//Change the icon to stand
	icon_list[1]->u[1]=get_u_start(26);
	icon_list[1]->v[0]=get_v_start(8);
	icon_list[1]->v[1]=get_v_start(26);
	icon_list[1]->help_message=get_named_string("tooltips", "stand");
}

void you_stand_up()
{
	you_sit=0;
	if(!icon_list[1])return;
	icon_list[1]->u[0]=get_u_start(7);
	icon_list[1]->u[1]=get_u_start(25);
	icon_list[1]->v[0]=get_v_start(7);
	icon_list[1]->v[1]=get_v_start(25);
	icon_list[1]->help_message=get_named_string("tooltips", "sit");
}


int get_icons_win_active_len(void)
{
	return 32 * icons_no;
}
