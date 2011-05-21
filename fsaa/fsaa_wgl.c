#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/wglext.h>
#include "../platform.h"
#include "../errors.h"

static const int iattr[] = {
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
	WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
	WGL_SAMPLES_ARB, 2,
	0
};

unsigned int get_fsaa_modes()
{
	int formats[256];
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC _wglGetPixelFormatAttribivARB;
	PFNWGLGETEXTENSIONSSTRINGARBPROC _wglGetExtensionsStringARB;
	PFNWGLCHOOSEPIXELFORMATARBPROC _wglewChoosePixelFormatARB;
	PIXELFORMATDESCRIPTOR pfd;
	HINSTANCE hinst;
	WNDCLASS wndclass;
	LPCSTR str;
	HWND hwnd;
	HGLRC hrc, oldrc;
	HDC hdc, oldhdc;
	unsigned int i, result, count;
	unsigned int multisample, pixel_format;
	int query, samples, format;

	LOG_DEBUG("Using wgl to get fsaa modes");

	result = 1;

	str = "fsaa_dummy";

	hinst = GetModuleHandle(0);

	memset(&wndclass, 0, sizeof(WNDCLASS));

	wndclass.style = CS_OWNDC;
	wndclass.hInstance = hinst;
	wndclass.lpfnWndProc = DefWindowProc;
	wndclass.lpszClassName = str;

	RegisterClass(&wndclass);

	hwnd = CreateWindow(str, str, WS_POPUP | WS_CLIPCHILDREN, 0, 0, 32, 32, 0, 0, hinst, 0);

	if (hwnd == 0)
	{
		LOG_ERROR("Can't create dummy window");

		return result;
	}

	hdc = GetDC(hwnd); 

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 15;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;

	format = ChoosePixelFormat(hdc, &pfd);

	if (format != 0)
	{
		SetPixelFormat(hdc, format, &pfd);
	}

	hrc = wglCreateContext(hdc);

	multisample = 0;
	pixel_format = 0;

	if (hrc)
	{
		oldrc = wglGetCurrentContext();
		oldhdc = wglGetCurrentDC();

		wglMakeCurrent(hdc, hrc);

		_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");

		if (_wglGetExtensionsStringARB != 0)
		{
			const char* str = _wglGetExtensionsStringARB(hdc);

			if (strstr(str, "WGL_ARB_pixel_format") != 0)
			{
				pixel_format = 1;
			}

			if (strstr(str, "WGL_ARB_multisample") != 0)
			{
				multisample = 1;
			}
		}

		if ((pixel_format == 1) && (multisample == 1))
		{
			_wglewChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			_wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");

			if (_wglewChoosePixelFormatARB(hdc, iattr, 0, 256, formats, &count))
			{
				query = WGL_SAMPLES_ARB;

				for (i = 0; i < count; i++)
				{
					if (_wglGetPixelFormatAttribivARB(hdc, formats[i], 0, 1, &query, &samples))
					{
						result |= 1 << samples;
					}
				}
			}
		}

		wglMakeCurrent(oldhdc, oldrc);
		wglDeleteContext(hrc);
	}

	DestroyWindow(hwnd);
	UnregisterClass(str, hinst);

	return result;
}
