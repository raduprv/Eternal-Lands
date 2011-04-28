#include "framebuffer.h"
#include "errors.h"
#include "gl_init.h"
#include "load_gl_extensions.h"
#include "translate.h"

static const GLenum stencil_formats[] =
{
	GL_NONE,
	GL_STENCIL_INDEX1_EXT,
	GL_STENCIL_INDEX4_EXT,
	GL_STENCIL_INDEX8_EXT,
	GL_STENCIL_INDEX16_EXT
};

static const size_t stencil_bits[] =
{
	0, 1, 4, 8, 16
};

#define STENCIL_FORMAT_COUNT (sizeof(stencil_formats) / sizeof(GLenum))

static const GLenum depth_formats[] =
{
	GL_NONE,
	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
	GL_DEPTH_COMPONENT32
};

static const size_t depth_bits[] =
{
	0, 16, 24, 32
};

#define DEPTH_FORMAT_COUNT (sizeof(depth_formats) / sizeof(GLenum))

static const GLenum color_formats[] =
{
	GL_NONE,
	GL_RGBA4,
	GL_RGB8,
	GL_RGB5,
	GL_RGBA8,
	GL_RGB5_A1,
	GL_LUMINANCE8,
	GL_LUMINANCE8_ALPHA8,
	GL_LUMINANCE16,
	GL_LUMINANCE16_ALPHA16,
#ifndef OSX
	GL_RGB32F_ARB,
	GL_RGBA32F_ARB,
#endif
	GL_LUMINANCE32F_ARB,
	GL_LUMINANCE_ALPHA32F_ARB,
#ifndef OSX
	GL_RGB16F_ARB,
	GL_RGBA16F_ARB,
#endif
	GL_LUMINANCE16F_ARB,
	GL_LUMINANCE_ALPHA16F_ARB
};

#define COLOR_FORMAT_COUNT (sizeof(color_formats) / sizeof(GLenum))

static const char* color_format_strs[] =
{
	"NONE",
	"RGBA4",
	"RGB8",
	"RGB5",
	"RGBA8",
	"RGB5_A1",
	"LUMINANCE8",
	"LUMINANCE8_ALPHA8",
	"LUMINANCE16",
	"LUMINANCE16_ALPHA16",
#ifndef OSX
	"RGB32F",
	"RGBA32F",
#endif
	"LUMINANCE32F",
	"LUMINANCE_ALPHA32F",
#ifndef OSX
	"RGB16F",
	"RGBA16F",
#endif
	"LUMINANCE16F",
	"LUMINANCE_ALPHA16F"
};

#define PROBE_SIZE 16

int try_format(GLenum depth_format, GLenum stencil_format)
{
	GLuint status, depth_rb, stencil_rb;
	int failed; // flag on GL errors

	depth_rb = 0;
	stencil_rb = 0;
	failed = 0; // flag on GL errors

	if (depth_format != GL_NONE)
	{
		/// Generate depth renderbuffer
		ELglGenRenderbuffersEXT(1, &depth_rb);
		/// Bind it to FBO
		ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_rb);
		
		/// Allocate storage for depth buffer
		ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depth_format, PROBE_SIZE,
			PROBE_SIZE);
		
		/// Attach depth
		ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, depth_rb);
	}

	if (stencil_format != GL_NONE)
	{
		/// Generate stencil renderbuffer
		ELglGenRenderbuffersEXT(1, &stencil_rb);
		/// Bind it to FBO
		ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencil_rb);
		glGetError(); // NV hack
		/// Allocate storage for stencil buffer
		ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, stencil_format, PROBE_SIZE,
			PROBE_SIZE); 

		if (glGetError() != GL_NO_ERROR) // NV hack
		{
			failed = 1;
		}
		/// Attach stencil
		ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, stencil_rb);
		if (glGetError() != GL_NO_ERROR) // NV hack
		{
			failed = 1;
		}
	}

	status = ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	/// If status is negative, clean up
	// Detach and destroy
	ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_RENDERBUFFER_EXT, 0);
	ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
		GL_RENDERBUFFER_EXT, 0);
	if (depth_rb != 0)
	{
		ELglDeleteRenderbuffersEXT(1, &depth_rb);
	}
	if (stencil_rb != 0)
	{
		ELglDeleteRenderbuffersEXT(1, &stencil_rb);
	}
	
	return (status == GL_FRAMEBUFFER_COMPLETE_EXT) && (failed == 0);
}

