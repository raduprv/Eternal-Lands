#include <stdlib.h>
#include <string.h>
#include <SDL_endian.h>
#include "context_menu.h"
#include "console.h"
#include "elconfig.h"
#include "dialogues.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "init.h"
#include "hud.h"
#include "interface.h"
#include "multiplayer.h"
#include "paste.h"
#include "sound.h"
#include "textures.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

/*!
 * response structure used in dialogues with NPCs. It contains the data of a response from some NPC.
 */
typedef struct{
	char text[200]; /*!< text of the response */

	int width_in_char;  /*!< the length of the respon option in characters */

    /*! \name calculate response coordinates @{ */
    int pos_x, pos_y, width;
    /*! @} */

	int to_actor; /*!< id of the actor to which this response is directed */
	int response_id; /*!< unique id of the response */
	int in_use; /*!< flag whether this response is in use or not */
	int mouse_over; /*!< flag whether the mouse is over this response */
}response;

#define MAX_RESPONSES 40 /* max. number of response entries in \see dialogue_responces */
#define	MAX_PORTRAITS_TEXTURES	16
#define MAX_MESS_LINES 9
#define MAX_SAVED_RESPONSES 8

static unsigned char dialogue_string[2048];
unsigned char npc_name[20] = "";
char npc_mark_str[20] = "%s (npc)";
int cur_portrait=8;
int portraits_tex[MAX_PORTRAITS_TEXTURES];

response dialogue_responces[MAX_RESPONSES];
int dialogue_win= -1;

int dialogue_menu_x=1;
int dialogue_menu_y=1;
static int char_frame_size = 0;
static int char_size = 0;
static int border_space = 0;
static const int available_text_width = 70;
static int bot_line_height = 0;

static int show_keypress_letters = 0;
static int recalc_option_positions = 1;
int autoclose_storage_dialogue=0;
int auto_select_storage_option=0;
int dialogue_copy_excludes_responses=0;
int dialogue_copy_excludes_newlines=0;
static int done_auto_storage_select=0;
static Uint32 copy_end_highlight_time = 0;
static Uint32 repeat_end_highlight_time = 0;
static int close_str_width = -1;
static int copy_str_width = -1;
static int repeat_str_width = -1;
static int close_pos_x = 0;
static int copy_pos_x = 0;
static int repeat_pos_x = 0;
static int highlight_close = 0;
static int highlight_copy = 0;
static int highlight_repeat = 0;
static int mouse_over_name = 0;
static int response_y_offset = 0;
static size_t cm_npcname_id = CM_INIT_VALUE;
static size_t cm_dialog_copy_id = CM_INIT_VALUE;
static size_t cm_dialog_repeat_id = CM_INIT_VALUE;
static int new_dialogue = 1;
static int npc_name_x_start = 0;
static int npc_name_len = 0;
static size_t saved_response_list_top = 0;
static size_t saved_response_list_bot = 0;
static size_t saved_response_list_cur = 0;
static size_t saved_response_init = 0;
static response saved_responses[MAX_SAVED_RESPONSES];
static int cm_dialogue_repeat_handler(window_info *win, int widget_id, int mx, int my, int option);
static void send_response(window_info *win, const response *the_response);


void load_dialogue_portraits(void)
{
	int i;
	for(i=0; i<MAX_PORTRAITS_TEXTURES; i++)
	{
		char buffer[256];
		safe_snprintf(buffer, sizeof(buffer), "textures/portraits%d.dds", i+1);
		if (check_image_name(buffer, sizeof(buffer), buffer) != 0)
			portraits_tex[i] = load_texture_cached(buffer, tt_gui);
	}
}

void clear_dialogue_responses(void)
{
	int i;
	for(i=0;i<MAX_RESPONSES;i++)
		dialogue_responces[i].in_use=0;
}

