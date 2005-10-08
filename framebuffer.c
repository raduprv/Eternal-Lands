#ifdef	USE_FRAMEBUFFER
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

void free_color_framebuffer(int *FBO, int *FBORenderBuffer, int *FBOTexture)
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
		FBOTexture = 0;
	}
}

void make_color_framebuffer(int width, int height, int *FBO, int *FBORenderBuffer, int *FBOTexture)
{
	if ((width <= 0) || (height <= 0)) return;

	// create objects
	ELglGenFramebuffersEXT(1, FBO);// frame buffer
	ELglGenRenderbuffersEXT(1, FBORenderBuffer);// render buffer
	glGenTextures(1, FBOTexture);// texture
	
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO[0]);
	
	// initialize texture
	glBindTexture(GL_TEXTURE_2D, FBOTexture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// attach texture to framebuffercolor buffer
	ELglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, FBOTexture[0], 0);
	ELglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, FBORenderBuffer[0]);
	ELglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
	
	// attach renderbufferto framebufferdepth buffer
	ELglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, FBORenderBuffer[0]);
	
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void change_color_framebuffer_size(int width, int height, int *FBO, int *FBORenderBuffer, int *FBOTexture)
{
	free_color_framebuffer(FBO, FBORenderBuffer, FBOTexture);
	make_color_framebuffer(width, height, FBO, FBORenderBuffer, FBOTexture);
}
#endif
