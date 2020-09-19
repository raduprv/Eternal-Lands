#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include "errors.h"
#include "keys.h"
#include "asc.h"
#ifdef FASTER_MAP_LOAD
#include "io/elfilewrapper.h"
#endif

// default definitions for keys
el_key_def K_QUIT = {SDLK_x, KMOD_LALT, "#K_QUIT"};
#ifdef WINDOWS
// Windows SDL reports [Alt Gr] as [Ctrl], which hinders German users typing '@',
// so don't use Ctrl-q as a symbol to exit
el_key_def K_QUIT_ALT = {SDLK_x, KMOD_LALT, "#K_QUIT_ALT"};
#elif OSX
el_key_def K_QUIT_ALT = {SDLK_q, KMOD_LGUI, "#K_QUIT_ALT"};
#else
el_key_def K_QUIT_ALT = {SDLK_q, KMOD_LCTRL, "#K_QUIT_ALT"};
#endif
el_key_def K_CAMERAUP = {SDLK_UP, KMOD_NONE, "#K_CAMERAUP"};
el_key_def K_CAMERADOWN = {SDLK_DOWN, KMOD_NONE, "#K_CAMERADOWN"};
el_key_def K_ZOOMOUT ={SDLK_PAGEDOWN, KMOD_NONE, "#K_ZOOMOUT"};
el_key_def K_ZOOMIN = {SDLK_PAGEUP, KMOD_NONE, "#K_ZOOMIN"};
el_key_def K_TURNLEFT = {SDLK_INSERT, KMOD_NONE, "#K_TURNLEFT"};
el_key_def K_TURNRIGHT = {SDLK_DELETE, KMOD_NONE, "#K_TURNRIGHT"};
el_key_def K_ADVANCE = {SDLK_HOME, KMOD_NONE, "#K_ADVANCE"};
el_key_def K_HEALTHBAR = {SDLK_h, KMOD_LALT, "#K_HEALTHBAR"};
el_key_def K_VIEWNAMES = {SDLK_n, KMOD_LALT, "#K_VIEWNAMES"};
el_key_def K_VIEWHP = {SDLK_b, KMOD_LALT, "#K_VIEWHP"};
el_key_def K_VIEWETHER = {SDLK_k, KMOD_LALT, "#K_VIEWETHER"};
el_key_def K_ETHERBARS = {SDLK_j, KMOD_LALT, "#K_ETHERBARS"};
el_key_def K_STATS = {SDLK_a, KMOD_LCTRL, "#K_STATS"};
el_key_def K_QUESTLOG = {SDLK_g, KMOD_LCTRL, "#K_QUESTLOG"};
el_key_def K_SESSION = {SDLK_z, KMOD_LCTRL, "#K_SESSION"};
el_key_def K_WALK = {SDLK_w, KMOD_LCTRL, "#K_WALK"};
el_key_def K_LOOK = {SDLK_l, KMOD_LCTRL, "#K_LOOK"};
el_key_def K_USE = {SDLK_u, KMOD_LCTRL, "#K_USE"};
el_key_def K_OPTIONS = {SDLK_o, KMOD_LCTRL, "#K_OPTIONS"};
el_key_def K_REPEATSPELL = {SDLK_r, KMOD_LCTRL, "#K_REPEATSPELL"};
el_key_def K_SIGILS = {SDLK_s, KMOD_LCTRL, "#K_SIGILS"};
el_key_def K_MANUFACTURE = {SDLK_m, KMOD_LCTRL, "#K_MANUFACTURE"};
el_key_def K_ITEMS = {SDLK_i, KMOD_LCTRL, "#K_ITEMS"};
el_key_def K_MAP = {SDLK_TAB, KMOD_NONE, "#K_MAP"};
el_key_def K_MINIMAP = {SDLK_m, KMOD_LALT, "#K_MINIMAP"};
el_key_def K_ROTATELEFT = {SDLK_LEFT, KMOD_NONE, "#K_ROTATELEFT"};
el_key_def K_ROTATERIGHT = {SDLK_RIGHT, KMOD_NONE, "#K_ROTATERIGHT"};
el_key_def K_FROTATELEFT = {SDLK_LEFT, KMOD_LSHIFT, "#K_FROTATELEFT"};
el_key_def K_FROTATERIGHT = {SDLK_RIGHT, KMOD_LSHIFT, "#K_FROTATERIGHT"};
el_key_def K_BROWSER = {SDLK_F2, KMOD_NONE, "#K_BROWSER"};
el_key_def K_BROWSERWIN = {SDLK_F2, KMOD_LCTRL, "#K_BROWSERWIN"};
el_key_def K_ESCAPE = {SDLK_ESCAPE, KMOD_NONE, "#K_ESCAPE"};
el_key_def K_CONSOLE = {SDLK_F1, KMOD_NONE, "#K_CONSOLE"};
el_key_def K_SHADOWS = {SDLK_F3, KMOD_NONE, "#K_SHADOWS"};
el_key_def K_KNOWLEDGE = {SDLK_k, KMOD_LCTRL, "#K_KNOWLEDGE"};
el_key_def K_ENCYCLOPEDIA = {SDLK_e, KMOD_LCTRL, "#K_ENCYCLOPEDIA"};
el_key_def K_HELP = {SDLK_h, KMOD_LCTRL, "#K_HELP"};
el_key_def K_NOTEPAD = {SDLK_n, KMOD_LCTRL, "#K_NOTEPAD"};
el_key_def K_HIDEWINS = {SDLK_d, KMOD_LALT, "#K_HIDEWINS"};
el_key_def K_ITEM1 = {SDLK_1, KMOD_LCTRL, "#K_ITEM1"};
el_key_def K_ITEM2 = {SDLK_2, KMOD_LCTRL, "#K_ITEM2"};
el_key_def K_ITEM3 = {SDLK_3, KMOD_LCTRL, "#K_ITEM3"};
el_key_def K_ITEM4 = {SDLK_4, KMOD_LCTRL, "#K_ITEM4"};
el_key_def K_ITEM5 = {SDLK_5, KMOD_LCTRL, "#K_ITEM5"};
el_key_def K_ITEM6 = {SDLK_6, KMOD_LCTRL, "#K_ITEM6"};
el_key_def K_ITEM7 = {SDLK_7, KMOD_LCTRL, "#K_ITEM7"};
el_key_def K_ITEM8 = {SDLK_8, KMOD_LCTRL, "#K_ITEM8"};
el_key_def K_ITEM9 = {SDLK_9, KMOD_LCTRL, "#K_ITEM9"};
el_key_def K_ITEM10 = {SDLK_0, KMOD_LCTRL, "#K_ITEM10"};
el_key_def K_ITEM11 = {SDLK_MINUS, KMOD_LCTRL, "#K_ITEM11"};
el_key_def K_ITEM12 = {SDLK_EQUALS, KMOD_LCTRL, "#K_ITEM12"};
el_key_def K_SCREENSHOT = {SDLK_p, KMOD_LCTRL, "#K_SCREENSHOT"};
el_key_def K_VIEWTEXTASOVERTEXT = {SDLK_o, KMOD_LALT, "#K_VIEWTEXTASOVERTEXT"};
el_key_def K_AFK = {SDLK_a, KMOD_LCTRL, "#K_AFK"};
el_key_def K_SIT = {SDLK_s, KMOD_LALT, "#K_SIT"};
el_key_def K_RANGINGLOCK = {SDLK_r, KMOD_LALT, "#K_RANGINGLOCK"};
el_key_def K_BUDDY = {SDLK_b, KMOD_LCTRL, "#K_BUDDY"};
el_key_def K_NEXT_CHAT_TAB = {SDLK_PAGEDOWN, KMOD_LCTRL, "#K_NEXT_CHAT_TAB"};
el_key_def K_PREV_CHAT_TAB = {SDLK_PAGEUP, KMOD_LCTRL, "#K_PREV_CHAT_TAB"};
el_key_def K_RULES = {SDLK_F5, KMOD_NONE, "#K_RULES"};
el_key_def K_SPELL1 = {SDLK_1, KMOD_LALT, "#K_SPELL1"};
el_key_def K_SPELL2 = {SDLK_2, KMOD_LALT, "#K_SPELL2"};
el_key_def K_SPELL3 = {SDLK_3, KMOD_LALT, "#K_SPELL3"};
el_key_def K_SPELL4 = {SDLK_4, KMOD_LALT, "#K_SPELL4"};
el_key_def K_SPELL5 = {SDLK_5, KMOD_LALT, "#K_SPELL5"};
el_key_def K_SPELL6 = {SDLK_6, KMOD_LALT, "#K_SPELL6"};
el_key_def K_SPELL7 = {SDLK_7, KMOD_LALT, "#K_SPELL7"};
el_key_def K_SPELL8 = {SDLK_8, KMOD_LALT, "#K_SPELL8"};
el_key_def K_SPELL9 = {SDLK_9, KMOD_LALT, "#K_SPELL9"};
el_key_def K_SPELL10 = {SDLK_0, KMOD_LALT, "#K_SPELL10"};
el_key_def K_SPELL11 = {SDLK_MINUS, KMOD_LALT, "#K_SPELL11"};
el_key_def K_SPELL12 = {SDLK_EQUALS, KMOD_LALT, "#K_SPELL12"};
el_key_def K_TABCOMPLETE = {SDLK_SPACE, KMOD_LCTRL, "#K_TABCOMPLETE"};
el_key_def K_WINDOWS_ON_TOP = {SDLK_w, KMOD_LALT, "#K_WINDOWS_ON_TOP"};
el_key_def K_MARKFILTER = {SDLK_f, KMOD_LCTRL, "#K_MARKFILTER"};
el_key_def K_OPAQUEWIN = {SDLK_d, KMOD_LCTRL, "#K_OPAQUEWIN"};
el_key_def K_GRAB_MOUSE = {SDLK_g, KMOD_LALT, "#K_GRAB_MOUSE"};
el_key_def K_FIRST_PERSON = {SDLK_f, KMOD_LALT, "#K_FIRST_PERSON"};
el_key_def K_EXTEND_CAM = {SDLK_e, KMOD_LALT, "#K_EXTEND_CAM"};
#ifndef OSX
el_key_def K_CUT = {SDLK_x, KMOD_LCTRL, "#K_CUT"};
el_key_def K_COPY = {SDLK_c, KMOD_LCTRL, "#K_COPY"};
el_key_def K_PASTE = {SDLK_v, KMOD_LCTRL, "#K_PASTE"};
#else
el_key_def K_CUT = {SDLK_x, KMOD_LGUI, "#K_CUT"};
el_key_def K_COPY = {SDLK_c, KMOD_LGUI, "#K_COPY"};
el_key_def K_PASTE = {SDLK_v, KMOD_LGUI, "#K_PASTE"};
#endif
el_key_def K_COPY_ALT = {SDLK_INSERT, KMOD_LCTRL, "#K_COPY_ALT"};
el_key_def K_PASTE_ALT = {SDLK_INSERT, KMOD_LSHIFT, "#K_PASTE_ALT"};
#ifdef ECDEBUGWIN
el_key_def K_ECDEBUGWIN = { SDLK_c, KMOD_LALT|KMOD_LCTRL, "#K_ECDEBUGWIN"};
#endif
el_key_def K_EMOTES = {SDLK_j, KMOD_LCTRL, "#K_EMOTES"};
el_key_def K_RANGINGWIN = {SDLK_t, KMOD_LCTRL, "#K_RANGINGWIN"};
el_key_def K_TARGET_CLOSE = {SDLK_t, KMOD_LALT, "#K_TARGET_CLOSE"};
el_key_def K_WINSCALEUP = {SDLK_UP, KMOD_LCTRL, "#K_WINSCALEUP"};
el_key_def K_WINSCALEDOWN = {SDLK_DOWN, KMOD_LCTRL, "#K_WINSCALEDOWN"};
el_key_def K_WINSCALEDEF = {SDLK_HOME, KMOD_LCTRL, "#K_WINSCALEDEF"};
el_key_def K_WINSCALEINIT = {SDLK_END, KMOD_LCTRL, "#K_WINSCALEINIT"};
el_key_def K_SUMMONINGMENU = {SDLK_u, KMOD_LALT, "#K_SUMMONINGMENU"};
el_key_def K_CHAT = {SDLK_c, KMOD_LALT, "#K_CHAT"};
// Dont forget to add to key_def_store[]