void build_response_entries (const Uint8 *data, int total_length)
{
	int i = 0;
	int len = 0;
	int last_index=0;

	//first, clear the previous dialogue entries
	clear_dialogue_responses();
	recalc_option_positions = new_dialogue = 1;

	for(i=0; i < MAX_RESPONSES;i++)
	{
		// break if we don't have a length field
		if (last_index + 3 > total_length)
			break;
		len = SDL_SwapLE16(*((Uint16 *)(data + last_index)));
		// break if we don't have a complete response
		if (last_index + 3 + len + 2 + 2 > total_length)
			break;

		dialogue_responces[i].in_use = 1;
		my_strncp(dialogue_responces[i].text, (char*)&data[last_index + 2], len);
		dialogue_responces[i].response_id = SDL_SwapLE16(*((Uint16 *)(data + last_index + 2 + len)));
		dialogue_responces[i].to_actor = SDL_SwapLE16(*((Uint16 *)(data + last_index + 2 + 2 + len)));
		dialogue_responces[i].width_in_char = len;
		last_index += len + 3 * 2;
	}

	/* remove any previous saved responses */
	if (saved_response_init)
		for(i=0;i<MAX_RESPONSES;i++)
			if (dialogue_responces[i].in_use && (dialogue_responces[i].to_actor != saved_responses[saved_response_list_top].to_actor))
			{
				saved_response_list_bot = saved_response_list_top = saved_response_init = saved_response_list_cur = 0;
				cm_set(cm_dialog_repeat_id, "--\n", NULL);
				break;
			}
}

static void calculate_option_positions(window_info *win)
{
	int i = 0;
	int start_x = border_space;
	int start_y = response_y_offset;
	int width = 0;
	int width_extra = (show_keypress_letters) ?3: 0; // e.g "1] "
	for(i=0; i < MAX_RESPONSES;i++)
	{
		if (!dialogue_responces[i].in_use)
			break;
		width = (dialogue_responces[i].width_in_char + ((i<36) ?width_extra: 0)) * win->small_font_len_x;
		if ((start_x + width) > (win->len_x - 2 * border_space))
		{
			start_x = border_space;
			start_y += win->small_font_len_y;
		}
		dialogue_responces[i].pos_x = start_x;
		dialogue_responces[i].pos_y = start_y;
		dialogue_responces[i].width = width;
		start_x += width + 2 * win->small_font_len_x;
	}
	recalc_option_positions = 0;
}

