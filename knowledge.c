#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "knowledge.h"
#include "asc.h"
#include "books.h"
#include "context_menu.h"
#include "elconfig.h"
#include "elloggingwrapper.h"
#include "io/elpathwrapper.h"
#include "elwindows.h"
#include "gamewin.h"
#include "hud.h"
#include "multiplayer.h"
#include "named_colours.h"
#include "notepad.h"
#include "paste.h"
#include "sound.h"
#include "stats.h"
#include "tabs.h"
#include "textures.h"
#include "translate.h"
#include "widgets.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

static int knowledge_scroll_id= 13;
static int knowledge_book_image_id;
static int knowledge_book_label_id;
static int knowledge_book_id= 0;

knowledge knowledge_list[KNOWLEDGE_LIST_SIZE];
static int knowledge_count= 0;

#define TEXTBUFSIZE 400
static char raw_knowledge_string[TEXTBUFSIZE]="";

static size_t cm_know_id = CM_INIT_VALUE;
static INPUT_POPUP ipu_know;
static char highlight_string[KNOWLEDGE_NAME_SIZE] = "";
static int know_show_win_help = 0;
static int mouse_over_progress_bar = 0;
static int selected_book = -1;

static const int displayed_book_rows = 16;
static const int info_lines = 5;
static int booklist_y_len = 0;
static int booklist_y_step = 0;
static int progressbox_y_len = 0;
static int progress_top_y = 0;
static int progress_bot_y = 0;
static int progress_right_x = 0;
static int progress_left_x = 0;
static int text_border = 0;
static int book_start_x = 0;

static int add_knowledge_book_image(int window_id)
{
	// Book image
	int isize, tsize, tid, picsperrow, xtile, ytile, id;
	float ftsize, u, v, uend, vend;

	isize=256;
	tsize=51;
	tid=21;
	picsperrow=isize/tsize;
	xtile=tid%picsperrow;
	ytile=tid/picsperrow;
	ftsize=(float)tsize/isize;
	u=ftsize*xtile;
	v=-ftsize*ytile;
	uend=u+ftsize;
	vend=v-ftsize;
	id = load_texture_cached("textures/items1", tt_gui);
	return image_add_extended(window_id, 0, NULL, 0, 0, 0, 0, WIDGET_DISABLED, 1.0, 1.0, 1.0, 1.0, id, u, v, uend, vend, 0.05f);
}

int handle_knowledge_book(void)
{
	open_book(knowledge_book_id + 10000);
	// Bring the new window to the front               <----- Doesn't work. Is in front for the first usage, but not after that
	select_window(book_win);
	return 1;
}

void check_book_known(void)
{
	static int last_checked_book = -1;
	if (your_info.researching != last_checked_book)
	{
		last_checked_book = your_info.researching;
		if ((your_info.researching >= knowledge_count) && (your_info.researching < sizeof(knowledge_list)))
			LOG_TO_CONSOLE(c_red2, unknown_book_long_str);
	}
}

int is_researching(void)
{
	if (your_info.researching < sizeof(knowledge_list))
		return 1;
	else
		return 0;
}

float get_research_fraction(void)
{
	float progress = 0;
	if (your_info.research_total > 0)
		progress = (float)your_info.research_completed / (float)your_info.research_total;
	if (progress > 1)
		progress = 1;
	return progress;
}

static float research_rate = -1;

void update_research_rate(void)
{
	static int last_research_completed = -1;
	if (last_research_completed > your_info.research_completed)
		last_research_completed = -1;
	if (last_research_completed > 0)
	{
		if ((your_info.research_completed - last_research_completed) > 0)
		{
			research_rate = 1.0 / (float)(your_info.research_completed - last_research_completed);
			last_research_completed = your_info.research_completed;
		}
	}
	else
	{
		last_research_completed = your_info.research_completed;
		research_rate = 2.0 / (float)(your_info.wil.cur + your_info.rea.cur);
	}
}

static float get_research_eta(void)
{
	if (research_rate < 0)
		return 0;
	return research_rate * (your_info.research_total - your_info.research_completed);
}