// Remaining keys are not assigned to the keyboard but
// can be redefined or used by the #keypress command.
// They will get values at startup along with any keys
// undefined in key.ini
el_key_def K_COUNTERS = {SDLK_UNKNOWN, KMOD_NONE, "#K_COUNTERS"};
el_key_def K_HELPSKILLS = {SDLK_UNKNOWN, KMOD_NONE, "#K_HELPSKILLS"};

// a table of key definition, so we can search for definitions
static el_key_def *key_def_store[] =
{
	&K_ADVANCE, &K_AFK, &K_BROWSER, &K_BROWSERWIN, &K_BUDDY, &K_CAMERADOWN, &K_CAMERAUP, &K_CHAT,
	&K_CONSOLE, &K_COPY, &K_COPY_ALT, &K_COUNTERS, &K_CUT, &K_EMOTES, &K_ENCYCLOPEDIA, &K_ESCAPE,
	&K_ETHERBARS, &K_EXTEND_CAM, &K_FIRST_PERSON, &K_FROTATELEFT, &K_FROTATERIGHT, &K_GRAB_MOUSE,
	&K_HEALTHBAR, &K_HELP, &K_HELPSKILLS, &K_HIDEWINS, &K_ITEM1, &K_ITEM10, &K_ITEM11, &K_ITEM12,
	&K_ITEM2, &K_ITEM3, &K_ITEM4, &K_ITEM5, &K_ITEM6, &K_ITEM7, &K_ITEM8, &K_ITEM9, &K_ITEMS,
	&K_KNOWLEDGE, &K_LOOK, &K_MANUFACTURE, &K_MAP, &K_MARKFILTER, &K_MINIMAP, &K_NEXT_CHAT_TAB,
	&K_NOTEPAD, &K_OPAQUEWIN, &K_OPTIONS, &K_PASTE, &K_PASTE_ALT, &K_PREV_CHAT_TAB, &K_QUESTLOG,
	&K_QUIT, &K_QUIT_ALT, &K_RANGINGLOCK, &K_RANGINGWIN, &K_REPEATSPELL, &K_ROTATELEFT,
	&K_ROTATERIGHT, &K_RULES, &K_SCREENSHOT, &K_SESSION, &K_SHADOWS, &K_SIGILS, &K_SIT, &K_SPELL1,
	&K_SPELL10, &K_SPELL11, &K_SPELL12, &K_SPELL2, &K_SPELL3, &K_SPELL4, &K_SPELL5, &K_SPELL6,
	&K_SPELL7, &K_SPELL8, &K_SPELL9, &K_STATS, &K_SUMMONINGMENU, &K_TABCOMPLETE, &K_TARGET_CLOSE,
	&K_TURNLEFT, &K_TURNRIGHT, &K_USE, &K_VIEWETHER, &K_VIEWHP, &K_VIEWNAMES,
	&K_VIEWTEXTASOVERTEXT, &K_WALK, &K_WINDOWS_ON_TOP, &K_WINSCALEDEF, &K_WINSCALEDOWN,
	&K_WINSCALEINIT, &K_WINSCALEUP, &K_ZOOMIN, &K_ZOOMOUT,
#ifdef ECDEBUGWIN
	&K_ECDEBUGWIN
#endif
};