static int display_dialogue_handler(window_info *win)
{
	static int last_show_keypress_letters = -1;
	int i;
	int last_pos_y = 0;
	unsigned char str[128];

	// auto select storage option
	if (auto_select_storage_option && !done_auto_storage_select)
	{
		for(i=0;i<MAX_RESPONSES;i++)
			if (dialogue_responces[i].in_use && (strcmp(dialogue_responces[i].text, open_storage_str) == 0))
			{
				send_response(win, &dialogue_responces[i]);
				done_auto_storage_select = 1;
				break;
			}
	}

	if (last_show_keypress_letters != show_keypress_letters)
	{
		last_show_keypress_letters = show_keypress_letters;
		recalc_option_positions = 1;
	}
	if (recalc_option_positions)
		calculate_option_positions(win);

	//calculate the npc_name_x_start (to have it centered on the screen)
	npc_name_len = strlen((char*)npc_name);
	npc_name_x_start = win->len_x / 2 - (npc_name_len * win->small_font_len_x) / 2;

	glDisable(GL_TEXTURE_2D);
	//draw the character frame
	glColor3f(0.0f,1.0f,1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3i(0,0,0);
	glVertex3i(char_frame_size, 0, 0);
	glVertex3i(char_frame_size, char_frame_size, 0);
	glVertex3i(0, char_frame_size, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the portrait
	if(cur_portrait!=-1)
	{
		float u_start,v_start,u_end,v_end;
		int x_start,x_end,y_start,y_end;
		int this_texture;

		//get the UV coordinates.
		u_start = 0.25f * (cur_portrait % 4);
		u_end = u_start + 0.25f;
		v_start = 0.25f * (cur_portrait / 4);
		v_end = v_start + 0.25f;

		//get the x and y
		x_start=1;
		x_end = x_start + char_size;
		y_start=1;
		y_end = y_start + char_size;

		//get the texture this item belongs to
		this_texture=cur_portrait/16;
		this_texture=portraits_tex[this_texture];

		bind_texture(this_texture);
		glBegin(GL_QUADS);
		draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
		glEnd();
	}

	//draw the main text
	draw_string_small_zoomed(char_frame_size + border_space, border_space, dialogue_string, MAX_MESS_LINES, win->current_scale);

	//ok, now draw the responses
	for(i=0;i<MAX_RESPONSES;i++)
	{
		if(dialogue_responces[i].in_use)
		{
			if(dialogue_responces[i].mouse_over)
				glColor3f(1.0f,0.5f,0.0f);
			else
				glColor3f(1.0f,1.0f,0.0f);
			if(show_keypress_letters)
			{
				if(i>=0 && i<=8) // 1-9
					safe_snprintf((char*)str,sizeof(str),"%c] %s",49+i,(unsigned char*)dialogue_responces[i].text);
				else if(i==9) //0
					safe_snprintf((char*)str,sizeof(str),"0] %s",(unsigned char*)dialogue_responces[i].text);
				else if(i>=10 && i<=35) // A-Z
					safe_snprintf((char*)str,sizeof(str),"%c] %s",55+i, (unsigned char*)dialogue_responces[i].text);
				else // too many dialogue options, you have to click these
					 safe_snprintf((char*)str,sizeof(str),"%s",(unsigned char*)dialogue_responces[i].text);
			}
			else
				safe_snprintf((char*)str,sizeof(str),"%s",(unsigned char*)dialogue_responces[i].text);
			last_pos_y = dialogue_responces[i].pos_y;
			draw_string_small_zoomed(dialogue_responces[i].pos_x, last_pos_y, str, 1, win->current_scale);
		}
		else
			break;
	}

	// automatically resize window if there are more options than space
	if ((last_pos_y + 3 * win->small_font_len_y + 1) > win->len_y)
	{
		resize_window(win->window_id, win->len_x, last_pos_y + 3 * win->small_font_len_y + 1);
		new_dialogue = 1;
	}

	//now, draw the character name
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small_zoomed(npc_name_x_start, win->len_y - bot_line_height, npc_name, 1, win->current_scale);

	if (highlight_close)
		glColor3f(1.0f,0.5f,0.0f);
	else
		glColor3f(1.0f,1.0f,1.0f);
	draw_string_small_zoomed(close_pos_x, win->len_y - bot_line_height, (unsigned char*)close_str, 1, win->current_scale);

	if (copy_end_highlight_time > SDL_GetTicks())
		glColor3f(1.0f,0.25f,0.0f);
	else if (highlight_copy)
		glColor3f(1.0f,0.5f,0.0f);
	else
		glColor3f(1.0f,1.0f,1.0f);
	draw_string_small_zoomed(copy_pos_x, win->len_y - bot_line_height, (unsigned char*)dialogue_copy_str, 1, win->current_scale);

	if (!saved_response_init)
		glColor3f(0.5f,0.5f,0.5f);
	else if (repeat_end_highlight_time > SDL_GetTicks())
		glColor3f(1.0f,0.25f,0.0f);
	else if (highlight_repeat)
		glColor3f(1.0f,0.5f,0.0f);
	else
		glColor3f(1.0f,1.0f,1.0f);
	draw_string_small_zoomed(repeat_pos_x, win->len_y - bot_line_height, (unsigned char*)dialogue_repeat_str, 1, win->current_scale);

	// display help text if appropriate
	if ((show_help_text) && (highlight_repeat || highlight_copy || mouse_over_name))
		show_help(cm_help_options_str, 0, win->len_y+10, win->current_scale);

	show_keypress_letters = highlight_close = highlight_copy = highlight_repeat = mouse_over_name = 0;

	// if this is the first time we displayed this dialogue, do first time stuff
	if (new_dialogue)
	{
		new_dialogue = 0;
		done_auto_storage_select = 0;
		cm_remove_regions(win->window_id);
		cm_add_region(cm_npcname_id, win->window_id, npc_name_x_start,
			win->len_y - bot_line_height, npc_name_len * win->small_font_len_x, win->small_font_len_y);
		cm_add_region(cm_dialog_copy_id, win->window_id, copy_pos_x,
			win->len_y - bot_line_height, copy_str_width, win->small_font_len_y);
		cm_add_region(cm_dialog_repeat_id, win->window_id, repeat_pos_x,
			win->len_y - bot_line_height, repeat_str_width, win->small_font_len_y);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 0;
}

void close_dialogue(void)
{
	if(dialogue_win >= 0)
	{
		hide_window(dialogue_win);
	}
}

static int mouseover_dialogue_handler(window_info *win, int mx, int my)
{
	int i;

	show_keypress_letters = 0;
	if(use_keypress_dialogue_boxes)
	{
	 	if(use_full_dialogue_window || ((mx >= 0 && mx <= char_size) && (my >= 0 && my <= char_size)))
			show_keypress_letters = 1;
	}

	if(mx >= close_pos_x && mx < (close_pos_x + close_str_width) && my >= (win->len_y - bot_line_height))
		highlight_close = 1;
	if(mx > copy_pos_x && mx < (copy_pos_x + copy_str_width) && my >= (win->len_y - bot_line_height))
		highlight_copy = 1;
	if(mx > repeat_pos_x && mx < (repeat_pos_x + repeat_str_width) &&  my >= (win->len_y - bot_line_height))
		highlight_repeat = 1;
	if (mx > npc_name_x_start && mx < (npc_name_x_start + npc_name_len * win->small_font_len_x) && my >= (win->len_y - bot_line_height))
		mouse_over_name = 1;

	//first, clear the mouse overs
	for(i=0;i<MAX_RESPONSES;i++)
		dialogue_responces[i].mouse_over=0;

	for(i=0;i<MAX_RESPONSES;i++)
	{
		if(dialogue_responces[i].in_use)
		{
			if (mx >= dialogue_responces[i].pos_x && mx <= (dialogue_responces[i].pos_x + dialogue_responces[i].width) &&
				my >= dialogue_responces[i].pos_y && my <= (dialogue_responces[i].pos_y + win->small_font_len_y))
			{
				dialogue_responces[i].mouse_over=1;
				return 0;
			}
		}
		else
			break;
	}
	return 0;
}


#if 0
/* just for debug */
static void show_response_list(void)
{
	size_t i;
	printf("Responses:-\n");
	for (i=saved_response_list_top; saved_response_list_bot<MAX_SAVED_RESPONSES; i--)
	{
		printf("[%s] %s\n", saved_responses[i].text, ((i==saved_response_list_cur) ?"*" :""));
		if (i == saved_response_list_bot)
			break;
		if (i == 0)
			i = MAX_SAVED_RESPONSES;
	}
	printf("\n");
}
#endif

static void save_response(const response *last_response)
{
	size_t i;

	/* not the first time for this dialogue */
	if (saved_response_init)
	{
		/* check for repeats */
		for (i=saved_response_list_top; saved_response_list_bot<MAX_SAVED_RESPONSES; i--)
		{
			if (saved_responses[i].response_id == last_response->response_id)
			{
				saved_response_list_cur = i;
				return;
			}
			if (i == saved_response_list_bot)
				break;
			if (i == 0)
				i = MAX_SAVED_RESPONSES;
		}
		/* move indexes */
		if (++saved_response_list_top >= MAX_SAVED_RESPONSES)
			saved_response_list_top = 0;
		if (saved_response_list_top == saved_response_list_bot)
			if (++saved_response_list_bot >= MAX_SAVED_RESPONSES)
				saved_response_list_bot = 0;
	}
	/* first time for this dialogue */
	else
		saved_response_init = 1;

	/* save the response - memcpy() ok as no pointers */
	saved_response_list_cur = saved_response_list_top;
	memcpy(&saved_responses[saved_response_list_top], last_response, sizeof(response));

	/* rebuild the context menu from current saved responses, newest first */
	cm_set(cm_dialog_repeat_id, "", cm_dialogue_repeat_handler);
	for (i=saved_response_list_top; saved_response_list_bot<MAX_SAVED_RESPONSES; i--)
	{
		cm_add(cm_dialog_repeat_id, saved_responses[i].text, NULL);
		if (i == saved_response_list_bot)
			break;
		if (i == 0)
			i = MAX_SAVED_RESPONSES;
	}
}


static void send_response(window_info *win, const response *the_response)
{
	Uint8 str[16];
	str[0]=RESPOND_TO_NPC;
	*((Uint16 *)(str+1))=SDL_SwapLE16((short)the_response->to_actor);
	*((Uint16 *)(str+3))=SDL_SwapLE16((short)the_response->response_id);
	my_tcp_send(my_socket,str,5);
	if (autoclose_storage_dialogue && strcmp(the_response->text, open_storage_str) == 0)
 		hide_window(win->window_id);
	save_response(the_response);
}


static void copy_dialogue_text(void)
{
	size_t to_str_max = 2048 + 1024;
	char *to_str = (char *)malloc(to_str_max);
	size_t response_str_len = 128;
	char *response_str = (char *)malloc(response_str_len);
	size_t from_str_len = strlen((char *)dialogue_string);
	int response_num = 1;
	int from_index = 0;
	int to_index = 0;
	int i;

	// copy the body of text stripped of any colour, soft wrapping and optionally, newlines characters
	while(from_index<from_str_len && to_index<to_str_max-1)
	{
		if (!is_color (dialogue_string[from_index]) && dialogue_string[from_index] != '\r' &&
			(!dialogue_copy_excludes_newlines || dialogue_string[from_index] != '\n'))
			to_str[to_index++] = dialogue_string[from_index];
		from_index++;
	}
	to_str[to_index] = '\0';

	if (!dialogue_copy_excludes_responses)
	{
		// if there are responses, add some new lines between the text.
		for(i=0;i<MAX_RESPONSES;i++)
			if(dialogue_responces[i].in_use)
			{
				safe_strcat(to_str, "\n\n", to_str_max);
				break;
			}
	
		// add the numbered response strings, one per line
		for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use)
			{
				safe_snprintf(response_str, response_str_len, "%d) %s\n", response_num++, dialogue_responces[i].text);
				safe_strcat(to_str, response_str, to_str_max);
			}
		}
	}

	copy_to_clipboard(to_str);
	free(response_str);
	free(to_str);
}

static void send_repeat(window_info *win)
{
	if (!saved_response_init)
		return;
	repeat_end_highlight_time = SDL_GetTicks() + 500;
	send_response(win, &saved_responses[saved_response_list_cur]);
	do_click_sound();
}

static void do_copy(void)
{
	copy_end_highlight_time = SDL_GetTicks() + 500;
	copy_dialogue_text();
	do_click_sound();
}

static int click_dialogue_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	for(i=0;i<MAX_RESPONSES;i++)
		{
			if(dialogue_responces[i].in_use && dialogue_responces[i].mouse_over)
				{
					send_response(win, &dialogue_responces[i]);
					do_click_sound();
					return 1;
				}
		}
	if(mx >= close_pos_x && mx < (close_pos_x + close_str_width) && my >= (win->len_y - bot_line_height))
		{
			do_window_close_sound();
			hide_window(win->window_id);
			return 1;
		}
	if((flags & ELW_LEFT_MOUSE) && mx > copy_pos_x && mx < (copy_pos_x + copy_str_width) && my >= (win->len_y - bot_line_height))
		{
			do_copy();
			return 1;
		}
	if((flags & ELW_LEFT_MOUSE) && mx > repeat_pos_x && mx < (repeat_pos_x + repeat_str_width) && my >= (win->len_y - bot_line_height))
		{
			send_repeat(win);
			return 1;
		}

	return 0;
}

