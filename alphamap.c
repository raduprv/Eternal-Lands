#include <stdlib.h>
#include "alphamap.h"
#include "textures.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

int use_alpha_border = 1;
int alpha_border_tex = 0;

void draw_window_alphaborder(window_info *win) {
	int w = win->len_x, h= win->len_y; // width, height

	if (!alpha_border_tex) {
		alpha_border_tex = load_texture_cached("textures/alphaborder.dds", tt_image);
		if (!alpha_border_tex) return;
	}
	
	glColor3fv(win->back_color);
	glBegin(GL_LINE_LOOP);
	glVertex3i(0, 0, 0);
	glVertex3i(w, 0, 0);
	glVertex3i(w, h, 0);
	glVertex3i(0, h, 0);
	glEnd();

	// The higher the alpha value (a=r+g+b/3) in the border texture,
	// the darker the shadow
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	bind_texture(alpha_border_tex);

	glBegin(GL_QUADS);

	/*
	 * 
	 *       r===============================i
	 *     11|                               |99
	 *    111|                               |999
	 *    111|                               |999
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222|                               |888
	 *    222'-------------------------------'888
	 *    333444555555555555555555555555555666777
	 *    333444555555555555555555555555555666777
	 *     3344455555555555555555555555555566677
	 *        4455555555555555555555555555566 
	 */

	glTexCoord2f(1.0f, 1.0f); glVertex2i( - 8,    0);
	glTexCoord2f(1.0f, 0.5f); glVertex2i(   0,    0);
	glTexCoord2f(0.5f, 0.5f); glVertex2i(   0,   16);
	glTexCoord2f(0.5f, 1.0f); glVertex2i( - 8,   16);

	glTexCoord2f(0.5f, 1.0f); glVertex2i( - 8,   16);
	glTexCoord2f(0.5f, 0.5f); glVertex2i(   0,   16);
	glTexCoord2f(0.1f, 0.5f); glVertex2i(   0, h   );
	glTexCoord2f(0.1f, 1.0f); glVertex2i( - 8, h   );

	glTexCoord2f(0.5f, 1.0f); glVertex2i( - 8, h   );
	glTexCoord2f(0.5f, 0.5f); glVertex2i(   0, h   );
	glTexCoord2f(1.0f, 0.5f); glVertex2i(   0, h+16);
	glTexCoord2f(1.0f, 1.0f); glVertex2i( - 8, h+16);

	glTexCoord2f(0.5f, 0.5f); glVertex2i(   0, h   );
	glTexCoord2f(0.5f, 0.0f); glVertex2i(   8, h   );
	glTexCoord2f(1.0f, 0.0f); glVertex2i(   8, h+16);
	glTexCoord2f(1.0f, 0.5f); glVertex2i(   0, h+16);

	glTexCoord2f(0.1f, 0.0f); glVertex2i(   8, h   );
	glTexCoord2f(0.5f, 0.0f); glVertex2i(w- 8, h   );
	glTexCoord2f(0.5f, 1.0f); glVertex2i(w- 8, h+16);
	glTexCoord2f(0.1f, 1.0f); glVertex2i(   8, h+16);

	glTexCoord2f(0.5f, 0.0f); glVertex2i(w- 8, h   );
	glTexCoord2f(0.5f, 0.5f); glVertex2i(w   , h   );
	glTexCoord2f(1.0f, 0.5f); glVertex2i(w   , h+16);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(w- 8, h+16);

	glTexCoord2f(0.5f, 0.5f); glVertex2i(w   , h   );
	glTexCoord2f(0.5f, 1.0f); glVertex2i(w+ 8, h   );
	glTexCoord2f(1.0f, 1.0f); glVertex2i(w+ 8, h+16);
	glTexCoord2f(1.0f, 0.5f); glVertex2i(w   , h+16);

	glTexCoord2f(0.1f, 0.5f); glVertex2i(w   ,   16);
	glTexCoord2f(0.1f, 1.0f); glVertex2i(w+ 8,   16);
	glTexCoord2f(0.5f, 1.0f); glVertex2i(w+ 8, h   );
	glTexCoord2f(0.5f, 0.5f); glVertex2i(w   , h   );

	glTexCoord2f(1.0f, 0.5f); glVertex2i(w   ,    0);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(w+ 8,    0);
	glTexCoord2f(0.5f, 1.0f); glVertex2i(w+ 8,   16);
	glTexCoord2f(0.5f, 0.5f); glVertex2i(w   ,   16);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
