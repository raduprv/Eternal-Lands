#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <SDL_video.h>
#include "loading_win.h"
#include "draw_scene.h"
#include "elwindows.h"
#include "errors.h"
#include "font.h"
#include "gl_init.h"
#include "interface.h"
#include "multiplayer.h"
#include "textures.h"
#include "widgets.h"

#define PROGRESSBAR_ID       1

const float load_bar_colors[12] = {
/*
	// exp bar colors
	//red    green   blue
	0.100f, 0.800f, 0.100f, // topleft
	0.100f, 0.800f, 0.100f, // topright
	0.100f, 0.400f, 0.100f, // bottomright
	0.100f, 0.400f, 0.100f  // bottomleft
*/
	// Roja's colors
	//red    green   blue
	0.086f, 0.659f, 0.988f, // topleft
	0.086f, 0.659f, 0.988f, // topright
	0.294f, 0.173f, 0.690f, // bottomright
	0.294f, 0.173f, 0.690f  // bottomleft
};

int loading_win = -1;
static Uint32 loading_win_progress_bar = -1;
static float total_progress = 0;
static GLuint loading_texture = 0;
static float frac_x, frac_y;
static Uint32 loading_texture_handle;
static Uint32 use_snapshot = 0;
static unsigned char text_buffer[255] = {0};
static char version_str[250] = {0};
static int progressbar_len = 0;
static int progressbar_height = 0;

static int display_loading_win_handler(window_info *win)
{
	if (use_snapshot == 0)
	{
		bind_texture(loading_texture_handle);

		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3i(0, 0, 0);

		glTexCoord2f(0.0f, frac_y);
		glVertex3i(0, win->len_y, 0);

	 	glTexCoord2f(frac_x, frac_y);
		glVertex3i(win->len_x, win->len_y, 0);

		glTexCoord2f(frac_x, 0.0f);
		glVertex3i(win->len_x, 0, 0);
		glEnd();
	}
	else
	{
		bind_texture_id(loading_texture);

		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2f (0.0f, frac_y);
		glVertex3i(0, 0, 0);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3i(0, win->len_y, 0);

		glTexCoord2f(frac_x, 0.0f);
		glVertex3i(win->len_x, win->len_y, 0);

	 	glTexCoord2f(frac_x, frac_y);
		glVertex3i(win->len_x, 0, 0);
		glEnd();
	}

	// Since the background doesn't use the texture cache, invalidate
	// the last texture, so that the font will be loaded
	last_texture = -1;
	glColor3f (1.0, 1.0, 1.0);
	draw_string_zoomed_centered(win->len_x / 2, (win->len_y * 2) / 3 - win->default_font_len_y - 2,
		(const unsigned char*)version_str, 1, win->current_scale);
	draw_string_small_zoomed_centered(win->len_x / 2, (win->len_y * 2) / 3 + progressbar_height + 2,
		text_buffer, 1, win->current_scale);

	glDisable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static void take_snapshot (int width, int height)
{
	int bg_width = 1024;
	int bg_height = 512;

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	glGenTextures (1, &loading_texture);
	glBindTexture (GL_TEXTURE_2D, loading_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// texture sizes need to be powers of 2
	while (bg_width < width)
		bg_width *= 2;
	while (bg_height < height)
		bg_height *= 2;

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	// Copy the current screen to the texture
	glReadBuffer(GL_BACK);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	if (glGetError() != GL_NO_ERROR)
	{
		LOG_ERROR("%s: %d glReadBuffer(GL_BACK) problem.\n", __FUNCTION__, __LINE__);
		glReadBuffer(GL_FRONT);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bg_width, bg_height, 0, GL_RGBA, GL_BYTE, NULL);
	if (glIsTexture(loading_texture) == GL_FALSE)
		LOG_ERROR("%s: %d texture problem.\n", __FUNCTION__, __LINE__);
	else
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, bg_width, bg_height);

	frac_x = ((float) width) / bg_width;
	frac_y = ((float) height) / bg_height;

	use_snapshot = 1;

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int create_loading_win (int width, int height, int snapshot)
{
	version_str[0] = '\0';
	if (snapshot)
	{
		take_snapshot (width, height);
	}

	if (loading_win == -1)// Make sure we only have one loading window
	{
		window_info * win = NULL;
		loading_win = create_window("Loading window", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW);
		set_window_handler(loading_win, ELW_HANDLER_DISPLAY, &display_loading_win_handler);
		if (loading_win >= 0 && loading_win < windows_list.num_windows)
			win = &windows_list.window[loading_win];
		else
			return -1;
		progressbar_len = (int)(0.5 + win->current_scale * 300);
		progressbar_height = (int)(0.5 + win->current_scale * 20);
		loading_win_progress_bar = progressbar_add_extended(loading_win, PROGRESSBAR_ID, NULL, (width - progressbar_len)/2, (height*2)/3,
				progressbar_len, progressbar_height, 0, 1.0f, 0.0f, load_bar_colors);
		widget_set_color(loading_win, loading_win_progress_bar, 0.0f, 0.0f, 0.0f);
		if (!snapshot)
		{
			loading_texture_handle = load_texture_cached("./textures/login_back", tt_image);
			frac_x = frac_y = 1.0f;
			use_snapshot = 0;

			get_version_string (version_str, sizeof (version_str));
		}
	}

	return loading_win;
}

void update_loading_win (char *text, float progress_increase)
{
	if(loading_win != -1) {
		total_progress += progress_increase;
		LOG_DEBUG("%s (%.0f%%)", text, total_progress);
		if (total_progress > 100.1f)
		{
			LOG_ERROR("Loading window progress > 100%%! (%g)", total_progress);
			total_progress = 100.0f;
		}

		progressbar_set_progress(loading_win, loading_win_progress_bar, total_progress);

		if (text != NULL && strlen(text) <= 255)
		{
			if (loading_win >= 0 && loading_win < windows_list.num_windows)
			{
				put_small_text_in_box_zoomed((unsigned char*)text, strlen(text),
					window_width, text_buffer, windows_list.window[loading_win].current_scale);
			}
		}
		// The loading window is supposed to display stuff while
		// loading maps when the draw_scene loop is held up. Hence
		// we have to call our own drawing code. Instead of making
		// sure that the proper root window is hidden, we call
		// display_window directly.
		glLoadIdentity ();
		Enter2DMode ();
		display_window (loading_win);
		Leave2DMode ();
		SDL_GL_SwapWindow(el_gl_window);
	}
}

int destroy_loading_win(void)
{
	if (use_snapshot != 0)
	{
		glDeleteTextures (1, &loading_texture);
	}
	destroy_window(loading_win);
	loading_win = -1;
	loading_texture = -1;
	loading_win_progress_bar = -1;
	total_progress = 0.0f;

	return 0;
}
