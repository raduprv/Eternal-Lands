#include <stdlib.h>
#include <string.h>
#include "global.h"

#ifndef WINDOWS
 #include <SDL_syswm.h>
#endif

void do_paste(const Uint8 * buffer)
{
	paste_in_input_field (buffer);
}

#ifdef OSX
void startpaste () 
{
	// Todo, actually fill these!
}

void finishpaste (void* event) 
{
	// Todo. This :-)
}
#else
 #ifndef WINDOWS

int use_clipboard = 1;

void processpaste(Display *dpy, Window window, Atom atom)
{
	Atom type;
	int actualformat;
	unsigned long items;
	unsigned long bytes;
	unsigned char * value = NULL;

	XGetWindowProperty(dpy,window,atom,0,256,1,XA_STRING,&type,&actualformat,&items, &bytes, &value);
	if(type == XA_STRING) {
		do_paste(value);
	}
	/* XGetWindowProperty allocated this, so we have to free it */
	if(value) {
		XFree(value);
	}
}

void startpaste(void)
{
	Display * dpy;
	Window window;
	SDL_SysWMinfo wminfo;
	Atom selection;
	Atom property;

	SDL_VERSION(&wminfo.version);
	if(SDL_GetWMInfo(&wminfo) && wminfo.subsystem==SDL_SYSWM_X11) {
		wminfo.info.x11.lock_func();

		dpy=wminfo.info.x11.display;
		window=wminfo.info.x11.window;

		/* Set selection to XA_PRIMARY to use xterm-style select,
		 * or CLIPBOARD to use Gnome-style cut'n'paste.
		 * KDE3 should use CLIPBOARD also, I'm not sure about older
		 * KDE versions */

		if(use_clipboard) {
			selection=XInternAtom(dpy,"CLIPBOARD",0);
		} else {
			selection=XA_PRIMARY;
		}
		property=XInternAtom(dpy,"PASTE",0);
		XConvertSelection(dpy,selection,XA_STRING,property,window,CurrentTime);
		/* - If we used the CLIPBOARD, we don't get a SelectionNotify
		 *   event, so we have to call processpaste immediately.
		 * - If we used XA_PRIMARY, the selection-holder will send us a
		 *   SelectionNotify-event, as soon as the selection is
		 *   available for us. Then finishpaste calls processpaste
		 */
		if(use_clipboard) {
			processpaste(dpy, window, property);
		}
		wminfo.info.x11.unlock_func();
	}
}

void finishpaste(XSelectionEvent event)
{
	Display * dpy;
	Window window;
	SDL_SysWMinfo wminfo;

	SDL_VERSION(&wminfo.version);
	if(SDL_GetWMInfo(&wminfo) && wminfo.subsystem==SDL_SYSWM_X11) {
		wminfo.info.x11.lock_func();

		dpy=wminfo.info.x11.display;
		window=wminfo.info.x11.window;

		if(event.property == None) {
			fprintf(stderr,"%s\n",not_ascii);
			return;
		}
		processpaste(dpy, window, event.property);
		wminfo.info.x11.unlock_func();
	}
}

 #else

void windows_paste(void)
{
	OpenClipboard(NULL);

	do_paste(GetClipboardData(CF_TEXT));

	CloseClipboard();
}

 #endif //ndef WINDOWS
#endif //OSX
