#include <stdlib.h>
#include <string.h>
#include "global.h"

#include <SDL_syswm.h>

char* cur_text = NULL;
text_field* cur_text_field = NULL;

void do_paste (const Uint8* buffer)
{
	paste_in_input_field (buffer);
}

void do_paste_to_text_field (text_field *tf, const char* text)
{
	// XXX FIXME: We should probably check if the text field
	// is editable, and the data buffer reallocatable.
	int bytes = strlen (text);
	text_message* msg = &tf->buffer[tf->msg];
	int min_size, p;

	while (bytes > 0 && !text[bytes - 1])
		bytes--;
	min_size = msg->len + bytes + 1;
	if (msg->size < min_size)
	{
		int new_size = msg->size;
		while (new_size < min_size)
			new_size *= 2;
		msg->data = realloc (msg->data, new_size);
		msg->size = new_size;
	}
	p = tf->cursor;
	memmove (&msg->data[p + bytes], &msg->data[p], msg->len - p + 1);
	memcpy (&msg->data[p], text, bytes);
	msg->len += bytes;
	tf->cursor += bytes;
	text_field_find_cursor_line (tf);
}

void start_paste_to_text_field (text_field *tf)
{
	cur_text_field = tf;
	startpaste ();
}

#if defined OSX

void startpaste () 
{
	// Todo, actually fill these!
}

void copy_to_clipboard (const char* text)
{
	// Todo
}

#elif defined WINDOWS

void startpaste ()
{
	if (OpenClipboard (NULL))
	{
		HANDLE hText = GetClipboardData (CF_TEXT);
		char* text = GlobalLock (hText);
		if (cur_text_field == NULL)
		{
			do_paste (text);
		}
		else
		{
			do_paste_to_text_field (cur_text_field, text);
			cur_text_field = NULL;
		}
		GlobalUnlock (hText);
		CloseClipboard ();
	}
}

void copy_to_clipboard (const char* text)
{
	SDL_SysWMInfo info;

	if (text == NULL)
		return;

	SDL_VERSION (&info.version);
	if (SDL_GetWMInfo (&info))
	{
		if (OpenClipboard (info.window))
		{
			HGLOBAL hCopy = GlobalAlloc (GMEM_MOVEABLE, 1+strlen (text));
			char* copy = GlobalLock (hCopy);
			strcpy (copy, text);
			GlobalUnlock (hCopy);

			EmptyClipboard ();
			SetClipboardData (CF_TEXT, hCopy);
			CloseClipboard ();
		}
	}	
}

#else

int use_clipboard = 1;
Atom targets_atom = None;

void processpaste(Display *dpy, Window window, Atom atom)
{
	Atom type;
	int actualformat;
	unsigned long items;
	unsigned long bytes, tmp;
	unsigned char * value = NULL;

	XGetWindowProperty(dpy, window, atom, 0, 0, 0, XA_STRING, &type, &actualformat, &items, &bytes, &value);
	XFree(value);
	// From the XGetWindowProperty man page:
	// *) The length parameter is in 32 bit units, so we can divide
	//    bytes by four (rounding up
	// *) It always allocates one extra byte and sets it to zero, so
	//    using value as a zero-terminated string should be safe
	XGetWindowProperty(dpy, window, atom, 0, (bytes+3)/4, 1, XA_STRING, &type, &actualformat, &items, &tmp, &value);
	if(type == XA_STRING)
	{
		if (cur_text_field == NULL)
		{
			do_paste(value); // copy to input line
		}
		else
		{
			do_paste_to_text_field (cur_text_field, (const char*) value);
			cur_text_field = NULL;
		}
	}
	/* XGetWindowProperty allocated this, so we have to free it */
	if(value)
	{
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

		if (targets_atom == None)
			targets_atom = XInternAtom (dpy, "TARGETS", False);

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
	Atom targets[] = {
		targets_atom,
		XA_STRING
	};
	
	if (e->target == XA_STRING)
	{
		// Copy the string
		XChangeProperty(e->display, e->requestor, e->property, XA_STRING, 8, PropModeReplace, (unsigned char *)cur_text, strlen(cur_text));
		r.xselection.property = e->property;
	}
	else if (targets_atom != None && e->target == targets_atom)
	{
		// Tell X we have a string available
		XChangeProperty (e->display, e->requestor, e->property, XA_ATOM, 32, PropModeReplace, (unsigned char*) targets, sizeof(targets) / sizeof (targets[0]));
		r.xselection.property = e->property;
	}
	else
	{
		// No idea what X is requesting
		r.xselection.property = None;
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

#endif // def OSX / def WINDOWS / other