static int keypress_dialogue_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch;

	if ((key & 0xffff) == SDLK_ESCAPE) // close window if Escape pressed
	{
		do_window_close_sound();
		hide_window(win->window_id);
		return 1;
	}

	if(!use_keypress_dialogue_boxes)
	{
		return 0;
	}

	if ((use_full_dialogue_window == 0) && (mx<0 || mx>char_size || my<0 || my>char_size))
	{
		return 0;
	}

	ch = key_to_char (unikey);

	if(ch<'0' || ch>'z') // do not send special keys
	{
		return 0;
	}
	if(ch>='a' && ch<='z')
		ch-=87; //a-z->10-35
	else if(ch>='A' && ch<='Z')
		ch-=55; //A-Z->10-35
	else if(ch>='1' && ch<='9')
		ch-=49; //1-9->0-8
	else if(ch=='0') //0->9
		ch=9;
	else //out of range
	{
		return 0;
	}
	
	// if not being used for responses, check for other use
	if ((key & ELW_ALT) && ((MAX_RESPONSES-1<ch) || (dialogue_responces[ch].in_use == 0)))
	{
		if ((strlen(dialogue_repeat_str)>1) && (ch == (Uint8)dialogue_repeat_str[1]-87))
		{
			send_repeat(win);
			return 1;
		}
		if ((strlen(dialogue_copy_str)>1) && (ch == (Uint8)dialogue_copy_str[1]-87))
		{
			do_copy();
			return 1;
		}
	}

	if((key & ELW_ALT) || (key & ELW_CTRL)) //Do not process Ctrl or Alt keypresses
	{
		return 0;
	}

	if(MAX_RESPONSES-1<ch)//pressed a key that the client is not expecting, ignore it
	{
		return 1;
	}
	if(dialogue_responces[ch].in_use)
	{
		do_click_sound();
		send_response(win, &dialogue_responces[ch]);
		return 1;
	}
	return 0;
}

