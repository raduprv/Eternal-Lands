#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "knowledge.h"
#include "asc.h"
#include "books.h"
#include "context_menu.h"
#include "cursors.h"
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
static unsigned char raw_knowledge_string[TEXTBUFSIZE]="";
static int rewrap_knowledge_now = 0;

static size_t cm_know_id = CM_INIT_VALUE;
static INPUT_POPUP ipu_know;
static char highlight_string[KNOWLEDGE_NAME_SIZE] = "";
static int know_show_win_help = 0;
static int mouse_over_progress_bar = 0;
static int selected_book = -1;
static int book_clicked = 0;

static const int displayed_book_rows = 16;
static const int info_lines = 5;
static int booklist_y_len = 0;
static int booklist_y_step = 0;
static int progressbox_y_len = 0;
static int progress_top_y = 0;
static int progress_bot_y = 0;
static int progress_right_x = 0;
static int progress_left_x = 0;
static int progress_text_y_offset = 0;
static int text_border = 0;
static int label_x_left = 0;
static int book_x_left = 0;
static int book_start_x = 0;
static int mouse_over_book_link = 0;

static int add_knowledge_book_image(int window_id)
{
	// Book image
	int isize, tsize, tid, picsperrow, xtile, ytile, id, img_id;
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
	img_id = image_add_extended(window_id, 0, NULL, 0, 0, 0, 0, WIDGET_DISABLED, 1.0, id, u, v, uend, vend, 0.05f);
	widget_set_color(window_id, img_id, 1.0f, 1.0f, 1.0f);
	return img_id;
}

static int handle_knowledge_book(void)
{
	open_book(knowledge_book_id + 10000);
	book_clicked = 1;
	return 1;
}