el_key_def get_key_value(const char* name)
{
	size_t num_keys = sizeof(key_def_store)/sizeof(el_key_def *);
	el_key_def null_key_def = {SDLK_UNKNOWN, KMOD_NONE, ""};
	size_t i;

	if ((name == NULL) || strlen(name) == 0)
	{
		LOG_ERROR("%s() empty name\n", __FUNCTION__);
		return null_key_def;
	}
	for (i=0; i<num_keys; i++)
	{
		if (strcasecmp(name, key_def_store[i]->name) == 0)
			return *key_def_store[i];
	}
	return null_key_def;
}

static Uint32 CRC32(const char *data, int len)
{
	unsigned int result = 0;
	int i, j;
	unsigned char octet;

	for (i = 0; i < len; i++)
	{
		octet = *(data++);
		for (j = 0; j < 8; j++)
		{
			if ((octet >> 7) ^ (result >> 31))
				result = (result << 1) ^ 0x04c11db7;
			else
				result = (result << 1);
			octet <<= 1;
		}
	}

	return ~result;
}

// Using the key names from https://wiki.libsdl.org/SDL_Keycode,
// we can convert a specified input from the key.ini (or defaults)
// to the actual key code to use.
static void add_to_key_def(el_key_def *key_def, char *name)
{
	SDL_Keycode key_code;
	size_t i;

	// the key names can contain spaces so we replace space with underscore
	// when defining keys and convert them to real key name, here.
	for (i=0; i<strlen(name); i++)
		if (name[i] == '_')
			name[i] = ' ';

	// the key code is found from the key name...
	key_code = SDL_GetKeyFromName(name);
	if (key_code != SDLK_UNKNOWN)
		key_def->key_code = key_code;

	// ...any modifiers from this hash version of the string.
	else
	{
		int len = strlen(name);
		Uint32 crc = CRC32(name,len);
		switch(crc){
			case 0x6fa8765e: key_def->key_mod |= KMOD_RSHIFT; break; //RSHIFT
			case 0x5a59f8b9: key_def->key_mod |= KMOD_LSHIFT; break; //LSHIFT
			case 0xc535c663: key_def->key_mod |= KMOD_RCTRL; break; //RCTRL
			case 0xb5e083f0: key_def->key_mod |= KMOD_LCTRL; break; //LCTRL
			case 0xf7a834fb: key_def->key_mod |= KMOD_RALT; break; //RALT
			case 0x39b9e58e: key_def->key_mod |= KMOD_LALT; break; //LALT
			case 0x34796737: key_def->key_mod |= KMOD_RGUI; break; //RMETA
			case 0x44ac22a4: key_def->key_mod |= KMOD_LGUI; break; //LMETA
		}
	}
}

