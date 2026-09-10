#include "gamma_postprocess.h"
#include "gl_init.h"
#include "errors.h"

int hardware_gamma_available = 1;
int software_gamma_active = 0;

void free_gamma_postprocess(void)
{
	software_gamma_active = 0;
}

void init_gamma_postprocess(void)
{
	/* only a fallback for when the platform can't do this itself */
	if (hardware_gamma_available || disable_gamma_adjust)
		return;

	software_gamma_active = 1;
	LOG_DEBUG("Hardware gamma control is unavailable on this system - "
		"using a software brightness overlay instead.");
}

void gamma_postprocess_apply(void)
{
	float amount;
	GLfloat saved_projection[16];
	GLfloat saved_modelview[16];
	GLint saved_matrix_mode;

	if (!software_gamma_active || disable_gamma_adjust)
		return;
	if (gamma_var == 1.0f)
		return;

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glViewport(0, 0, window_width, window_height);

	/* Save/restore the matrix *contents* by value instead of using
	 * glPushMatrix()/glPopMatrix(): resize_root_window() only rebuilds the
	 * 3D projection matrix when the zoom level changes, not every frame, so
	 * whatever is left in GL_PROJECTION/GL_MODELVIEW after this function
	 * returns is exactly what next frame's terrain rendering starts from.
	 * glPushMatrix() here (even paired with a matching glPopMatrix()) was
	 * confirmed by bisection to corrupt that on at least one Mesa/radeonsi
	 * driver, so we capture and reload the values explicitly instead. */
	glGetIntegerv(GL_MATRIX_MODE, &saved_matrix_mode);
	glGetFloatv(GL_PROJECTION_MATRIX, saved_projection);
	glGetFloatv(GL_MODELVIEW_MATRIX, saved_modelview);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (gamma_var > 1.0f)
	{
		/* additive white overlay - gamma_var tops out at 3.0 */
		amount = (gamma_var - 1.0f) / 2.0f;
		if (amount > 1.0f)
			amount = 1.0f;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glColor4f(1.0f, 1.0f, 1.0f, amount);
	}
	else
	{
		/* alpha-blended black overlay - gamma_var bottoms out at 0.1 */
		amount = (1.0f - gamma_var) / 0.9f;
		if (amount > 1.0f)
			amount = 1.0f;
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, amount);
	}

	glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(1.0f, 0.0f);
		glVertex2f(1.0f, 1.0f);
		glVertex2f(0.0f, 1.0f);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(saved_projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(saved_modelview);
	glMatrixMode((GLenum)saved_matrix_mode);

	glPopAttrib();

	CHECK_GL_ERRORS();
}