char *get_research_eta_str(char *str, size_t size)
{
	float eta = get_research_eta();
	if (eta < 0.01)
		safe_snprintf(str, size, completed_research);
	else if (eta < 1)
		safe_snprintf(str, size, lessthanaminute_str);
	else
	{
		int ieta = (int)(eta + 0.5);
		safe_snprintf(str, size, "ETA: %i %s", ieta, (ieta==1)?minute_str:minutes_str);
	}
	return str;
}

int display_knowledge_handler(window_info *win)
{
	size_t i;
	int x = text_border;
	int y = text_border;
	int scroll = vscrollbar_get_pos (win->window_id, knowledge_scroll_id);
	char points_string[16];
	char *research_string;
	int points_pos;
	float font_ratio = win->small_font_len_x/12.0;
	float max_name_x = (win->len_x - win->box_size - 2*x)/2;
	int is_researching = 1;
	int text_width = win->len_x - 2 * text_border;
	char knowledge_text_buf[TEXTBUFSIZE];

	if(your_info.research_total &&
	   (your_info.research_completed==your_info.research_total))
		safe_snprintf(points_string, sizeof(points_string), "%s", completed_research);
	else
		safe_snprintf(points_string, sizeof(points_string), "%i/%i",your_info.research_completed,your_info.research_total);
	if(your_info.researching < knowledge_count)
	{
		research_string = knowledge_list[your_info.researching].name;
	}
	else if (your_info.researching < sizeof(knowledge_list))
	{
		research_string = unknown_book_short_str;
	}
	else
	{
		research_string = not_researching_anything;
		points_string[0] = '\0';
		is_researching = 0;
	}
	points_pos = (progress_right_x - progress_left_x - strlen(points_string) * win->small_font_len_x) / 2;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	// window separators
	glVertex3i(0,booklist_y_len,0);
	glVertex3i(win->len_x,booklist_y_len,0);
	glVertex3i(0,progressbox_y_len,0);
	glVertex3i(win->len_x,progressbox_y_len,0);
	//progress bar
	glVertex3i(progress_left_x,progress_top_y,0);
	glVertex3i(progress_right_x,progress_top_y,0);
	glVertex3i(progress_left_x,progress_bot_y,0);
	glVertex3i(progress_right_x,progress_bot_y,0);
	glVertex3i(progress_left_x,progress_top_y,0);
	glVertex3i(progress_left_x,progress_bot_y,0);
	glVertex3i(progress_right_x,progress_top_y,0);
	glVertex3i(progress_right_x,progress_bot_y,0);
	glEnd();
	//progress bar
	if (is_researching)
	{
		int progress = (progress_right_x - progress_left_x - 1) * get_research_fraction();
		glBegin(GL_QUADS);
		glColor3f(0.40f,0.40f,1.00f);
		glVertex3i(progress_left_x+1+gx_adjust,progress_top_y+gy_adjust,0);
		glVertex3i(progress_left_x+1+progress+gx_adjust,progress_top_y+gy_adjust,0);
		glColor3f(0.10f,0.10f,0.80f);
		glVertex3i(progress_left_x+1+progress+gx_adjust,progress_bot_y-1+gy_adjust,0);
		glVertex3i(progress_left_x+1+gx_adjust,progress_bot_y-1+gy_adjust,0);
		glColor3f(0.77f,0.57f,0.39f);
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	//draw text
	if (selected_book >= 0 && knowledge_list[selected_book].present && knowledge_list[selected_book].has_book)
		text_width = book_start_x - 2 * text_border;
	put_small_text_in_box_zoomed((unsigned char *)raw_knowledge_string, strlen((char *)raw_knowledge_string), text_width, (char *)knowledge_text_buf, win->current_scale);
	draw_string_small_zoomed(text_border,booklist_y_len + text_border,(unsigned char*)knowledge_text_buf, info_lines, win->current_scale);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small_zoomed(text_border,progress_top_y+3+gy_adjust,(unsigned char*)researching_str,1, win->current_scale);
	draw_string_small_zoomed(text_border+(strlen(researching_str)+1)*win->small_font_len_x,progress_top_y+3+gy_adjust,(unsigned char*)research_string,1, win->current_scale);
	draw_string_small_zoomed(progress_left_x+points_pos+gx_adjust,progress_top_y+3+gy_adjust,(unsigned char*)points_string,1, win->current_scale);
	if (is_researching && mouse_over_progress_bar)
	{
		char eta_string[20];
		int eta_pos;
		get_research_eta_str(eta_string, sizeof(eta_string));
		eta_pos = (int)(progress_right_x - progress_left_x - strlen(eta_string)*win->small_font_len_x) / 2;
		draw_string_small_zoomed(progress_left_x + eta_pos, progress_top_y - win->small_font_len_y + 2, (unsigned char*)eta_string, 1, win->current_scale);
		mouse_over_progress_bar=0;
	}
	// Draw knowledges
	for(i = 2*scroll; y < (booklist_y_len - booklist_y_step); i++)
	{
		int highlight = 0;
		float colour_brightness = (knowledge_list[i].present) ?1.0 : 0.6;

		if (*highlight_string && (strlen(knowledge_list[i].name) > 0) &&
			(get_string_occurance(highlight_string, knowledge_list[i].name, strlen(knowledge_list[i].name), 1) != -1))
			highlight = 1;

		if (!highlight && (i == selected_book))
		{
			GLfloat cols[3];
			elglGetColour3v("global.mouseselected", cols);
			glColor3f (cols[0]*colour_brightness, cols[1]*colour_brightness, cols[2]*colour_brightness);
		}
		else if (knowledge_list[i].mouse_over)
			elglColourN("global.mousehighlight");
		else if (highlight)
			glColor3f (1.0f*colour_brightness, 0.6f*colour_brightness, 0.0f*colour_brightness);
		else
			glColor3f (1.0f*colour_brightness, 1.0f*colour_brightness, 1.0f*colour_brightness);

		/* truncate the string if it is too long */
		if (get_string_width_ui((unsigned char*)knowledge_list[i].name, font_ratio) > max_name_x)
		{
			const char *append_str = "... ";
			size_t dest_max_len = strlen(knowledge_list[i].name)+strlen(append_str)+1;
			char *used_name = (char *)malloc(dest_max_len);
			truncated_string(used_name, knowledge_list[i].name, dest_max_len, append_str,
				max_name_x, UI_FONT, font_ratio);
			draw_string_zoomed(x, y, (unsigned char*)used_name,1,font_ratio);
			/* if the mouse is over this line and its truncated, tooltip to full name */
			if (knowledge_list[i].mouse_over)
			{
				show_help(knowledge_list[i].name, -TAB_MARGIN, win->len_y+10+TAB_MARGIN, win->current_scale);
				know_show_win_help = 0;
			}
			free(used_name);
		}
		else
			draw_string_zoomed(x,y,(unsigned char*)knowledge_list[i].name,1,font_ratio);

		x += (win->len_x-win->box_size-2*text_border)/2;
		if (i % 2 == 1)
		{
			y += booklist_y_step;
			x = text_border;
		}
	}
	if (know_show_win_help)
	{
		show_help(cm_help_options_str, -TAB_MARGIN, win->len_y+10+TAB_MARGIN, win->current_scale);
		know_show_win_help = 0;
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int mouseover_knowledge_handler(window_info *win, int mx, int my)
{
	int	i;

	if (mx>=progress_left_x && mx<progress_right_x && my>progress_top_y && my<progress_bot_y)
		mouse_over_progress_bar=1;

	if (cm_window_shown()!=CM_INIT_VALUE)
		return 0;

	for(i=0;i<knowledge_count;i++)knowledge_list[i].mouse_over=0;
	if (my>0)
		know_show_win_help = 1;
	if(mx>win->len_x-win->box_size)
		return 0;
	if(my>booklist_y_len)
		return 0;
	mx = (mx < (win->len_x-win->box_size)/2) ?0 :1;
	my/=booklist_y_step;
	knowledge_list[mx+2*(my+vscrollbar_get_pos (win->window_id, knowledge_scroll_id))].mouse_over=1;
	return 0;
}


int click_knowledge_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y,idx;
	Uint8 str[3];

	x= mx;
	y= my;
	if(x > win->len_x-win->box_size)
		return 0;
	if(y > booklist_y_len)
		return 0;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(win->window_id, knowledge_scroll_id);
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(win->window_id, knowledge_scroll_id);
		return 1;
	} else {

		selected_book = -1;
		x = (x < (win->len_x-win->box_size)/2) ?0 :1;
		y/=booklist_y_step;
		idx = x + 2 *(y + vscrollbar_get_pos (win->window_id, knowledge_scroll_id));
		if(idx < knowledge_count)
			{
				str[0] = GET_KNOWLEDGE_INFO;
				*(Uint16 *)(str+1) = SDL_SwapLE16((short)idx);
				my_tcp_send(my_socket,str,3);
				raw_knowledge_string[0] = '\0';
				// Check if we display the book image and label
				knowledge_book_id = idx;
				if (knowledge_list[idx].present && knowledge_list[idx].has_book) {
					widget_unset_flags (win->window_id, knowledge_book_image_id, WIDGET_DISABLED);
					widget_unset_flags (win->window_id, knowledge_book_label_id, WIDGET_DISABLED);
				} else {
					widget_set_flags(win->window_id, knowledge_book_image_id, WIDGET_DISABLED);
					widget_set_flags(win->window_id, knowledge_book_label_id, WIDGET_DISABLED);
				}
				selected_book = idx;
			}
		do_click_sound();
	}
	return 1;
}


void get_knowledge_list (Uint16 size, const char *list)
{
	int i;

	// make sure the entire knowledge list is 0 incase of short data
	for(i=0; i<KNOWLEDGE_LIST_SIZE; i++){
		knowledge_list[i].present= 0;
	}

	// watch for size being too large
	if(size*8 > KNOWLEDGE_LIST_SIZE){
		size= KNOWLEDGE_LIST_SIZE/8;
	}

	// now copy the data
	for(i=0; i<size; i++)
	{
		knowledge_list[i*8+0].present= list[i] & 0x01;
		knowledge_list[i*8+1].present= list[i] & 0x02;
		knowledge_list[i*8+2].present= list[i] & 0x04;
		knowledge_list[i*8+3].present= list[i] & 0x08;
		knowledge_list[i*8+4].present= list[i] & 0x10;
		knowledge_list[i*8+5].present= list[i] & 0x20;
		knowledge_list[i*8+6].present= list[i] & 0x40;
		knowledge_list[i*8+7].present= list[i] & 0x80;
	}
}

void get_new_knowledge(Uint16 idx)
{
	if(idx < KNOWLEDGE_LIST_SIZE){
		knowledge_list[idx].present= 1;
	}
}

static void set_hightlight_callback(const char *new_highlight_string, void *data)
{
	safe_strncpy(highlight_string, new_highlight_string, KNOWLEDGE_NAME_SIZE);
}

static int cm_knowledge_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case 0:
			close_ipu(&ipu_know);
			init_ipu(&ipu_know, win->window_id, 21, 1, 22, NULL, set_hightlight_callback);
			ipu_know.x = mx; ipu_know.y = my;
			display_popup_win(&ipu_know, know_highlight_prompt_str);
			if (ipu_know.popup_win >=0 && ipu_know.popup_win<windows_list.num_windows)
				windows_list.window[ipu_know.popup_win].opaque = 1;
			break;
		case 1:
			set_hightlight_callback("", NULL);
			break;
		case 2:
			{
				int i;
				for(i=0; i<knowledge_count; i++)
					if (knowledge_list[i].mouse_over)
					{
						copy_to_clipboard(knowledge_list[i].name);
						break;
					}
			}
			break;
	}
	return 1;
}