static void parse_key_line(const char *line)
{
	char kstr[100]="", t1[100]="", t2[100]="", t3[100]="", t4[100]="";
	el_key_def key_def = {SDLK_UNKNOWN, KMOD_NONE, "Unassigned"};
	int nkey = sscanf(line, " #K_%99s = %99s %99s %99s %99s", kstr, t1, t2, t3, t4);
	size_t num_keys = sizeof(key_def_store)/sizeof(el_key_def *);
	size_t i;

	if (nkey <= 1)
		return;

	add_to_key_def(&key_def, t1);
	if (nkey > 2 && t2[0] != '#')
	{
		add_to_key_def(&key_def, t2);
		if (nkey > 3 && t3[0] != '#')
		{
			add_to_key_def(&key_def, t3);
			if (nkey > 4 && t4[0] != '#')
			{
				add_to_key_def(&key_def, t4);
			}
		}
	}

	for (i=0; i<num_keys; i++)
		if (strcasecmp(kstr, &key_def_store[i]->name[3]) == 0) // skip "#K_"
		{
			key_def_store[i]->key_code = key_def.key_code;
			key_def_store[i]->key_mod = key_def.key_mod;
			//printf("Key def read: Name=%s New Keycode=%d,[%s] New Keymod=%d\n", key_def_store[i]->name, key_def_store[i]->key_code, SDL_GetKeyName(key_def_store[i]->key_code), key_def_store[i]->key_mod);
			break;
		}
}

