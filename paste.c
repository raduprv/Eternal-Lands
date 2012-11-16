#include <stdlib.h>
#include <string.h>
#include <SDL_syswm.h>
#include "paste.h"
#include "chat.h"
#include "translate.h"

void do_paste(const Uint8* buffer)
{
	paste_in_input_field(buffer);
}

void do_paste_to_text_field(widget_list *widget, const char* text)
{
	text_field *tf;
	int bytes = strlen(text);
	text_message* msg;
	int p;

	if ((widget == NULL) || (widget->widget_info == NULL))
		return;

	// if not editable, don't allow paste
	if (!(widget->Flags & TEXT_FIELD_EDITABLE))
		return;

	tf = (text_field *) widget->widget_info;
	msg = &tf->buffer[tf->msg];

	// if can't grow and would over fill, just use what we can
	if ((msg->len + bytes >= msg->size) && !(widget->Flags & TEXT_FIELD_CAN_GROW))
		bytes = msg->size - msg->len - 1;

	resize_text_message_data (msg, msg->len + bytes);

	p = tf->cursor;
	memmove (&msg->data[p + bytes], &msg->data[p], msg->len - p + 1);
	memcpy (&msg->data[p], text, bytes);
	msg->len += bytes;
	tf->cursor += bytes;
	text_field_find_cursor_line (tf);
}

#if defined OSX

void start_paste(widget_list *widget)
{
	OSStatus err = noErr;
	PasteboardRef gClipboard;
	PasteboardItemID  itemID;
	CFDataRef        flavorData;
	char*          flavorText;

	err = PasteboardCreate( kPasteboardClipboard, &gClipboard );
	//require_noerr( err, CantCreateClipboard );

  	err = PasteboardGetItemIdentifier( gClipboard, 1, &itemID );
	err = PasteboardCopyItemFlavorData( gClipboard, itemID, CFSTR("com.apple.traditional-mac-plain-text"), &flavorData );

	int flavorDataSize = CFDataGetLength(flavorData);
	flavorText=(char*)malloc(flavorDataSize+1);

	short dataIndex;
	for(dataIndex = 0; dataIndex <= flavorDataSize; dataIndex++ )
	{
		char byte = *(CFDataGetBytePtr( flavorData ) + dataIndex);
		flavorText[dataIndex] = (byte>32) ? byte : ' ';
	}
	flavorText[flavorDataSize] = '\0';
	CFRelease(flavorData);
	if (widget == NULL)
	{
		do_paste (flavorText);
	}
	else
	{
		do_paste_to_text_field(widget, flavorText);
	}

	free(flavorText);
	CFRelease( gClipboard );
}

void copy_to_clipboard(const char* text)
{
	OSStatus err = noErr;
	PasteboardRef gClipboard;
	CFDataRef textData;

	err = PasteboardCreate( kPasteboardClipboard, &gClipboard );
	err = PasteboardClear( gClipboard );

	// allocate data based on the size of the selection
	textData = CFDataCreate( kCFAllocatorSystemDefault, (UInt8*)text, strlen(text));

	// add text data to the pasteboard
	err = PasteboardPutItemFlavor( gClipboard, (PasteboardItemID)1,
		CFSTR("com.apple.traditional-mac-plain-text"), textData, 0 );
	CFRelease(textData);
	CFRelease( gClipboard );
}

#elif defined WINDOWS

void start_paste(widget_list *widget)
{
	if (OpenClipboard(NULL))
	{
		HANDLE hText = GetClipboardData (CF_TEXT);
		char* text = GlobalLock (hText);
		if (widget == NULL)
			do_paste(text);
		else
			do_paste_to_text_field(widget, text);
		GlobalUnlock (hText);
		CloseClipboard ();
	}
}

void copy_to_clipboard(const char* text)
{
	SDL_SysWMinfo info;

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

static widget_list *paste_to_widget = NULL;
static char* cur_text_primary = NULL;
static char* cur_text_clipboard = NULL;

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
		if (paste_to_widget == NULL)
		{
			do_paste(value); // copy to input line
		}
		else
		{
			do_paste_to_text_field(paste_to_widget, (const char*) value);
			paste_to_widget = NULL;
		}
	}
	/* XGetWindowProperty allocated this, so we have to free it */
	if(value)
	{
		XFree(value);
	}
}

