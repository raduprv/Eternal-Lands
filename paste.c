#include <stdlib.h>
#include <string.h>
#include <SDL_syswm.h>
#include "elloggingwrapper.h"
#include "gl_init.h"
#include "paste.h"
#include "chat.h"
#include "text.h"
#include "translate.h"

void do_paste(const Uint8* buffer)
{
	paste_in_input_field(buffer);
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
	err = PasteboardCopyItemFlavorData( gClipboard, itemID, CFSTR("public.utf8-plain-text"), &flavorData );

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
		widget_handle_paste(widget, flavorText);
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
		CFSTR("public.utf8-plain-text"), textData, 0 );
	CFRelease(textData);
	CFRelease( gClipboard );
}

#elif defined WINDOWS

void start_paste(widget_list *widget)
{
	if (OpenClipboard(NULL))
	{
		HANDLE hText = GetClipboardData (CF_TEXT);
		if (hText != NULL)
		{
			char* text = GlobalLock (hText);
			if (text != NULL)
			{
				if (widget == NULL)
					do_paste((Uint8 *)text);
				else
					widget_handle_paste(widget, text);
			}
			else
				LOG_TO_CONSOLE(c_red3, "Paste error: GlobalLock()");
			GlobalUnlock (hText);
		}
		else
			LOG_TO_CONSOLE(c_red3, "Paste error: GetClipboardData()");
		CloseClipboard ();
	}
	else
		LOG_TO_CONSOLE(c_red3, "Paste error: OpenClipboard()");
}

void copy_to_clipboard(const char* text)
{
	SDL_SysWMinfo info;

	if (text == NULL)
		return;

	SDL_VERSION (&info.version);
	if (SDL_GetWindowWMInfo (el_gl_window, &info))
	{
		if (OpenClipboard (info.info.win.window))
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

#elif defined ANDROID
// ANDROID_TODO can we have copy/paste back?
int use_clipboard = 0;

void copy_to_clipboard(const char* text)
{
}

void start_paste(widget_list *widget)
{
}

#else

// SDL_Set/GetClipboardText() are portable since SDL 2.0.0 and work
// correctly under both X11 and Wayland (unlike the raw Xlib selection
// code this replaced, which silently did nothing under Wayland — there
// is no SDL_SYSWM_X11 subsystem to match there).
//
// SDL_Set/GetPrimarySelectionText() (the xterm-style middle-click
// selection) are portable only since SDL 2.26.0, so that half is
// compile-time guarded and simply unavailable on an older SDL2 — matches
// the previous behaviour of never populating PRIMARY under Wayland,
// rather than silently misbehaving.
#include <SDL_version.h>

int use_clipboard = 1;

void start_paste(widget_list *widget)
{
	char *text = NULL;

	if (use_clipboard)
	{
		text = SDL_GetClipboardText();
	}
	else
	{
#if SDL_VERSION_ATLEAST(2, 26, 0)
		text = SDL_GetPrimarySelectionText();
#endif
	}

	if (text == NULL || text[0] == '\0')
	{
		SDL_free(text);
		return;
	}

	if (widget == NULL)
		do_paste((const Uint8 *)text);
	else
		widget_handle_paste(widget, text);

	SDL_free(text);
}

void start_paste_from_primary(widget_list *widget)
{
	char *text = NULL;

#if SDL_VERSION_ATLEAST(2, 26, 0)
	text = SDL_GetPrimarySelectionText();
#endif

	if (text == NULL || text[0] == '\0')
	{
		SDL_free(text);
		return;
	}

	if (widget == NULL)
		do_paste((const Uint8 *)text);
	else
		widget_handle_paste(widget, text);

	SDL_free(text);
}

void copy_to_clipboard(const char* text)
{
	if (text == NULL)
		return;

	if (use_clipboard)
	{
		if (SDL_SetClipboardText(text) != 0)
			LOG_ERROR("SDL_SetClipboardText: %s", SDL_GetError());
	}
	else
	{
#if SDL_VERSION_ATLEAST(2, 26, 0)
		if (SDL_SetPrimarySelectionText(text) != 0)
			LOG_ERROR("SDL_SetPrimarySelectionText: %s", SDL_GetError());
#endif
	}
}

void copy_to_primary(const char* text)
{
	if (text == NULL)
		return;

#if SDL_VERSION_ATLEAST(2, 26, 0)
	if (SDL_SetPrimarySelectionText(text) != 0)
		LOG_ERROR("SDL_SetPrimarySelectionText: %s", SDL_GetError());
#endif
}

void init_x11_copy_paste(void)
{
	// No longer needed: SDL owns the selection handling now, there's no
	// raw Xlib error handler to install here anymore. Kept as a no-op so
	// callers (gl_init.c) don't need touching.
}

#endif // def OSX / def WINDOWS / other