static int resize_knowledge_handler(window_info *win, int new_width, int new_height)
{
	int gap_y;
	int image_size = (int)(0.5 + win->current_scale * 50);
	int label_width = 0;
	int label_height = 0;
	int book_x_off = 0;
	int label_x_left = 0;
	int book_x_left = 0;

	text_border = win->small_font_len_x / 2;
	booklist_y_step = win->small_font_len_y - 2;
	booklist_y_len = 2 * text_border + (int)(0.5 + booklist_y_step * displayed_book_rows);
	progressbox_y_len = booklist_y_len + 2 * text_border + (int)(0.5 + win->small_font_len_y * info_lines);

	gap_y = (win->len_y - progressbox_y_len - win->small_font_len_y - 4) / 2;
	progress_top_y = progressbox_y_len + gap_y;
	progress_bot_y = win->len_y - gap_y;
	progress_right_x = win->len_x - (int)(0.5 + win->current_scale * 15);
	progress_left_x = win->len_x - (int)(0.5 + win->current_scale * 140);

	widget_resize(win->window_id, knowledge_scroll_id, win->box_size, booklist_y_len);
	widget_move(win->window_id, knowledge_scroll_id, win->len_x - win->box_size, 0);

	widget_set_size(win->window_id, knowledge_book_label_id, win->current_scale * 0.8);
	widget_resize(win->window_id, knowledge_book_label_id, strlen(knowledge_read_book) * win->default_font_len_x * 0.8, win->default_font_len_y * 0.8);
	widget_resize(win->window_id, knowledge_book_image_id, image_size, image_size);

	label_width = widget_get_width(win->window_id, knowledge_book_label_id);
	label_height = widget_get_height(win->window_id, knowledge_book_label_id);
	book_x_off = (label_width > image_size) ?label_width :image_size;
	gap_y = booklist_y_len + (progressbox_y_len - booklist_y_len - image_size - label_height)/ 2;
	label_x_left = progress_right_x - book_x_off/2 - label_width/2;
	book_x_left = progress_right_x - book_x_off/2 - image_size/2;
	book_start_x = (label_x_left < book_x_left) ?label_x_left :book_x_left;

	widget_move(win->window_id, knowledge_book_label_id, label_x_left, gap_y);
	widget_move(win->window_id, knowledge_book_image_id, book_x_left, gap_y + label_height);


	return 0;
}

