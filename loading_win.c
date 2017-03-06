#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <SDL_video.h>
#include "loading_win.h"
#include "console.h"
#include "draw_scene.h"
#include "elwindows.h"
#include "errors.h"
#include "font.h"
#include "gl_init.h"
#include "interface.h"
#include "textures.h"
#include "widgets.h"

#define PROGRESSBAR_LEN    300
#define PROGRESSBAR_HEIGHT  20
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

Uint32 loading_win = -1;
Uint32 loading_win_progress_bar = -1;
static float total_progress = 0;
GLuint loading_texture = 0;
float frac_x, frac_y;
#ifdef	NEW_TEXTURES
Uint32 loading_texture_handle;
Uint32 use_snapshot = 0;
#else	/* NEW_TEXTURES */
int delete_texture = 0;
#endif	/* NEW_TEXTURES */

unsigned char text_buffer[255] = {0};

char version_str[250] = {0};
int version_width;

int display_loading_win_handler(window_info *win)
{
#ifdef	NEW_TEXTURES
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
#else	/* NEW_TEXTURES */
	glBindTexture (GL_TEXTURE_2D, loading_texture);

	glEnable (GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glTexCoord2f (0.0f, frac_y);
	glVertex3i (0, 0, 0);

	glTexCoord2f (0.0f, 0.0f);
	glVertex3i (0, win->len_y, 0);

	glTexCoord2f (frac_x, 0.0f);
	glVertex3i (win->len_x, win->len_y, 0);

 	glTexCoord2f (frac_x, frac_y);
	glVertex3i (win->len_x, 0, 0);
	glEnd();
#endif	/* NEW_TEXTURES */

	// Since the background doesn't use the texture cache, invalidate
	// the last texture, so that the font will be loaded
	last_texture = -1;
	glColor3f (1.0, 1.0, 1.0);
	draw_string ((win->len_x - version_width) / 2, (win->len_y * 2) / 3 - 20, (unsigned char*)version_str, 1);
	draw_string_small((win->len_x - (get_string_width(text_buffer)*SMALL_FONT_X_LEN)/12)/2, (win->len_y*2)/3 + PROGRESSBAR_HEIGHT + 2, text_buffer, 1);

	glDisable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

void take_snapshot (int width, int height)
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

#ifdef	NEW_TEXTURES
	use_snapshot = 1;
#else	/* NEW_TEXTURES */
	delete_texture = 1;
#endif	/* NEW_TEXTURES */

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
		loading_win = create_window("Loading window", -1, -1, 0, 0, width, height, ELW_TITLE_NONE|ELW_SHOW);
		set_window_handler(loading_win, ELW_HANDLER_DISPLAY, &display_loading_win_handler);
		loading_win_progress_bar = progressbar_add_extended(loading_win, PROGRESSBAR_ID, NULL, (width - PROGRESSBAR_LEN)/2, (height*2)/3,
				PROGRESSBAR_LEN, PROGRESSBAR_HEIGHT, 0, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, load_bar_colors);
		if (!snapshot)
		{
#ifdef	NEW_TEXTURES
			loading_texture_handle = load_texture_cached("./textures/login_back", tt_image);
			frac_x = frac_y = 1.0f;
			use_snapshot = 0;
#else	/* NEW_TEXTURES */
			int idx = load_texture_cache ("./textures/login_back.bmp", 255);
			loading_texture = get_texture_id (idx);
			frac_x = frac_y = 1.0f;
			delete_texture = 0;
#endif	/* NEW_TEXTURES */

			print_version_string (version_str, sizeof (version_str));
			version_width = (get_string_width ((unsigned char*)version_str) * DEFAULT_FONT_X_LEN) / 12;
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
			put_small_text_in_box((unsigned char*)text, strlen(text), window_width, (char*)text_buffer);
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
		SDL_GL_SwapBuffers ();
	}
}

int destroy_loading_win(void)
{
#ifdef	NEW_TEXTURES
	if (use_snapshot != 0)
	{
		glDeleteTextures (1, &loading_texture);
	}
#else	/* NEW_TEXTURES */
	if (delete_texture)
		glDeleteTextures (1, &loading_texture);
#endif	/* NEW_TEXTURES */
	destroy_window(loading_win);
	loading_win = -1;
	loading_texture = -1;
	loading_win_progress_bar = -1;
	total_progress = 0.0f;

	return 0;
}
