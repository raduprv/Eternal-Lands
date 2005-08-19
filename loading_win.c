#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "elwindows.h"
#include "widgets.h"
#include "global.h"

#define PROGRESSBAR_LEN 300
#define PROGRESSBAR_HEIGHT 30

Uint32 loading_win = -1;
Uint32 loading_win_progress_bar = -1;
int loading_texture = -1;
static float total_progress = 0;

unsigned char text_buffer[255] = {0};

int display_loading_win_handler(window_info *win)
{
	draw_console_pic(loading_texture);
	glColor3f (1.0, 1.0, 1.0);
	draw_string_small(win->len_x/2-(get_string_width(text_buffer)*0.7)/2, (win->len_y/3)*2+PROGRESSBAR_HEIGHT+2, text_buffer, 1);

	return 1;
}

int create_loading_win(int width, int height)
{
	if(loading_win == -1) { //Make sure we only have one loading window
		loading_win = create_window("Loading window", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW);
		set_window_handler(loading_win, ELW_HANDLER_DISPLAY, &display_loading_win_handler);
		loading_win_progress_bar = progressbar_add(loading_win, NULL, width/2-PROGRESSBAR_LEN/2, (height/3)*2, PROGRESSBAR_LEN, PROGRESSBAR_HEIGHT);
		loading_texture = load_texture_cache("./textures/login_back.bmp",255);
	}

	return loading_win;
}

void update_loading_win(char *text, float progress_increase)
{
	if(loading_win != -1) {
		total_progress += progress_increase;
		if(total_progress > 100) {
			fprintf(stderr, "Loading window progress > 100%%! (%g)\n", total_progress);
		} else {
			progressbar_set_progress(loading_win, loading_win_progress_bar, total_progress);
		}
		if(text != NULL && strlen(text) <= 255) {
			put_small_text_in_box(text, strlen(text), PROGRESSBAR_LEN, text_buffer);
		}
		draw_scene();
	}
}

int destroy_loading_win(void)
{
	update_loading_win("", 0);
	destroy_window(loading_win);
	loading_win = -1;
	loading_texture = -1;
	loading_win_progress_bar = -1;
	total_progress = 0.0f;

	return 0;
}