void fill_knowledge_win (int window_id)
{
	set_window_custom_scale(window_id, &custom_scale_factors.stats);
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_MOUSEOVER, &mouseover_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_knowledge_handler );

	knowledge_scroll_id = vscrollbar_add_extended (window_id, knowledge_scroll_id, NULL, 0,  0, 0, 0, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, (knowledge_count+1)/2-displayed_book_rows);
	knowledge_book_image_id = add_knowledge_book_image(window_id);
	widget_set_OnClick(window_id, knowledge_book_image_id, &handle_knowledge_book);
	knowledge_book_label_id = label_add_extended(window_id, knowledge_book_image_id + 1, NULL, 0, 0, WIDGET_DISABLED, 0.8, 1.0, 1.0, 1.0, knowledge_read_book);
	widget_set_OnClick(window_id, knowledge_book_label_id, &handle_knowledge_book);

	if (cm_valid(!cm_know_id))
	{
		cm_know_id = cm_create(know_highlight_cm_str, cm_knowledge_handler);
		cm_add_window(cm_know_id, window_id);
		init_ipu(&ipu_know, -1, 1, 1, 1, NULL, NULL);
	}
}

void set_knowledge_string(const Uint8 *in_data, int data_length)
{
	safe_strncpy2(raw_knowledge_string, (const char *)in_data, TEXTBUFSIZE, data_length);
}


void load_knowledge_list(void)
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];
	char *out;

	memset(knowledge_list, 0, sizeof(knowledge_list));
	i= 0;
	knowledge_count= 0;
	// try the language specific knowledge list
	f=open_file_lang("knowledge.lst", "rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"knowledge.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while(1)
		{
			if(!fgets(strLine, sizeof(strLine), f)) {
				break;
			}
			out = knowledge_list[i].name;
			my_xmlStrncopy(&out, strLine, sizeof(knowledge_list[i].name)-1);
			i++;
		}
	// memorize the count
	knowledge_count= i;
	// close the file
	fclose(f);
}
