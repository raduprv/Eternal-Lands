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

// EL's internal text buffers store one byte per glyph, where the byte
// value is the character's ISO-8859-1 (Latin-1) codepoint (0x00-0xFF) -
// see utf8_to_unicode() in events.c, which decodes SDL's UTF-8
// SDL_TEXTINPUT events back down to that same single-byte encoding
// before storing typed characters. SDL_Set/GetClipboardText() and
// SDL_Set/GetPrimarySelectionText() require valid UTF-8, so without
// converting at this boundary, any accented character (byte >= 0x80,
// e.g. ae/oe/ue-umlauts) is invalid UTF-8 on copy - most receiving
// applications silently drop or replace invalid UTF-8 bytes, which is
// exactly the "umlauts don't get copied" symptom. Pasting has the same
// problem in reverse: real UTF-8 from another application would get
// inserted byte-for-byte into EL's single-byte buffer instead of being
// decoded back to one Latin-1 byte per character.

// Encode an EL-internal Latin-1 string as newly malloc'd UTF-8, for handing to SDL's clipboard functions.
static char* latin1_to_utf8(const char* text)
{
	const unsigned char* p = (const unsigned char*)text;
	char* out = malloc(2 * strlen(text) + 1); // worst case: every byte becomes 2 UTF-8 bytes
	char* o = out;

	for (; *p != '\0'; ++p)
	{
		if (*p < 0x80)
		{
			*o++ = (char)*p;
		}
		else
		{
			*o++ = (char)(0xc0 | (*p >> 6));
			*o++ = (char)(0x80 | (*p & 0x3f));
		}
	}
	*o = '\0';
	return out;
}

// Decode UTF-8 clipboard text (from SDL, i.e. from another application) into a newly malloc'd
// EL-internal Latin-1 string. Codepoints outside U+0000-U+00FF can't be represented in EL's
// single-byte charset/font and are replaced with '?'; malformed UTF-8 bytes are skipped.
static char* utf8_to_latin1(const char* text)
{
	const unsigned char* p = (const unsigned char*)text;
	char* out = malloc(strlen(text) + 1); // decoding never grows the byte count
	char* o = out;

	while (*p != '\0')
	{
		if (*p < 0x80)
		{
			*o++ = (char)*p++;
		}
		else if ((p[0] & 0xe0) == 0xc0 && (p[1] & 0xc0) == 0x80)
		{
			unsigned int cp = ((p[0] & 0x1f) << 6) | (p[1] & 0x3f);
			*o++ = (cp <= 0xff) ? (char)cp : '?';
			p += 2;
		}
		else if ((p[0] & 0xf0) == 0xe0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80)
		{
			*o++ = '?';
			p += 3;
		}
		else if ((p[0] & 0xf8) == 0xf0 && (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 && (p[3] & 0xc0) == 0x80)
		{
			*o++ = '?';
			p += 4;
		}
		else
		{
			++p; // invalid UTF-8 lead/continuation byte, drop it
		}
	}
	*o = '\0';
	return out;
}

void start_paste(widget_list *widget)
{
	char *text = NULL;
	char *latin1_text;

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

	latin1_text = utf8_to_latin1(text);
	SDL_free(text);

	if (widget == NULL)
		do_paste((const Uint8 *)latin1_text);
	else
		widget_handle_paste(widget, latin1_text);

	free(latin1_text);
}

void start_paste_from_primary(widget_list *widget)
{
	char *text = NULL;
	char *latin1_text;

#if SDL_VERSION_ATLEAST(2, 26, 0)
	text = SDL_GetPrimarySelectionText();
#endif

	if (text == NULL || text[0] == '\0')
	{
		SDL_free(text);
		return;
	}

	latin1_text = utf8_to_latin1(text);
	SDL_free(text);

	if (widget == NULL)
		do_paste((const Uint8 *)latin1_text);
	else
		widget_handle_paste(widget, latin1_text);

	free(latin1_text);
}

void copy_to_clipboard(const char* text)
{
	char *utf8_text;

	if (text == NULL)
		return;

	utf8_text = latin1_to_utf8(text);

	if (use_clipboard)
	{
		if (SDL_SetClipboardText(utf8_text) != 0)
			LOG_ERROR("SDL_SetClipboardText: %s", SDL_GetError());
	}
	else
	{
#if SDL_VERSION_ATLEAST(2, 26, 0)
		if (SDL_SetPrimarySelectionText(utf8_text) != 0)
			LOG_ERROR("SDL_SetPrimarySelectionText: %s", SDL_GetError());
#endif
	}

	free(utf8_text);
}

void copy_to_primary(const char* text)
{
	char *utf8_text;

	if (text == NULL)
		return;

	utf8_text = latin1_to_utf8(text);

#if SDL_VERSION_ATLEAST(2, 26, 0)
	if (SDL_SetPrimarySelectionText(utf8_text) != 0)
		LOG_ERROR("SDL_SetPrimarySelectionText: %s", SDL_GetError());
#endif

	free(utf8_text);
}

void init_x11_copy_paste(void)
{
	// No longer needed: SDL owns the selection handling now, there's no
	// raw Xlib error handler to install here anymore. Kept as a no-op so
	// callers (gl_init.c) don't need touching.
}

#endif // def OSX / def WINDOWS / other
