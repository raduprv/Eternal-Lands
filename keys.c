#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <SDL_keysym.h>
#include "errors.h"
#include "keys.h"
#include "asc.h"
#include "init.h"
#include "misc.h"
#ifdef FASTER_MAP_LOAD
#include "io/elfilewrapper.h"
#endif

// default definitions for keys
Uint32 K_QUIT=ALT|'x';
#ifdef WINDOWS
// Windows SDL reports [Alt Gr] as [Ctrl], which hinders German users typing '@',
// so don't use Ctrl-q as a symbol to exit
Uint32 K_QUIT_ALT=ALT|'x';
#elif OSX
Uint32 K_QUIT_ALT=KMOD_LMETA|'q';
#else
Uint32 K_QUIT_ALT=CTRL|'q';
#endif
Uint32 K_CAMERAUP=SDLK_UP;
Uint32 K_CAMERADOWN=SDLK_DOWN;
Uint32 K_ZOOMOUT=SDLK_PAGEDOWN;
Uint32 K_ZOOMIN=SDLK_PAGEUP;
Uint32 K_TURNLEFT=SDLK_INSERT;
Uint32 K_TURNRIGHT=SDLK_DELETE;
Uint32 K_ADVANCE=SDLK_HOME;
Uint32 K_HEALTHBAR=ALT|'h';
Uint32 K_VIEWNAMES=ALT|'n';
Uint32 K_VIEWHP=ALT|'b';
Uint32 K_STATS=CTRL|'a';
Uint32 K_QUESTLOG=CTRL|'g';
Uint32 K_SESSION=CTRL|'z';
Uint32 K_WALK=CTRL|'w';
Uint32 K_LOOK=CTRL|'l';
Uint32 K_USE=CTRL|'u';
Uint32 K_OPTIONS=CTRL|'o';
Uint32 K_REPEATSPELL=CTRL|'r';
Uint32 K_SIGILS=CTRL|'s';
Uint32 K_MANUFACTURE=CTRL|'m';
Uint32 K_ITEMS=CTRL|'i';
Uint32 K_MAP=SDLK_TAB;
Uint32 K_MINIMAP=ALT|'m';
Uint32 K_ROTATELEFT=SDLK_LEFT;
Uint32 K_ROTATERIGHT=SDLK_RIGHT;
Uint32 K_FROTATELEFT=SHIFT|SDLK_LEFT;
Uint32 K_FROTATERIGHT=SHIFT|SDLK_RIGHT;
Uint32 K_BROWSER=SDLK_F2;
Uint32 K_BROWSERWIN=CTRL|SDLK_F2;
Uint32 K_ESCAPE=SDLK_ESCAPE;
Uint32 K_CONSOLE=SDLK_F1;
Uint32 K_SHADOWS=SDLK_F3;
Uint32 K_KNOWLEDGE=CTRL|'k';
Uint32 K_ENCYCLOPEDIA=CTRL|'e';
Uint32 K_HELP=CTRL|'h';
Uint32 K_NOTEPAD=CTRL|'n';
Uint32 K_HIDEWINS=ALT|'d';
Uint32 K_ITEM1=CTRL|'1';
Uint32 K_ITEM2=CTRL|'2';
Uint32 K_ITEM3=CTRL|'3';
Uint32 K_ITEM4=CTRL|'4';
Uint32 K_ITEM5=CTRL|'5';
Uint32 K_ITEM6=CTRL|'6';
Uint32 K_ITEM7=CTRL|'7';
Uint32 K_ITEM8=CTRL|'8';
Uint32 K_ITEM9=CTRL|'9';
Uint32 K_ITEM10=CTRL|'0';
Uint32 K_ITEM11=CTRL|'-';
Uint32 K_ITEM12=CTRL|'=';
Uint32 K_SCREENSHOT=CTRL|'p';
Uint32 K_VIEWTEXTASOVERTEXT=ALT|'o';
Uint32 K_AFK=CTRL|ALT|'a';
Uint32 K_SIT=ALT|'s';
Uint32 K_RANGINGLOCK=ALT|'r';
Uint32 K_BUDDY=CTRL|'b';
Uint32 K_NEXT_CHAT_TAB=CTRL|SDLK_PAGEDOWN;
Uint32 K_PREV_CHAT_TAB=CTRL|SDLK_PAGEUP;
Uint32 K_RULES=SDLK_F5;
Uint32 K_SPELL1=ALT|'1';
Uint32 K_SPELL2=ALT|'2';
Uint32 K_SPELL3=ALT|'3';
Uint32 K_SPELL4=ALT|'4';
Uint32 K_SPELL5=ALT|'5';
Uint32 K_SPELL6=ALT|'6';
Uint32 K_SPELL7=ALT|'7';
Uint32 K_SPELL8=ALT|'8';
Uint32 K_SPELL9=ALT|'9';
Uint32 K_SPELL10=ALT|'0';
Uint32 K_SPELL11=ALT|'-';
Uint32 K_SPELL12=ALT|'=';
Uint32 K_TABCOMPLETE=CTRL|' ';
Uint32 K_WINDOWS_ON_TOP=ALT|'w';
Uint32 K_MARKFILTER=CTRL|'f';
Uint32 K_OPAQUEWIN=CTRL|'d';
Uint32 K_GRAB_MOUSE=ALT|'g';
Uint32 K_FIRST_PERSON=ALT|'f';
Uint32 K_EXTEND_CAM=ALT|'e';
#ifndef OSX
Uint32 K_CUT=CTRL|'x';
Uint32 K_COPY=CTRL|'c';
Uint32 K_PASTE=CTRL|'v';
#else
Uint32 K_CUT=KMOD_LMETA|'x';
Uint32 K_COPY=KMOD_LMETA|'c';
Uint32 K_PASTE=KMOD_LMETA|'v';
#endif
Uint32 K_COPY_ALT=CTRL|SDLK_INSERT;
Uint32 K_PASTE_ALT=SHIFT|SDLK_INSERT;
#ifdef ECDEBUGWIN
Uint32 K_ECDEBUGWIN=ALT|CTRL|'c';
#endif
Uint32 K_EMOTES=CTRL|'j';
Uint32 K_RANGINGWIN=CTRL|'t';

