#include <string.h>
#include "global.h"

#ifndef WINDOWS
#include <SDL_syswm.h>
#endif

void do_paste(Uint8 * buffer)
{
	Uint32 i;
	Uint8 ch;

	if(!buffer)return;
	for(i=0;i<strlen(buffer);i++)
		{
			ch=buffer[i];
        	if(((ch>=32 && ch<=126) || (ch>127+c_grey4)) && input_text_lenght<160)
        	    {
        	    	put_char_in_buffer(ch);
		    }
			if(input_text_lenght>=160)return;
		}
}

#ifndef WINDOWS

void startpaste() {
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

		/* selection=XA_PRIMARY; */
		selection=XInternAtom(dpy,"CLIPBOARD",0);
		property=XInternAtom(dpy,"PASTE",0);
		XConvertSelection(dpy,selection,XA_STRING,property,window,CurrentTime);

		wminfo.info.x11.unlock_func();
	}
}

void finishpaste(XSelectionEvent event) {
	Display * dpy;
	Window window;
	SDL_SysWMinfo wminfo;
	Atom type;
	int actualformat;
	unsigned long items;
	unsigned long bytes;
	unsigned char * value;

	SDL_VERSION(&wminfo.version);
	if(SDL_GetWMInfo(&wminfo) && wminfo.subsystem==SDL_SYSWM_X11) {
		wminfo.info.x11.lock_func();

		dpy=wminfo.info.x11.display;
		window=wminfo.info.x11.window;

		if(event.property == None) {
			fprintf(stderr,"%s\n",not_ascii);
			return;
		}
		XGetWindowProperty(dpy,window,event.property,0,256,1,XA_STRING,&type,&actualformat,&items, &bytes, &value);
		if(type == XA_STRING)
			{
				do_paste(value);
			}
		wminfo.info.x11.unlock_func();
	}
}

#else

void windows_paste()
{
	OpenClipboard(NULL);

	do_paste(GetClipboardData(CF_TEXT));

	CloseClipboard();
}


#endif