static int cm_dialogue_repeat_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (saved_response_init && (option < MAX_SAVED_RESPONSES))
	{
		size_t i = saved_response_list_top;
		if (option > i)
			i += MAX_SAVED_RESPONSES;
		i -= option;
		send_response(win, &saved_responses[i]);
		return 1;
	}
	else
		return 0;
}

static int cm_npcname_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option == 0)
		copy_to_clipboard((const char *)npc_name);
	else if (option == 1)
	{
		char str[80];
		safe_snprintf(str, sizeof(str), npc_mark_str, npc_name);
		command_unmark_special(str, strlen(str), 0);
		command_mark(str, strlen(str));
	}
	else
		return 0;
	return 1;
}

int ui_scale_dialogue_handler(window_info *win)
{
	int dialogue_menu_x_len = (int)(0.5 + available_text_width * win->small_font_len_x);
	int dialogue_menu_y_len = (int)(0.5 + win->current_scale * 220);
	char_size = (int)(0.5 + win->current_scale * 64);
	char_frame_size = char_size + 2;
	border_space = (int)(0.5 + win->current_scale * 5);
	dialogue_menu_x_len += 2 * border_space + char_frame_size;
	response_y_offset = 2 * border_space + MAX_MESS_LINES * win->small_font_len_y;
	bot_line_height = win->small_font_len_y + 1;

	copy_pos_x = border_space;
	copy_str_width = get_string_width((unsigned char*)dialogue_copy_str) * win->small_font_len_x / 12.0;
	close_str_width = get_string_width((unsigned char*)close_str) * win->small_font_len_x / 12.0;
	close_pos_x = dialogue_menu_x_len - close_str_width - border_space;
	repeat_str_width = get_string_width((unsigned char*)dialogue_repeat_str) * win->small_font_len_x / 12.0;
	repeat_pos_x = copy_pos_x + close_str_width + 2 * border_space;

	resize_window(win->window_id, dialogue_menu_x_len, dialogue_menu_y_len);
	recalc_option_positions = new_dialogue = 1;

	return 1;
}

