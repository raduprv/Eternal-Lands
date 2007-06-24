#include <stdlib.h>
#include <string.h>
#include "global.h"

#ifndef WINDOWS
 #include <SDL_syswm.h>
#endif

char* cur_text = NULL;
text_field* cur_text_field = NULL;

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
	unsigned long bytes, tmp;
	unsigned char * value = NULL;

	XGetWindowProperty(dpy, window, atom, 0, 0, 0, XA_STRING, &type, &actualformat, &items, &bytes, &value);
	XFree(value);
	// XXX Grum: according to my man page, parameter long_length is
	// the length in 32-bit multiples of the data to be retrieved. So can
	// bytes be divided by 4 in the call below?
	XGetWindowProperty(dpy, window, atom, 0, bytes, 1, XA_STRING, &type, &actualformat, &items, &tmp, &value);
	if(type == XA_STRING)
	{
		int p;
		if (cur_text_field == NULL)
		{
			do_paste(value); // copy to input line
		}
		else
		{
			// XXX FIXME: We should probably check if the text field
			// is editable, and the data buffer reallocatable.
			text_message* msg = &cur_text_field->buffer[cur_text_field->msg];
			int min_size = msg->len + bytes + 1;

			while (!value[bytes - 1]) bytes--;
			if (msg->size < min_size)
			{
				int new_size = msg->size;
				while (new_size < min_size)
					new_size *= 2;
				msg->data = (char*) realloc(msg->data, new_size);
				msg->size = new_size;
			}
			p = cur_text_field->cursor;
			memmove(&msg->data[p + bytes], &msg->data[p], msg->len - p + 1);
			memcpy(&msg->data[p], value, bytes);
			msg->len += bytes;
			cur_text_field->cursor += bytes;
			text_field_find_cursor_line(cur_text_field);
			cur_text_field = NULL;
		}
	}
	/* XGetWindowProperty allocated this, so we have to free it */
	if(value)
	{
		XFree(value);
	}
}

void start_paste_to_text_field(text_field* tf)
{
	cur_text_field = tf;
	startpaste();
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
		// Alia: we should receive SelectionNotify event, property is NULL until it comes.
		// Let's try to comment it :)
		// //if(use_clipboard) {
		//	processpaste(dpy, window, property);
		//}
		wminfo.info.x11.unlock_func();
	}
}

void copy_to_clipboard(const char* text)
{
	Display* dpy;
	Window window;
	SDL_SysWMinfo wminfo;
	Atom selection;

	if (cur_text != NULL) free(cur_text);
	cur_text = strdup(text);
	SDL_VERSION(&wminfo.version);
	if (SDL_GetWMInfo(&wminfo) && (wminfo.subsystem == SDL_SYSWM_X11))
	{
		wminfo.info.x11.lock_func();

		dpy = wminfo.info.x11.display;
		window = wminfo.info.x11.window;

		if(use_clipboard)
		{
			selection=XInternAtom(dpy,"CLIPBOARD",0);
		}
		else
		{
			selection=XA_PRIMARY;
		}
		
		//property = XInternAtom(dpy, "PASTE", 0);
		XSetSelectionOwner(dpy, selection, window, CurrentTime);
		wminfo.info.x11.unlock_func();
	}

}

void process_copy(XSelectionRequestEvent* e)
{
	XEvent r;

	if (e->target != XA_STRING)
	{
		r.xselection.property = None;
	}
	else
	{
		XChangeProperty(e->display, e->requestor, e->property, XA_STRING, 8, PropModeReplace, (unsigned char *)cur_text, strlen(cur_text) + 1);
		r.xselection.property = e->property;
	}
	r.xselection.type = SelectionNotify;
	r.xselection.display = e->display;
	r.xselection.requestor = e->requestor;
	r.xselection.selection =e->selection;
	r.xselection.target= e->target;
	r.xselection.time = e->time;
	XSendEvent (e->display, e->requestor, 0, 0, &r);
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