static void add_key(Uint32 *key, Uint32 n)
{
	switch (n)
	{
		case 303:
		case 304:
			*key |= SHIFT;
			break;
		case 305:
		case 306:
			*key |= CTRL;
			break;
		case 307:
		case 308:
			*key |= ALT;
			break;
		default:
			*key = (n & 0xFFFF) | (*key &0xFFFF0000);
        }
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

static Uint16 get_key_code(const char *key)
{
	int len = strlen(key);

	if (len==1)
	{
		return tolower(key[0]);
	}
	else
	{
		Uint32 crc = CRC32(key,len);
		switch(crc){
			case 0x414243d2: //UP
				return 273;
			case 0x8b9c5c32: //F1
				return 282;
			case 0x86df7aeb: //F2
				return 283;
			case 0x821e675c: //F3
				return 284;
			case 0x9c593759: //F4
				return 285;
			case 0x98982aee: //F5
				return 286;
			case 0x95db0c37: //F6
				return 287;
			case 0x911a1180: //F7
				return 288;
			case 0xa955ac3d: //F8
				return 289;
			case 0xad94b18a: //F9
				return 290;
			case 0xbbde3454: //F10
				return 291;
			case 0xbf1f29e3: //F11
				return 292;
			case 0xb25c0f3a: //F12
				return 293;
			case 0xb69d128d: //F13
				return 294;
			case 0xa8da4288: //F14
				return 295;
			case 0xac1b5f3f: //F15
				return 296;
			case 0xe5b332af: //BACKSPACE
				return 8;
			case 0x3d6742da: //TAB
				return 9;
			case 0xe4f512ce: //CLEAR
				return 12;
			case 0xe5c642f: //RETURN
				return 13;
			case 0x1a3dbcf4: //PAUSE
				return 19;
			case 0xb23e322f: //ESCAPE
				return 27;
			case 0xe0ea4208: //SPACE
				return 32;
			case 0x3f048816: //DELETE
				return 127;
			case 0x5dd541: //KP0
				return 256;
			case 0x49cc8f6: //KP1
				return 257;
			case 0x9dfee2f: //KP2
				return 258;
			case 0xd1ef398: //KP3
				return 259;
			case 0x1359a39d: //KP4
				return 260;
			case 0x1798be2a: //KP5
				return 261;
			case 0x1adb98f3: //KP6
				return 262;
			case 0x1e1a8544: //KP7
				return 263;
			case 0x265538f9: //KP8
				return 264;
			case 0x2294254e: //KP9
				return 265;
			case 0xc9681663: //KP_PERIOD
				return 266;
			case 0xf2032002: //KP_DIVIDE
				return 267;
			case 0xc69c9177: //KP_MULTIPLY
				return 268;
			case 0xe05a3b75: //KP_MINUS
				return 269;
			case 0x7a14ede0: //KP_PLUS
				return 270;
			case 0xb95fb1fa: //KP_ENTER
				return 271;
			case 0x997d27b6: //KP_EQUALS
				return 272;
			case 0x412c789a: //DOWN
				return 274;
			case 0xcfd43bcf: //RIGHT
				return 275;
			case 0x14618acf: //LEFT
				return 276;
			case 0xb448467c: //INSERT
				return 277;
			case 0xd59321ba: //HOME
				return 278;
			case 0x863456b7: //END
				return 279;
			case 0xd541afe1: //PAGEUP
				return 280;
			case 0x77a53c61: //PAGEDOWN
				return 281;
			case 0x8563dfd4: //NUMLOCK
				return 300;
			case 0x4b601de5: //CAPSLOCK
				return 301;
			case 0x7b642f: //SCROLLOCK
				return 302;
			case 0x6fa8765e: //RSHIFT
				return 303;
			case 0x5a59f8b9: //LSHIFT
				return 304;
			case 0xc535c663: //RCTRL
				return 305;
			case 0xb5e083f0: //LCTRL
				return 306;
			case 0xf7a834fb: //RALT
				return 307;
			case 0x39b9e58e: //LALT
				return 308;
			case 0x34796737: //RMETA
				return 309;
			case 0x44ac22a4: //LMETA
				return 310;
			case 0x8ec5890c: //LSUPER
				return 311;
			case 0xbb3407eb: //RSUPER
				return 312;
			case 0x2d5a7586: //MODE
				return 313;
			case 0x87140862: //COMPOSE
				return 314;
			case 0x512a6d4b: //HELP
				return 315;
			case 0xdc87c39e: //PRINT
				return 316;
			case 0xbdf2d984: //SYSREQ
				return 317;
			case 0xd318f49: //BREAK
				return 318;
			case 0x46854e9d: //MENU
				return 319;
			case 0x8758b6ec: //POWER
				return 320;
			case 0x1e43eaa9: //EURO
				return 321;
			case 0xdf6ba7e: //UNDO
				return 322;
			default:
				return 0;
		}
	}
}

#ifdef FASTER_MAP_LOAD
static void parse_key_line(const char *line)
{
	char kstr[100], t1[100], t2[100], t3[100], t4[100];
	Uint32 key = 0;
	int nkey = sscanf(line, " #K_%99s = %99s %99s %99s %99s", kstr,
		t1, t2, t3, t4);

	if (nkey <= 1)
		return;

	add_key(&key, get_key_code(t1));
	if (nkey > 2 && t2[0] != '#')
	{
		add_key(&key, get_key_code(t2));
		if (nkey > 3 && t3[0] != '#')
		{
			add_key(&key, get_key_code(t3));
			if (nkey > 4 && t4[0] != '#')
			{
				add_key(&key, get_key_code(t4));
			}
		}
	}

	if (strcasecmp(kstr, "QUIT") == 0)
		K_QUIT = key;
	else if (strcasecmp(kstr, "QUIT_ALT") == 0)
		K_QUIT_ALT = key;
	else if (strcasecmp(kstr, "CAMERAUP") == 0)
		K_CAMERAUP = key;
	else if (strcasecmp(kstr, "CAMERADOWN") == 0)
		K_CAMERADOWN = key;
	else if (strcasecmp(kstr, "ZOOMOUT") == 0)
		K_ZOOMOUT = key;
	else if (strcasecmp(kstr, "ZOOMIN") == 0)
		K_ZOOMIN = key;
	else if (strcasecmp(kstr, "TURNLEFT") == 0)
		K_TURNLEFT = key;
	else if (strcasecmp(kstr, "TURNRIGHT") == 0)
		K_TURNRIGHT = key;
	else if (strcasecmp(kstr, "ADVANCE") == 0)
		K_ADVANCE = key;
	else if (strcasecmp(kstr, "HEALTHBAR") == 0)
		K_HEALTHBAR = key;
	else if (strcasecmp(kstr, "VIEWNAMES") == 0)
		K_VIEWNAMES = key;
	else if (strcasecmp(kstr, "VIEWHP") == 0)
		K_VIEWHP = key;
	else if (strcasecmp(kstr, "STATS") == 0)
		K_STATS = key;
	else if (strcasecmp(kstr, "QUESTLOG") == 0)
		K_QUESTLOG = key;
	else if (strcasecmp(kstr, "SESSION") == 0)
		K_SESSION = key;
	else if (strcasecmp(kstr, "WALK") == 0)
		K_WALK = key;
	else if (strcasecmp(kstr, "LOOK") == 0)
		K_LOOK = key;
	else if (strcasecmp(kstr, "USE") == 0)
		K_USE = key;
	else if (strcasecmp(kstr, "OPTIONS") == 0)
		K_OPTIONS = key;
	else if (strcasecmp(kstr, "REPEATSPELL") == 0)
		K_REPEATSPELL = key;
	else if (strcasecmp(kstr, "SIGILS") == 0)
		K_SIGILS = key;
	else if (strcasecmp(kstr, "MANUFACTURE") == 0)
		K_MANUFACTURE = key;
	else if (strcasecmp(kstr, "ITEMS") == 0)
		K_ITEMS = key;
	else if (strcasecmp(kstr, "MAP") == 0)
		K_MAP = key;
	else if (strcasecmp(kstr, "MINIMAP") == 0)
		K_MINIMAP = key;
	else if (strcasecmp(kstr, "ROTATELEFT") == 0)
		K_ROTATELEFT = key;
	else if (strcasecmp(kstr, "ROTATERIGHT") == 0)
		K_ROTATERIGHT = key;
	else if (strcasecmp(kstr, "FROTATELEFT") == 0)
		K_FROTATELEFT = key;
	else if (strcasecmp(kstr, "FROTATERIGHT") == 0)
		K_FROTATERIGHT = key;
	else if (strcasecmp(kstr, "BROWSER") == 0)
		K_BROWSER = key;
	else if (strcasecmp(kstr, "BROWSERWIN") == 0)
		K_BROWSERWIN = key;
	else if (strcasecmp(kstr, "ESCAPE") == 0)
		K_ESCAPE = key;
	else if (strcasecmp(kstr, "CONSOLE") == 0)
		K_CONSOLE = key;
	else if (strcasecmp(kstr, "SHADOWS") == 0)
		K_SHADOWS = key;
	else if (strcasecmp(kstr, "KNOWLEDGE") == 0)
		K_KNOWLEDGE = key;
	else if (strcasecmp(kstr, "ENCYCLOPEDIA") == 0)
		K_ENCYCLOPEDIA = key;
	else if (strcasecmp(kstr, "HELP") == 0)
		K_HELP = key;
	else if (strcasecmp(kstr, "NOTEPAD") == 0)
		K_NOTEPAD = key;
	else if (strcasecmp(kstr, "HIDEWINS") == 0)
		K_HIDEWINS = key;
	else if (strcasecmp(kstr, "ITEM1") == 0)
		K_ITEM1 = key;
	else if (strcasecmp(kstr, "ITEM2") == 0)
		K_ITEM2 = key;
	else if (strcasecmp(kstr, "ITEM3") == 0)
		K_ITEM3 = key;
	else if (strcasecmp(kstr, "ITEM4") == 0)
		K_ITEM4 = key;
	else if (strcasecmp(kstr, "ITEM5") == 0)
		K_ITEM5 = key;
	else if (strcasecmp(kstr, "ITEM6") == 0)
		K_ITEM6 = key;
	else if (strcasecmp(kstr, "ITEM7") == 0)
		K_ITEM7 = key;
	else if (strcasecmp(kstr, "ITEM8") == 0)
		K_ITEM8 = key;
	else if (strcasecmp(kstr, "ITEM9") == 0)
		K_ITEM9 = key;
	else if (strcasecmp(kstr, "ITEM10") == 0)
		K_ITEM10 = key;
	else if (strcasecmp(kstr, "ITEM11") == 0)
		K_ITEM11 = key;
	else if (strcasecmp(kstr, "ITEM12") == 0)
		K_ITEM12 = key;
	else if (strcasecmp(kstr, "SCREENSHOT") == 0)
		K_SCREENSHOT = key;
	else if (strcasecmp(kstr, "VIEWTEXTASOVERTEXT") == 0)
		K_VIEWTEXTASOVERTEXT = key;
	else if (strcasecmp(kstr, "AFK") == 0)
		K_AFK = key;
	else if (strcasecmp(kstr, "SIT") == 0)
		K_SIT = key;
	else if (strcasecmp(kstr, "RANGINGLOCK") == 0)
		K_RANGINGLOCK = key;
	else if (strcasecmp(kstr, "BUDDY") == 0)
		K_BUDDY = key;
	else if (strcasecmp(kstr, "NEXT_CHAT_TAB") == 0)
		K_NEXT_CHAT_TAB = key;
	else if (strcasecmp(kstr, "PREV_CHAT_TAB") == 0)
		K_PREV_CHAT_TAB = key;
	else if (strcasecmp(kstr, "RULES") == 0)
		K_RULES = key;
	else if (strcasecmp(kstr, "SPELL1") == 0)
		K_SPELL1 = key;
	else if (strcasecmp(kstr, "SPELL2") == 0)
		K_SPELL2 = key;
	else if (strcasecmp(kstr, "SPELL3") == 0)
		K_SPELL3 = key;
	else if (strcasecmp(kstr, "SPELL4") == 0)
		K_SPELL4 = key;
	else if (strcasecmp(kstr, "SPELL5") == 0)
		K_SPELL5 = key;
	else if (strcasecmp(kstr, "SPELL6") == 0)
		K_SPELL6 = key;
	else if (strcasecmp(kstr, "SPELL7") == 0)
		K_SPELL7 = key;
	else if (strcasecmp(kstr, "SPELL8") == 0)
		K_SPELL8 = key;
	else if (strcasecmp(kstr, "SPELL9") == 0)
		K_SPELL9 = key;
	else if (strcasecmp(kstr, "SPELL10") == 0)
		K_SPELL10 = key;
	else if (strcasecmp(kstr, "SPELL11") == 0)
		K_SPELL11 = key;
	else if (strcasecmp(kstr, "SPELL12") == 0)
		K_SPELL12 = key;
	else if (strcasecmp(kstr, "TABCOMPLETE") == 0)
		K_TABCOMPLETE = key;
	else if (strcasecmp(kstr, "WINDOWS_ON_TOP") == 0)
		K_WINDOWS_ON_TOP = key;
	else if (strcasecmp(kstr, "MARKFILTER") == 0)
		K_MARKFILTER = key;
	else if (strcasecmp(kstr, "OPAQUEWIN") == 0)
		K_OPAQUEWIN = key;
	else if (strcasecmp(kstr, "GRAB_MOUSE") == 0)
		K_GRAB_MOUSE = key;
	else if (strcasecmp(kstr, "FIRST_PERSON") == 0)
		K_FIRST_PERSON = key;
	else if (strcasecmp(kstr, "EXTEND_CAM") == 0)
		K_EXTEND_CAM = key;
	else if (strcasecmp(kstr, "EMOTES") == 0)
		K_EMOTES = key;
}

// load the dynamic definitions for keys
void read_key_config()
{
	char line[512];
	el_file_ptr f;

	f = el_open_custom("key.ini");
	if (!f)
		return; //take the defaults

	while (el_fgets(line, sizeof(line), f))
		parse_key_line(line);

	el_close(f);
}

#else  // FASTER_MAP_LOAD

Uint32 parse_key_string (const char *s)
{
	char t1[100],t2[100],t3[100],t4[100];
	Uint32 key=0;
	int nkey = sscanf (s, "%99s %99s %99s %99s", t1, t2, t3, t4);

	if (nkey > 0)
	{
		add_key (&key, get_key_code (t1));
		if (nkey > 1 && t2[0] != '#')
		{
			add_key (&key, get_key_code (t2));
			if (nkey > 2 && t3[0] != '#')
			{
				add_key (&key, get_key_code (t3));
				if (nkey > 3 && t4[0] != '#')
				{
					add_key (&key, get_key_code (t4));
				}
			}
		}
	}

	return key;
}

// load the dynamic definitions for keys
void read_key_config()
{
	FILE *f = NULL;
	char * file_mem;
	struct stat key_file;
	int key_file_size,t;
	size_t ret;

#ifndef WINDOWS
	char key_ini[256];
	safe_snprintf (key_ini, sizeof (key_ini), "%s/key.ini", configdir);
	// don't use my_fopen, not everyone keeps local settings
	f=fopen(key_ini,"rb"); //try to load local settings
	if(!f) //use global settings
		{
			safe_snprintf (key_ini, sizeof (key_ini), "%s/key.ini", datadir);
			f=my_fopen(key_ini,"rb");
		}

	if (f)
		{
			fstat (fileno (f), &key_file);
		}

#else
	f=my_fopen("key.ini","rb");
	if (f)
        fstat (fileno (f), &key_file);
#endif

	if(!f)
	{
		return; //take the defaults
	}

	key_file_size = key_file.st_size;
	if (key_file_size <= 0)
	{
		fclose(f);
		return;
	}

	file_mem = (char *) calloc(key_file_size+2, sizeof(Uint8));
	ret = fread (file_mem, 1, key_file_size+1, f);
	fclose(f);
	if (ret != key_file_size)
	{
		LOG_ERROR("%s() read failed %zu %d\n", __FUNCTION__, ret ,key_file_size+1);
		free(file_mem);
		return;
	}

	if ( (t = get_string_occurance ("#K_QUIT", file_mem, key_file_size, 0)) != -1)
		K_QUIT = parse_key_string (&file_mem[t]);
	if ( (t = get_string_occurance ("#K_QUIT_ALT", file_mem, key_file_size, 0)) != -1)
		K_QUIT_ALT = parse_key_string (&file_mem[t]);
	if((t=get_string_occurance("#K_CAMERAUP",file_mem,key_file_size,0))!=-1)
		K_CAMERAUP = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_CAMERADOWN",file_mem,key_file_size,0))!=-1)
		K_CAMERADOWN = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ZOOMOUT",file_mem,key_file_size,0))!=-1)
		K_ZOOMOUT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ZOOMIN",file_mem,key_file_size,0))!=-1)
		K_ZOOMIN = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_TURNLEFT",file_mem,key_file_size,0))!=-1)
		K_TURNLEFT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_TURNRIGHT",file_mem,key_file_size,0))!=-1)
		K_TURNRIGHT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ADVANCE",file_mem,key_file_size,0))!=-1)
		K_ADVANCE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_HEALTHBAR",file_mem,key_file_size,0))!=-1)
		K_HEALTHBAR = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_VIEWNAMES",file_mem,key_file_size,0))!=-1)
		K_VIEWNAMES = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_VIEWHP",file_mem,key_file_size,0))!=-1)
		K_VIEWHP = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_STATS",file_mem,key_file_size,0))!=-1)
		K_STATS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_QUESTLOG",file_mem,key_file_size,0))!=-1)
		K_QUESTLOG = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SESSION",file_mem,key_file_size,0))!=-1)
		K_SESSION = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_WALK",file_mem,key_file_size,0))!=-1)
		K_WALK = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_LOOK",file_mem,key_file_size,0))!=-1)
		K_LOOK = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_USE",file_mem,key_file_size,0))!=-1)
		K_USE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_OPTIONS",file_mem,key_file_size,0))!=-1)
		K_OPTIONS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_REPEATSPELL",file_mem,key_file_size,0))!=-1)
		K_REPEATSPELL = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SIGILS",file_mem,key_file_size,0))!=-1)
		K_SIGILS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_MANUFACTURE",file_mem,key_file_size,0))!=-1)
		K_MANUFACTURE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEMS",file_mem,key_file_size,0))!=-1)
		K_ITEMS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_MAP",file_mem,key_file_size,0))!=-1)
		K_MAP = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_MINIMAP",file_mem,key_file_size,0))!=-1)
		K_MINIMAP = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ROTATELEFT",file_mem,key_file_size,0))!=-1)
		K_ROTATELEFT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ROTATERIGHT",file_mem,key_file_size,0))!=-1)
		K_ROTATERIGHT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_FROTATELEFT",file_mem,key_file_size,0))!=-1)
		K_FROTATELEFT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_FROTATERIGHT",file_mem,key_file_size,0))!=-1)
		K_FROTATERIGHT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_BROWSER",file_mem,key_file_size,0))!=-1)
		K_BROWSER = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_BROWSERWIN",file_mem,key_file_size,0))!=-1)
		K_BROWSERWIN = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ESCAPE",file_mem,key_file_size,0))!=-1)
		K_ESCAPE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_CONSOLE",file_mem,key_file_size,0))!=-1)
		K_CONSOLE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SHADOWS",file_mem,key_file_size,0))!=-1)
		K_SHADOWS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_KNOWLEDGE",file_mem,key_file_size,0))!=-1)
		K_KNOWLEDGE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ENCYCLOPEDIA",file_mem,key_file_size,0))!=-1)
		K_ENCYCLOPEDIA = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_HELP",file_mem,key_file_size,0))!=-1)
		K_HELP = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_NOTEPAD",file_mem,key_file_size,0))!=-1)
		K_NOTEPAD = parse_key_string(&file_mem[t]);	
	if((t=get_string_occurance("#K_HIDEWINS",file_mem,key_file_size,0))!=-1)
		K_HIDEWINS = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM1",file_mem,key_file_size,0))!=-1)
		K_ITEM1 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM2",file_mem,key_file_size,0))!=-1)
		K_ITEM2 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM3",file_mem,key_file_size,0))!=-1)
		K_ITEM3 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM4",file_mem,key_file_size,0))!=-1)
		K_ITEM4 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM5",file_mem,key_file_size,0))!=-1)
		K_ITEM5 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM6",file_mem,key_file_size,0))!=-1)
		K_ITEM6 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM7",file_mem,key_file_size,0))!=-1)
		K_ITEM7 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM8",file_mem,key_file_size,0))!=-1)
		K_ITEM8 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM9",file_mem,key_file_size,0))!=-1)
		K_ITEM9 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM10",file_mem,key_file_size,0))!=-1)
		K_ITEM10 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM11",file_mem,key_file_size,0))!=-1)
		K_ITEM11 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_ITEM12",file_mem,key_file_size,0))!=-1)
		K_ITEM12 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SCREENSHOT",file_mem,key_file_size,0))!=-1)
		K_SCREENSHOT = parse_key_string (&file_mem[t]);
	if((t=get_string_occurance("#K_VIEWTEXTASOVERTEXT",file_mem,key_file_size,0))!=-1)
		K_VIEWTEXTASOVERTEXT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_AFK",file_mem,key_file_size,0))!=-1)
		K_AFK = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SIT",file_mem,key_file_size,0))!=-1)
		K_SIT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_RANGINGLOCK",file_mem,key_file_size,0))!=-1)
		K_RANGINGLOCK = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_BUDDY",file_mem,key_file_size,0))!=-1)
		K_BUDDY = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_NEXT_CHAT_TAB",file_mem,key_file_size,0))!=-1)
		K_NEXT_CHAT_TAB = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_PREV_CHAT_TAB",file_mem,key_file_size,0))!=-1)
		K_PREV_CHAT_TAB = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_RULES",file_mem,key_file_size,0))!=-1)
		K_RULES = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL1",file_mem,key_file_size,0))!=-1)
		K_SPELL1 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL2",file_mem,key_file_size,0))!=-1)
		K_SPELL2 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL3",file_mem,key_file_size,0))!=-1)
		K_SPELL3 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL4",file_mem,key_file_size,0))!=-1)
		K_SPELL4 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL5",file_mem,key_file_size,0))!=-1)
		K_SPELL5 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL6",file_mem,key_file_size,0))!=-1)
		K_SPELL6 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL7",file_mem,key_file_size,0))!=-1)
		K_SPELL7 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL8",file_mem,key_file_size,0))!=-1)
		K_SPELL8 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL9",file_mem,key_file_size,0))!=-1)
		K_SPELL9 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL10",file_mem,key_file_size,0))!=-1)
		K_SPELL10 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL11",file_mem,key_file_size,0))!=-1)
		K_SPELL11 = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_SPELL12",file_mem,key_file_size,0))!=-1)
		K_SPELL12 = parse_key_string(&file_mem[t]);
	if((t = get_string_occurance("#K_TABCOMPLETE",file_mem,key_file_size,0)) != -1)
		K_TABCOMPLETE = parse_key_string(&file_mem[t]);
	if((t = get_string_occurance("#K_WINDOWS_ON_TOP",file_mem,key_file_size,0)) != -1)
		K_WINDOWS_ON_TOP = parse_key_string(&file_mem[t]);
	if((t = get_string_occurance("#K_MARKFILTER",file_mem,key_file_size,0)) != -1)
		K_MARKFILTER = parse_key_string(&file_mem[t]);
	if((t = get_string_occurance("#K_OPAQUEWIN",file_mem,key_file_size,0)) != -1)
		K_OPAQUEWIN = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_GRAB_MOUSE",file_mem,key_file_size,0))!=-1)
		K_GRAB_MOUSE = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_FIRST_PERSON",file_mem,key_file_size,0))!=-1)
		K_FIRST_PERSON = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_EXTEND_CAM",file_mem,key_file_size,0))!=-1)
		K_EXTEND_CAM = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_EMOTES",file_mem,key_file_size,0))!=-1)
		K_EMOTES = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_RANGINGWIN",file_mem,key_file_size,0))!=-1)
		K_RANGINGWIN = parse_key_string(&file_mem[t]);

	free(file_mem);
}
#endif // FASTER_MAP_LOAD

// Returns (in the buffer provided) a string describing the specified keydef.
const char *get_key_string(Uint32 keydef, char *buf, size_t buflen)
{
	char base = keydef & 0xFF;
	char *mod = "";
	if (keydef & CTRL)
		mod = "ctrl-";
	else if (keydef & ALT)
		mod = "alt-";
	else if (keydef & SHIFT)
		mod = "shift-";
	safe_snprintf(buf, buflen, "%s%c", mod, base);
	return buf;
}