void display_dialogue(const Uint8 *in_data, int data_length)
{	
	if (!get_show_window(dialogue_win))
		do_icon_click_sound();

	if(dialogue_win < 0)
	{
		dialogue_win= create_window("Dialogue", game_root_win, 0, dialogue_menu_x, dialogue_menu_y, 0, 0, (ELW_USE_UISCALE|ELW_WIN_DEFAULT)^ELW_CLOSE_BOX);

		set_window_handler(dialogue_win, ELW_HANDLER_DISPLAY, &display_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_MOUSEOVER, &mouseover_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_KEYPRESS, &keypress_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_CLICK, &click_dialogue_handler );
		set_window_handler(dialogue_win, ELW_HANDLER_UI_SCALE, &ui_scale_dialogue_handler );

		cm_add(windows_list.window[dialogue_win].cm_id, cm_dialog_menu_str, NULL);
		cm_add(windows_list.window[dialogue_win].cm_id, cm_dialog_options_str, NULL);
		cm_bool_line(windows_list.window[dialogue_win].cm_id, ELW_CM_MENU_LEN+1, &use_keypress_dialogue_boxes, "use_keypress_dialog_boxes");
		cm_bool_line(windows_list.window[dialogue_win].cm_id, ELW_CM_MENU_LEN+2, &use_full_dialogue_window, "use_full_dialogue_window");
		cm_bool_line(windows_list.window[dialogue_win].cm_id, ELW_CM_MENU_LEN+3, &autoclose_storage_dialogue, NULL);
		cm_bool_line(windows_list.window[dialogue_win].cm_id, ELW_CM_MENU_LEN+4, &auto_select_storage_option, NULL);

		cm_npcname_id = cm_create(cm_npcname_menu_str, cm_npcname_handler);
		cm_dialog_copy_id = cm_create(cm_dialog_copy_menu_str, NULL);
		cm_dialog_repeat_id = cm_create("--\n", cm_dialogue_repeat_handler);
		cm_bool_line(cm_dialog_copy_id, 0, &dialogue_copy_excludes_responses, NULL);
		cm_bool_line(cm_dialog_copy_id, 1, &dialogue_copy_excludes_newlines, NULL);

	}
	else
	{
		show_window(dialogue_win);
		select_window(dialogue_win);
	}

	if (dialogue_win >=0 && dialogue_win < windows_list.num_windows)
		ui_scale_dialogue_handler(&windows_list.window[dialogue_win]);

	put_small_text_in_box(in_data, data_length, available_text_width * SMALL_FONT_X_LEN, (char*)dialogue_string);
	recalc_option_positions = new_dialogue = 1;
}

