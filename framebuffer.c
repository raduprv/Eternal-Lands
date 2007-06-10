#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

void print_fbo_errors(const char *file, const char *func, int line)
{
	GLuint error_no = ELglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	switch (error_no)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			log_error_detailed(fbo_attachment_error, file, func, line);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			log_error_detailed(fbo_missing_attachment_error, file, func, line);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			log_error_detailed(fbo_dimensions_error, file, func, line);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			log_error_detailed(fbo_formats_error, file, func, line);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			log_error_detailed(fbo_draw_buffer_error, file, func, line);
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			log_error_detailed(fbo_read_buffer_error, file, func, line);
			break;
		default:
			log_error_detailed(fbo_unknow_error, file, func, line);
			break;
	}	
}

void free_color_framebuffer(GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture)
{
	if (FBO[0] > 0)
	{
		ELglDeleteFramebuffersEXT(1, FBO);
		FBO[0] = 0;
	}
	if (FBORenderBuffer[0] > 0)
	{
		ELglDeleteRenderbuffersEXT(1, FBORenderBuffer);
		FBORenderBuffer[0] = 0;
	}
	if (FBOTexture[0] > 0)
	{
		glDeleteTextures(1, FBOTexture);
		FBOTexture[0] = 0;
	}
}

void make_color_framebuffer(int width, int height, GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture)
{
	GLint depth_bits, depth_format;

	if ((width <= 0) || (height <= 0)) return;

	// create objects
	ELglGenFramebuffersEXT(1, FBO);// frame buffer
	ELglGenRenderbuffersEXT(1, FBORenderBuffer);// render buffer
	glGenTextures(1, FBOTexture);// texture

	glGetIntegerv(GL_DEPTH_BITS, &depth_bits);

	if (depth_bits == 16) depth_format = GL_DEPTH_COMPONENT16_ARB;
	else depth_format = GL_DEPTH_COMPONENT24_ARB;

	// initialize texture
	glBindTexture(GL_TEXTURE_2D, FBOTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO[0]);

	// attach texture to frame-buffer color-buffer
	ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, FBOTexture[0], 0);

	// attach render-buffer
	ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, FBORenderBuffer[0]);

	// setting size of the render-buffer
	ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA, width, height);
	ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depth_format, width, height);

	// attach render-buffer to frame-buffer depth-buffer
	ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, FBORenderBuffer[0]);

	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();

	//Turn off our frame buffer object
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
}

void change_color_framebuffer_size(int width, int height, GLuint *FBO, GLuint *FBORenderBuffer, GLuint *FBOTexture)
{
	free_color_framebuffer(FBO, FBORenderBuffer, FBOTexture);
	make_color_framebuffer(width, height, FBO, FBORenderBuffer, FBOTexture);
}

void free_depth_framebuffer(GLuint *FBO, GLuint *FBOTexture)
{
	if (FBO[0] > 0)
	{
		ELglDeleteFramebuffersEXT(1, FBO);
		FBO[0] = 0;
	}
	if (FBOTexture[0] > 0)
	{
		glDeleteTextures(1, FBOTexture);
		FBOTexture[0] = 0;
	}
}

void make_depth_framebuffer(int width, int height, GLuint *FBO, GLuint *FBOTexture)
{
	GLint depth_bits, depth_format;

	if ((width <= 0) || (height <= 0)) return;

	// create objects
	ELglGenFramebuffersEXT(1, FBO);// frame buffer
	glGenTextures(1, FBOTexture);// texture
	
	glGetIntegerv(GL_DEPTH_BITS, &depth_bits);

	if (depth_bits == 16) depth_format = GL_DEPTH_COMPONENT16_ARB;
	else depth_format = GL_DEPTH_COMPONENT24_ARB;

	glBindTexture(GL_TEXTURE_2D, FBOTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// initialize texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO[0]);

	// attach texture to frame-buffer depth-buffer
	ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, FBOTexture[0], 0);

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

void change_depth_framebuffer_size(int width, int height, GLuint *FBO, GLuint *FBOTexture)
{
	free_depth_framebuffer(FBO, FBOTexture);
	make_depth_framebuffer(width, height, FBO, FBOTexture);
}
