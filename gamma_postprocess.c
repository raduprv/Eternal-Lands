#include <string.h>
#include "gamma_postprocess.h"
#include "gl_init.h"
#include "framebuffer.h"
#include "errors.h"
#include "shader/shader.h"

int hardware_gamma_available = 1;
int software_gamma_active = 0;

static GLuint gamma_fbo = 0;
static GLuint gamma_fbo_depth_buffer = 0;
static GLuint gamma_fbo_texture = 0;
static GLhandleARB gamma_shader = 0;
static GLint gamma_uniform_tex = -1;
static GLint gamma_uniform_gamma = -1;
static int fbo_width = 0;
static int fbo_height = 0;

/* No #version directive -> GLSL 1.10. Only a fragment shader is attached,
 * so the fixed function vertex pipeline still feeds gl_TexCoord[0] and
 * gl_Position from the glBegin()/glEnd() quad drawn in
 * gamma_postprocess_end_frame() - this is the standard way to add a single
 * shader stage to an otherwise fixed function GL1/2 renderer. */
static const char gamma_fragment_shader_src[] =
	"uniform sampler2D tex;\n"
	"uniform float gamma;\n"
	"void main()\n"
	"{\n"
	"	vec3 color = texture2D(tex, gl_TexCoord[0].st).rgb;\n"
	"	gl_FragColor = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / gamma)), 1.0);\n"
	"}\n";

static GLhandleARB build_gamma_shader(void)
{
	GLint ret;
	GLhandleARB program, fragment_shader;
	const char *source = gamma_fragment_shader_src;
	GLint length = (GLint)strlen(gamma_fragment_shader_src);

	program = ELglCreateProgramObjectARB();
	fragment_shader = ELglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	ELglShaderSourceARB(fragment_shader, 1, &source, &length);
	ELglCompileShaderARB(fragment_shader);
	ELglGetObjectParameterivARB(fragment_shader, GL_OBJECT_COMPILE_STATUS_ARB, &ret);
	if (ret != 1)
	{
		LOG_ERROR("Compiling the software gamma correction shader failed.");
		ELglDeleteObjectARB(fragment_shader);
		ELglDeleteObjectARB(program);
		return 0;
	}

	ELglAttachObjectARB(program, fragment_shader);
	ELglDeleteObjectARB(fragment_shader);

	ELglLinkProgramARB(program);
	ELglGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &ret);
	if (ret != 1)
	{
		LOG_ERROR("Linking the software gamma correction shader failed.");
		ELglDeleteObjectARB(program);
		return 0;
	}

	return program;
}

void free_gamma_postprocess(void)
{
	if (gamma_shader != 0)
	{
		ELglDeleteObjectARB(gamma_shader);
		gamma_shader = 0;
	}
	free_color_framebuffer(&gamma_fbo, &gamma_fbo_depth_buffer, NULL, &gamma_fbo_texture);
	gamma_uniform_tex = -1;
	gamma_uniform_gamma = -1;
	fbo_width = 0;
	fbo_height = 0;
	software_gamma_active = 0;
}

/* Only a depth renderbuffer is attached (no stencil): a separate,
 * non-packed depth+stencil renderbuffer pair reports
 * GL_FRAMEBUFFER_UNSUPPORTED_EXT on at least some Mesa/AMD drivers. Stencil
 * shadows won't be visible while this fallback is active, which is an
 * acceptable trade-off for a fallback path that only exists because the
 * platform can't do gamma any other way. */
static int make_gamma_framebuffer(int width, int height)
{
	GLenum status;

	make_color_framebuffer(width, height, &gamma_fbo, &gamma_fbo_depth_buffer, NULL,
		&gamma_fbo_texture);
	if ((gamma_fbo == 0) || (gamma_fbo_texture == 0))
		return 0;

	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gamma_fbo);
	status = ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		LOG_ERROR("Software gamma correction frame buffer is incomplete (0x%x).",
			(unsigned int)status);
		return 0;
	}

	return 1;
}

void init_gamma_postprocess(void)
{
	/* only a fallback for when the platform can't do this itself */
	if (hardware_gamma_available || disable_gamma_adjust)
		return;

	if (!is_shader_supported() || !have_extension(ext_framebuffer_object))
	{
		LOG_DEBUG("Hardware gamma control is unavailable and shaders/FBOs "
			"aren't supported - the gamma slider will have no effect.");
		return;
	}

	if ((window_width <= 0) || (window_height <= 0))
		return;

	gamma_shader = build_gamma_shader();
	if (gamma_shader == 0)
		return;

	if (!make_gamma_framebuffer(window_width, window_height))
	{
		free_gamma_postprocess();
		return;
	}
	fbo_width = window_width;
	fbo_height = window_height;

	gamma_uniform_tex = ELglGetUniformLocationARB(gamma_shader, "tex");
	gamma_uniform_gamma = ELglGetUniformLocationARB(gamma_shader, "gamma");

	software_gamma_active = 1;
	LOG_DEBUG("Hardware gamma control is unavailable on this system - "
		"using a software gamma correction pass instead.");
}

static void resize_gamma_postprocess(int width, int height)
{
	if ((width == fbo_width) && (height == fbo_height))
		return;
	if ((width <= 0) || (height <= 0))
		return;

	if (!make_gamma_framebuffer(width, height))
	{
		LOG_ERROR("Failed to resize the software gamma correction frame buffer.");
		free_gamma_postprocess();
		return;
	}
	fbo_width = width;
	fbo_height = height;
}

int gamma_postprocess_begin_frame(void)
{
	if (!software_gamma_active || disable_gamma_adjust)
		return 0;

	resize_gamma_postprocess(window_width, window_height);
	if (!software_gamma_active)
		return 0;

	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gamma_fbo);
	return 1;
}

void gamma_postprocess_end_frame(void)
{
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);

	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	ELglActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gamma_fbo_texture);

	ELglUseProgramObjectARB(gamma_shader);
	if (gamma_uniform_tex != -1)
		ELglUniform1iARB(gamma_uniform_tex, 0);
	if (gamma_uniform_gamma != -1)
		ELglUniform1fARB(gamma_uniform_gamma, gamma_var);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
	glEnd();

	ELglUseProgramObjectARB(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();

	CHECK_GL_ERRORS();
}
