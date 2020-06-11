#include <GL/glx.h>
#include <X11/extensions/Xrandr.h>
#include "../platform.h"
#include "../errors.h"

static Display* get_gl_display()
{
	Display* gl_display = NULL;

#ifdef GLX_VERSION_1_2
	gl_display = glXGetCurrentDisplay();
#endif

	if (gl_display == NULL)
	{
		gl_display = XOpenDisplay(0);
	}

	return gl_display;
}

unsigned int get_fsaa_modes()
{
	PFNGLXCHOOSEFBCONFIGPROC _glXChooseFBConfig;
	PFNGLXGETFBCONFIGATTRIBPROC _glXGetFBConfigAttrib;
	PFNGLXCHOOSEFBCONFIGSGIXPROC _glXChooseFBConfigSGIX;
	PFNGLXGETFBCONFIGATTRIBSGIXPROC _glXGetFBConfigAttribSGIX;
	GLXFBConfig *configs;
	Display* display;
	int i, count, caveat, samples;
	unsigned int result;

	LOG_DEBUG("Using glx to get fsaa modes");

	_glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddressARB((const GLubyte*)"glXChooseFBConfig");
	_glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)glXGetProcAddressARB((const GLubyte*)"glXGetFBConfigAttrib");

	if ((_glXChooseFBConfig == 0) || (_glXGetFBConfigAttrib == 0))
	{
		_glXChooseFBConfigSGIX = (PFNGLXCHOOSEFBCONFIGSGIXPROC)glXGetProcAddressARB((const GLubyte*)"glXChooseFBConfigSGIX");
		_glXGetFBConfigAttribSGIX = (PFNGLXGETFBCONFIGATTRIBSGIXPROC)glXGetProcAddressARB((const GLubyte*)"glXGetFBConfigAttribSGIX");

		if ((_glXChooseFBConfigSGIX == 0) || (_glXGetFBConfigAttribSGIX == 0))
		{
			return 0;
		}
	}
	else
	{
		_glXChooseFBConfigSGIX = 0;
		_glXGetFBConfigAttribSGIX = 0;
	}

	count = 0;
	result = 1;
	if ((display = get_gl_display()) == NULL)
		return result;

	if (_glXChooseFBConfig != 0)
	{
		configs = _glXChooseFBConfig(display, DefaultScreen(display), NULL, &count);
	}
	else
	{
		configs = _glXChooseFBConfigSGIX(display, DefaultScreen(display), NULL, &count);
	}

	for (i = 0; i < count; i++)
	{
		if (_glXGetFBConfigAttrib != 0)
		{
			_glXGetFBConfigAttrib(display, configs[i], GLX_CONFIG_CAVEAT, &caveat);
		}
		else
		{
			_glXGetFBConfigAttribSGIX(display, configs[i], GLX_CONFIG_CAVEAT, &caveat);
		}

		if (caveat != GLX_SLOW_CONFIG)
		{
			if (_glXGetFBConfigAttrib != 0)
			{
				_glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLES, &samples);
			}
			else
			{
				_glXGetFBConfigAttribSGIX(display, configs[i], GLX_SAMPLES, &samples);
			}

			result |= 1 << samples;
		}
	}

	XFree(configs);

	return result;
}