void check_fbo_formats()
{
	// Try all formats, and report which ones work as target
	int i, j, k;
	GLuint fb, tid;
       	GLint old_drawbuffer, old_readbuffer;
	GLenum target, fmt;

	target = GL_TEXTURE_2D;

	glGetIntegerv (GL_DRAW_BUFFER, &old_drawbuffer);
	glGetIntegerv (GL_READ_BUFFER, &old_readbuffer);

	LOG_INFO("Checking supported framebuffe formats....");
	for (i = 0; i < COLOR_FORMAT_COUNT; i++)
	{
		GLuint status;
		fmt = color_formats[i];
		ELglGenFramebuffersEXT(1, &fb);
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		if (fmt != GL_NONE)
		{
			// Create and attach texture
			glGenTextures(1, &tid);
			glBindTexture(target, tid);
				
			// Set some default parameters so it won't fail on NVidia cards
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				    
			glTexImage2D(target, 0, fmt, PROBE_SIZE, PROBE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, tid, 0);
		}
		else
		{
			// Draw to nowhere -- stencil/depth only
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		// Check status
		status = ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

		// Ignore status in case of fmt == GL_NONE, because no implementation will accept
		// a buffer without *any* attachment. Buffers with only stencil and depth attachment
		// might still be supported, so we must continue probing.
		if (fmt == GL_NONE || status == GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			for (j = 0; j < DEPTH_FORMAT_COUNT; j++)
			{
				for (k = 0; k < STENCIL_FORMAT_COUNT; k++)
				{
					if (try_format(depth_formats[j], stencil_formats[k]))
					{
						LOG_INFO(fbo_supported_format, color_format_strs[i], depth_bits[j], stencil_bits[k]);
					}
				}
			}
		}
		ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		ELglDeleteFramebuffersEXT(1, &fb);
		if (fmt != GL_NONE)
		{
			glDeleteTextures(1, &tid);
		}
	}
	glDrawBuffer(old_drawbuffer);
	glReadBuffer(old_readbuffer);
}

void print_fbo_errors(const char *file, int line)
{
	GLuint error_no = ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	CHECK_GL_ERRORS();

	switch (error_no)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			log_error(file, line, fbo_attachment_error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			log_error(file, line, fbo_missing_attachment_error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			log_error(file, line, fbo_dimensions_error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			log_error(file, line, fbo_formats_error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			log_error(file, line, fbo_draw_buffer_error);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			log_error(file, line, fbo_read_buffer_error);
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			log_error(file, line, fbo_unsupported_fromat_error);
			break;
		default:
			log_error(file, line, fbo_unknown_error, error_no);
			break;
	}	
}

void free_color_framebuffer(GLuint *fbo, GLuint *fbo_depth_buffer, GLuint * fbo_stencil_buffer,
	GLuint *fbo_texture)
{
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo != NULL)
	{
		if (*fbo != 0)
		{
			ELglDeleteFramebuffersEXT(1, fbo);
		}
		*fbo = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo_depth_buffer != NULL)
	{
		if (*fbo_depth_buffer != 0)
		{
			ELglDeleteRenderbuffersEXT(1, fbo_depth_buffer);
		}
		*fbo_depth_buffer = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo_stencil_buffer != NULL)
	{
		if (*fbo_stencil_buffer != 0)
		{
			ELglDeleteRenderbuffersEXT(1, fbo_stencil_buffer);
		}
		*fbo_stencil_buffer = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo_texture != NULL)
	{
		if (*fbo_texture != 0)
		{
			glDeleteTextures(1, fbo_texture);
		}
		*fbo_texture = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
}

void make_color_framebuffer(int width, int height, GLuint *fbo, GLuint *fbo_depth_buffer,
	GLuint * fbo_stencil_buffer, GLuint *fbo_texture)
{
	if ((width <= 0) || (height <= 0)) return;

	glGenTextures(1, fbo_texture);
	ELglGenFramebuffersEXT(1, fbo);

	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);

	// initialize color texture
	glBindTexture(GL_TEXTURE_2D, *fbo_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,
		*fbo_texture, 0);

	// initialize depth renderbuffer
	if (fbo_depth_buffer != NULL)
	{
		ELglGenRenderbuffersEXT(1, fbo_depth_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, *fbo_depth_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, *fbo_depth_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}

	// initialize stencil renderbuffer
	if (fbo_stencil_buffer != NULL)
	{
		ELglGenRenderbuffersEXT(1, fbo_stencil_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, *fbo_stencil_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX8_EXT, width, height);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
		ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT, *fbo_stencil_buffer);
		CHECK_GL_ERRORS();
		CHECK_FBO_ERRORS();
	}

	//Turn off our frame buffer object
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
}

void change_color_framebuffer_size(int width, int height, GLuint *fbo, GLuint *fbo_depth_buffer,
	GLuint *fbo_stencil_buffer, GLuint *fbo_texture)
{
	free_color_framebuffer(fbo, fbo_depth_buffer, fbo_stencil_buffer, fbo_texture);
	make_color_framebuffer(width, height, fbo, fbo_depth_buffer, fbo_stencil_buffer, fbo_texture);
}

void free_depth_framebuffer(GLuint *fbo, GLuint *fbo_texture)
{
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo != NULL)
	{
		if (*fbo != 0)
		{
			ELglDeleteFramebuffersEXT(1, fbo);
		}
		*fbo = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	if (fbo_texture != NULL)
	{
		if (*fbo_texture != 0)
		{
			glDeleteTextures(1, fbo_texture);
		}
		*fbo_texture = 0;
	}
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
}

void make_depth_framebuffer(int width, int height, GLuint *fbo, GLuint *fbo_texture)
{
	GLint depth_bits;

	if ((width <= 0) || (height <= 0)) return;

	// create objects
	ELglGenFramebuffersEXT(1, fbo);// frame buffer
	glGenTextures(1, fbo_texture);// texture
	
	glGetIntegerv(GL_DEPTH_BITS, &depth_bits);

	glBindTexture(GL_TEXTURE_2D, *fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// initialize texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);

	// attach texture to frame-buffer depth-buffer
	ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, *fbo_texture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();

	//Turn off our frame buffer object
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
}

void change_depth_framebuffer_size(int width, int height, GLuint *fbo, GLuint *fbo_texture)
{
	free_depth_framebuffer(fbo, fbo_texture);
	make_depth_framebuffer(width, height, fbo, fbo_texture);
}