// load the dynamic definitions for keys
void read_key_config(void)
{
	char line[512];
	el_file_ptr f;
	size_t num_keys = sizeof(key_def_store)/sizeof(el_key_def *);
	size_t i;
	SDL_Keycode last_key_code_value = SDL_NUM_SCANCODES;

	f = el_open_custom("key.ini");
	if (f)
	{
		while (el_fgets(line, sizeof(line), f))
			parse_key_line(line);
		el_close(f);
	}

	// look for unassigned keys and assign one up from last defined key
	for (i=0; i<num_keys; i++)
		if (key_def_store[i]->key_code == SDLK_UNKNOWN)
			key_def_store[i]->key_code = SDL_SCANCODE_TO_KEYCODE(++last_key_code_value);
}

// Returns (in the buffer provided) a string describing the specified keydef.
const char *get_key_string(el_key_def keydef, char *buf, size_t buflen)
{
	size_t i;
	char *mod = "";
	if (keydef.key_mod & KMOD_LCTRL)
		mod = "lctrl-";
	else if (keydef.key_mod == KMOD_RCTRL)
		mod = "rctrl-";
	else if (keydef.key_mod == KMOD_CTRL)
		mod = "ctrl-";
	else if (keydef.key_mod == KMOD_LALT)
		mod = "lalt-";
	else if (keydef.key_mod == KMOD_RALT)
		mod = "ralt-";
	else if (keydef.key_mod == KMOD_ALT)
		mod = "alt-";
	else if (keydef.key_mod == KMOD_LSHIFT)
		mod = "lshift-";
	else if (keydef.key_mod == KMOD_RSHIFT)
		mod = "rshift-";
	else if (keydef.key_mod == KMOD_SHIFT)
		mod = "shift-";
	safe_snprintf(buf, buflen, "%s%s", mod, SDL_GetKeyName(keydef.key_code));
	for (i = 0; buf[i]; i++)
		buf[i] = tolower(buf[i]);
	return buf;
}