static int handle_mouseover_knowledge_book(widget_list *widget, int mx, int my)
{
	mouse_over_book_link = 1;
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
static int waiting_for_true_knowledge_info = 0;

//	Some books have total research points > 2^16 and so the value sent with
//	HERE_YOUR_STATS can get wrapped. Instead of using the value provided, get
//	it from text output for the "#research" server command.  We can't tell
//	from the numbers which books need this so do it for all.
//
void request_true_knowledge_info(void)
{
	if (your_info.researching < KNOWLEDGE_LIST_SIZE)
	{
		//printf("Sending #research\n");
		send_input_text_line("#research", 9);
		waiting_for_true_knowledge_info = 1;
	}
	else
	{
		//printf("NOT sending #research\n");
		update_research_rate();
	}
}

//	Parse the response to #reseach to get the true research values for
//	completed and total.  Sucess or now, now do the update_research_rate()
//	call.
int get_true_knowledge_info(const char *message)
{
	const char *needle = NULL, *start = NULL;
	Uint32 total, left = 0;

	// it was just a user requested #research to ignore it
	if (!waiting_for_true_knowledge_info)
		return 0;

	waiting_for_true_knowledge_info = 0;
	needle = "Research points left:";

	// if a valid message, get the completed and total from the server message
	if (((start = strstr(message, needle)) != NULL) && ((sscanf(start + strlen(needle), "%u/%u.", &left, &total) == 2)))
	{
		your_info.research_total = total;
		your_info.research_completed = total - left;
		//printf("Got True research info id=%u complete=%u total=%u\n[%s]\n",
		//	your_info.researching, your_info.research_completed, your_info.research_total, message);
	}
	//else
		//printf("Error reading #research output, not using: research info id=%u complete=%u total=%u\n[%s]\n",
		//	your_info.researching, your_info.research_completed, your_info.research_total, message);

	update_research_rate();
	return 1;
}


void update_research_rate(void)
{
	static Sint32 last_research_completed = -1;
	//printf("Updating research rate id=%u last=%d complete=%u total=%u\n", your_info.researching, last_research_completed, your_info.research_completed, your_info.research_total);
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
	float eta = 0.0f;
	if (your_info.research_total && (your_info.research_completed == your_info.research_total))
	{
		safe_snprintf(str, size, completed_research);
		return str;
	}
	if ((eta = get_research_eta()) < 1)
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
	unsigned char buf[128];
	size_t i;
	int x = text_border;
	int y = text_border;
	int scroll = vscrollbar_get_pos (win->window_id, knowledge_scroll_id);
	char points_string[16];
	char *research_string;
	float font_ratio = win->current_scale_small;
	float max_name_x = (win->len_x - win->box_size - 2*x)/2;
	int is_researching = 1;

	if(your_info.research_total &&
	   (your_info.research_completed==your_info.research_total))
		safe_snprintf(points_string, sizeof(points_string), "%s", completed_research);
	else
		safe_snprintf(points_string, sizeof(points_string), "%u/%u",your_info.research_completed,your_info.research_total);
	if(your_info.researching < knowledge_count)
	{
		research_string = knowledge_list[your_info.researching].name;
	}
	else if (your_info.researching < KNOWLEDGE_LIST_SIZE)
	{
		research_string = unknown_book_short_str;
	}
	else
	{
		research_string = not_researching_anything;
		points_string[0] = '\0';
		is_researching = 0;
	}

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	// window separators
	glVertex3i(0,booklist_y_len,0);
	glVertex3i(win->len_x,booklist_y_len,0);
	glVertex3i(0,progressbox_y_len,0);
	glVertex3i(win->len_x,progressbox_y_len,0);
	glEnd();
	//progress bar
	if (is_researching)
	{
		int progress = (progress_right_x - progress_left_x - 1) * get_research_fraction();
		glBegin(GL_QUADS);
		glColor3f(0.40f,0.40f,1.00f);
		glVertex3i(progress_left_x, progress_top_y, 0);
		glVertex3i(progress_left_x + progress,progress_top_y, 0);
		glColor3f(0.10f,0.10f,0.80f);
		glVertex3i(progress_left_x + progress,progress_bot_y, 0);
		glVertex3i(progress_left_x, progress_bot_y, 0);
		glColor3f(0.77f,0.57f,0.39f);
		glEnd();
	}
	//progress bar
	glBegin(GL_LINES);
	glVertex3i(progress_left_x, progress_top_y, 0);
	glVertex3i(progress_right_x, progress_top_y, 0);
	glVertex3i(progress_left_x, progress_bot_y, 0);
	glVertex3i(progress_right_x, progress_bot_y, 0);
	glVertex3i(progress_left_x, progress_top_y, 0);
	glVertex3i(progress_left_x, progress_bot_y, 0);
	glVertex3i(progress_right_x, progress_top_y, 0);
	glVertex3i(progress_right_x, progress_bot_y, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	//draw text
	if (rewrap_knowledge_now)
	{
		int text_width = win->len_x - 2 * text_border;
		if (selected_book >= 0 && knowledge_list[selected_book].present && knowledge_list[selected_book].has_book)
			text_width = book_start_x - 2 * text_border;
		reset_soft_breaks(raw_knowledge_string, strlen((const char*)raw_knowledge_string),
			sizeof(raw_knowledge_string), win->font_category, win->current_scale_small,
			text_width, NULL, NULL);
		rewrap_knowledge_now = 0;
	}
	draw_string_small_zoomed(text_border, booklist_y_len + text_border,
		raw_knowledge_string, info_lines, win->current_scale);
	glColor3f(1.0f,1.0f,1.0f);
	safe_snprintf((char *)buf, sizeof(buf), "%s %s", researching_str, research_string);
	draw_string_small_zoomed(text_border, progress_top_y + progress_text_y_offset, buf, 1, win->current_scale);
	if (*points_string)
		draw_string_small_zoomed_centered((progress_left_x + progress_right_x) / 2,
			progress_top_y + progress_text_y_offset, (const unsigned char*)points_string,
			1, win->current_scale);
	if (is_researching && mouse_over_progress_bar)
	{
		char eta_string[20];
		get_research_eta_str(eta_string, sizeof(eta_string));
		draw_string_small_zoomed_centered((progress_left_x + progress_right_x) / 2,
			progress_top_y - win->small_font_len_y, (const unsigned char*)eta_string,
			1, win->current_scale);
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
		draw_string_zoomed_ellipsis_font(x, y, (const unsigned char*)knowledge_list[i].name,
			max_name_x, 1, win->font_category, font_ratio);
		if (get_string_width_zoom((const unsigned char*)knowledge_list[i].name, win->font_category, font_ratio) > max_name_x)
		{
			/* if the mouse is over this line and its truncated, tooltip to full name */
			if (knowledge_list[i].mouse_over)
			{
				show_help(knowledge_list[i].name, -TAB_MARGIN, win->len_y+10+TAB_MARGIN, win->current_scale);
				know_show_win_help = 0;
			}
		}

		if (i % 2 == 0)
		{
			x += (win->len_x-win->box_size-2*text_border)/2;
		}
		else
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

	if (mouse_over_book_link)
	{
		elwin_mouse = CURSOR_USE;
		mouse_over_book_link = 0;
		return 1;
	}

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

const char *get_knowledge_state_tag(size_t index)
{
	if (index < KNOWLEDGE_LIST_SIZE)
	{
		if (knowledge_list[index].present > 0)
			return knowledge_read_book_tag;
		if ((your_info.researching < KNOWLEDGE_LIST_SIZE) && (your_info.researching == index))
			return knowledge_reading_book_tag;
		return knowledge_unread_book_tag;
	}
	else
		return "";
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

static void set_content_widths(window_info *win)
{
	int image_size = (int)(0.5 + win->current_scale * 50);
	int label_width = get_string_width_zoom((const unsigned char*)knowledge_read_book,
		win->font_category, win->current_scale * 0.8);

	int i, gap_y, book_x_off, max_width = 0;

	text_border = win->small_font_max_len_x / 2;
	booklist_y_step = win->small_font_len_y - 2;

	booklist_y_len = 2 * text_border + (int)(0.5 + booklist_y_step * displayed_book_rows);
	progressbox_y_len = booklist_y_len + 2 * text_border + win->small_font_len_y * info_lines;

	gap_y = (win->len_y - progressbox_y_len - win->small_font_len_y - 4) / 2;
	progress_top_y = progressbox_y_len + gap_y;
	progress_bot_y = win->len_y - gap_y;
	progress_right_x = win->len_x - (int)(0.5 + win->current_scale * 15);
	progress_left_x = win->len_x - (int)(0.5 + win->current_scale * 140);
	progress_text_y_offset = (progress_bot_y - progress_top_y - win->small_font_len_y) / 2;

	book_x_off = max2i(label_width, image_size);
	label_x_left = progress_right_x - book_x_off/2 - label_width/2;
	book_x_left = progress_right_x - book_x_off/2 - image_size/2;
	book_start_x = min2i(label_x_left, book_x_left);

	for (i = 0; i < knowledge_count; ++i)
	{
		int width = get_string_width_zoom((const unsigned char*)knowledge_list[i].name,
			win->font_category, win->current_scale_small);
		if (width > max_width)
			max_width = width;
	}

	win->min_len_x = 2 * text_border + 2 * max_width + 2 * win->small_font_max_len_x;
	win->min_len_y = progressbox_y_len + 2*text_border + win->small_font_len_y;
}

static int resize_knowledge_handler(window_info *win, int new_width, int new_height)
{
	int image_size = (int)(0.5 + win->current_scale * 50);
	int label_width = get_string_width_zoom((const unsigned char*)knowledge_read_book,
		win->font_category, win->current_scale * 0.8);
	int label_height = (int)(0.5 + win->default_font_len_y * 0.8);
	int gap_y;

	set_content_widths(win);

	widget_resize(win->window_id, knowledge_scroll_id, win->box_size, booklist_y_len);
	widget_move(win->window_id, knowledge_scroll_id, win->len_x - win->box_size, 0);

	widget_set_size(win->window_id, knowledge_book_label_id, win->current_scale * 0.8);
	widget_resize(win->window_id, knowledge_book_label_id, label_width, label_height);
	widget_resize(win->window_id, knowledge_book_image_id, image_size, image_size);

	gap_y = booklist_y_len + (progressbox_y_len - booklist_y_len - image_size - label_height)/ 2;
	widget_move(win->window_id, knowledge_book_label_id, label_x_left, gap_y);
	widget_move(win->window_id, knowledge_book_image_id, book_x_left, gap_y + label_height);

	rewrap_knowledge_now = 1;

	return 0;
}

static int ui_scale_knowledge_handler(window_info *win)
{
	set_content_widths(win);
	return 1;
}

static int change_knowledge_font_handler(window_info *win, font_cat font)
{
	if (font != UI_FONT)
		return 0;
	set_content_widths(win);
	return 1;
}

// Handler to bring the book window to the front if the book icon has been
// clicked. We can't do this from click handler itself, as the windows handling
// code selects this (the knowledge) window after it has successfully handled
// the click. So we set a flag in the click handler, and check it after each
// draw.
static int post_display_knowledge_handler(window_info *win)
{
	if (book_clicked)
	{
		select_book_window();
		book_clicked = 0;
	}
	return 1;
}

void fill_knowledge_win(int window_id)
{
	set_window_custom_scale(window_id, MW_STATS);
	set_window_handler(window_id, ELW_HANDLER_DISPLAY, &display_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_CLICK, &click_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_MOUSEOVER, &mouseover_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_RESIZE, &resize_knowledge_handler );
	set_window_handler(window_id, ELW_HANDLER_UI_SCALE, &ui_scale_knowledge_handler);
	set_window_handler(window_id, ELW_HANDLER_FONT_CHANGE, &change_knowledge_font_handler);
	set_window_handler(window_id, ELW_HANDLER_POST_DISPLAY, &post_display_knowledge_handler);

	knowledge_scroll_id = vscrollbar_add_extended (window_id, knowledge_scroll_id, NULL, 0,  0, 0, 0, 0, 1.0, 0, 1, (knowledge_count+1)/2-displayed_book_rows);
	knowledge_book_image_id = add_knowledge_book_image(window_id);
	widget_set_OnClick(window_id, knowledge_book_image_id, &handle_knowledge_book);
	widget_set_OnMouseover(window_id, knowledge_book_image_id, &handle_mouseover_knowledge_book);
	knowledge_book_label_id = label_add_extended(window_id, knowledge_book_image_id + 1, NULL, 0, 0, WIDGET_DISABLED, 0.8, knowledge_read_book);
	widget_set_color(window_id, knowledge_book_label_id, 1.0, 1.0, 1.0);
	widget_set_OnClick(window_id, knowledge_book_label_id, &handle_knowledge_book);
	widget_set_OnMouseover(window_id, knowledge_book_label_id, &handle_mouseover_knowledge_book);

	if (window_id >= 0 && window_id < windows_list.num_windows)
		set_content_widths(&windows_list.window[window_id]);

	if (cm_valid(!cm_know_id))
	{
		cm_know_id = cm_create(know_highlight_cm_str, cm_knowledge_handler);
		cm_add_window(cm_know_id, window_id);
		init_ipu(&ipu_know, -1, 1, 1, 1, NULL, NULL);
	}
}

void set_knowledge_string(const Uint8 *in_data, int data_length)
{
	safe_strncpy2((char *)raw_knowledge_string, (const char *)in_data, TEXTBUFSIZE, data_length);
	rewrap_knowledge_now = 1;
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