static void start_paste_from_target(widget_list *widget, int clipboard)
{
	Display *dpy;
	Window window;
	SDL_SysWMinfo wminfo;
	Atom selection;
	Atom property;

	SDL_VERSION(&wminfo.version);
	if (SDL_GetWMInfo(&wminfo) && wminfo.subsystem == SDL_SYSWM_X11)
	{
		wminfo.info.x11.lock_func();

		dpy = wminfo.info.x11.display;
		window = wminfo.info.x11.window;

		paste_to_widget = widget;

		/* Set selection to XA_PRIMARY to use xterm-style select,
		 * or CLIPBOARD to use Gnome-style cut'n'paste.
		 * KDE3 should use CLIPBOARD also, I'm not sure about older
		 * KDE versions */

		if (clipboard)
			selection = XInternAtom(dpy, "CLIPBOARD", 0);
		else
			selection = XA_PRIMARY;
		property = XInternAtom(dpy, "PASTE", 0);
		XConvertSelection(dpy, selection, XA_STRING, property, window, CurrentTime);
		/* - If we used the CLIPBOARD, we don't get a SelectionNotify
		 *   event, so we have to call processpaste immediately.
		 * - If we used XA_PRIMARY, the selection-holder will send us a
		 *   SelectionNotify-event, as soon as the selection is
		 *   available for us. Then finishpaste calls processpaste
		 */
		// Alia: we should receive SelectionNotify event, property is NULL until it comes.
		// Let's try to comment it :)
		// //if(clipboard) {
		//	processpaste(dpy, window, property);
		//}
		wminfo.info.x11.unlock_func();
	}
}

void start_paste(widget_list *widget)
{
	start_paste_from_target(widget, use_clipboard);
}

void start_paste_from_primary(widget_list *widget)
{
	start_paste_from_target(widget, 0);
}

static void copy_to_clipboard_target(const char* text, int clipboard)
{
	Display* dpy;
	Window window;
	SDL_SysWMinfo wminfo;
	Atom selection;

	SDL_VERSION(&wminfo.version);
	if (SDL_GetWMInfo(&wminfo) && (wminfo.subsystem == SDL_SYSWM_X11))
	{
		wminfo.info.x11.lock_func();

		dpy = wminfo.info.x11.display;
		window = wminfo.info.x11.window;

		if (targets_atom == None)
			targets_atom = XInternAtom(dpy, "TARGETS", False);

		if (clipboard)
		{
			selection = XInternAtom(dpy, "CLIPBOARD", False);
			if (cur_text_clipboard) free(cur_text_clipboard);
			cur_text_clipboard = strdup(text);
		}
		else
		{
			selection = XA_PRIMARY;
			if (cur_text_primary) free(cur_text_primary);
			cur_text_primary = strdup(text);
		}
		//property = XInternAtom(dpy, "PASTE", 0);
		XSetSelectionOwner(dpy, selection, window, CurrentTime);
		wminfo.info.x11.unlock_func();
	}
}

void copy_to_clipboard(const char* text)
{
	copy_to_clipboard_target(text, use_clipboard);
}

void copy_to_primary(const char* text)
{
	copy_to_clipboard_target(text, 0);
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
		char *buf = e->selection == XA_PRIMARY
			? cur_text_primary : cur_text_clipboard;
		XChangeProperty(e->display, e->requestor, e->property,
			XA_STRING, 8, PropModeReplace,
			(unsigned char *)buf, strlen(buf));
		r.xselection.property = e->property;
	}
	else if (targets_atom != None && e->target == targets_atom)
	{
		// Tell X we have a string available
		XChangeProperty(e->display, e->requestor, e->property, XA_ATOM,
			32, PropModeReplace, (unsigned char*)targets,
			sizeof(targets) / sizeof(targets[0]));
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
	if (SDL_GetWMInfo(&wminfo) && wminfo.subsystem==SDL_SYSWM_X11)
	{
		wminfo.info.x11.lock_func();

		dpy=wminfo.info.x11.display;
		window=wminfo.info.x11.window;

		if (event.property == None)
		{
			fprintf(stderr,"%s\n",not_ascii);
			return;
		}
		processpaste(dpy, window, event.property);
		wminfo.info.x11.unlock_func();
	}
}

#endif // def OSX / def WINDOWS / other
