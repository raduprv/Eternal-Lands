#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	"global.h"
#include	"keys.h"

// default definitions for keys
Uint32 K_CAMERAUP=273;
Uint32 K_CAMERADOWN=274;
Uint32 K_ZOOMOUT=281;
Uint32 K_ZOOMIN=280;
Uint32 K_TURNLEFT=277;
Uint32 K_TURNRIGHT=127;
Uint32 K_ADVANCE=278;
Uint32 K_HEALTHBAR=ALT|'h';
Uint32 K_VIEWNAMES=ALT|'n';
Uint32 K_VIEWHP=ALT|'b';
Uint32 K_STATS=CTRL|'a';
Uint32 K_WALK=CTRL|'w';
Uint32 K_LOOK=CTRL|'l';
Uint32 K_USE=CTRL|'u';
Uint32 K_OPTIONS=CTRL|'o';
Uint32 K_REPEATSPELL=CTRL|'r';
Uint32 K_SIGILS=CTRL|'s';
Uint32 K_MANUFACTURE=CTRL|'m';
Uint32 K_ITEMS=CTRL|'i';
Uint32 K_MAP=9;
Uint32 K_ROTATELEFT=276;
Uint32 K_ROTATERIGHT=275;
Uint32 K_FROTATELEFT=SHIFT|276;
Uint32 K_FROTATERIGHT=SHIFT|275;
Uint32 K_BROWSER=283;
Uint32 K_ESCAPE=27;
Uint32 K_CONSOLE=282;
Uint32 K_SHADOWS=284;
Uint32 K_KNOWLEDGE=CTRL|'k';
Uint32 K_ENCYCLOPEDIA=CTRL|'e';
Uint32 K_ITEM1=CTRL|'1';
Uint32 K_ITEM2=CTRL|'2';
Uint32 K_ITEM3=CTRL|'3';
Uint32 K_ITEM4=CTRL|'4';
Uint32 K_ITEM5=CTRL|'5';
Uint32 K_ITEM6=CTRL|'6';
Uint32 K_VIEWTEXTASOVERTEXT=ALT|'o';
Uint32 K_AFK=CTRL|ALT|'a';


// load the dynamic definitions for keys
void read_key_config()
{
	FILE *f = NULL;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	struct stat key_file;
	int key_file_size,t;

#ifndef WINDOWS
	char key_ini[256];
	strcpy(key_ini, configdir);
	strcat(key_ini, "key.ini");
	f=fopen(key_ini,"rb"); //try to load local settings
	if(!f) //use global settings
		{
			strcpy(key_ini, datadir);
			strcat(key_ini, "key.ini");
			f=fopen(key_ini,"rb");
		}

	stat(key_ini,&key_file);
#else
	f=fopen("key.ini","rb");
	stat("key.ini",&key_file);
#endif

	if(!f)return; //take the defaults

	key_file_size = key_file.st_size;
	file_mem = (Uint8 *) calloc(key_file_size+2, sizeof(Uint8));
	file_mem_start=file_mem;
	fread (file_mem, 1, key_file_size+1, f);

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
	if((t=get_string_occurance("#K_VIEWTEXTASOVERTEXT",file_mem,key_file_size,0))!=-1)
		K_VIEWTEXTASOVERTEXT = parse_key_string(&file_mem[t]);
	if((t=get_string_occurance("#K_AFK",file_mem,key_file_size,0))!=-1)
		K_AFK = parse_key_string(&file_mem[t]);
}

Uint32 parse_key_string(char *s)
{
	char t1[100],t2[100],t3[100],t4[100];
	Uint32 key=0;
	*t1='#';
	*t4='#';
	*t3='#';
	*t2='#';
	sscanf(s,"%s %s %s %s",t1,t2,t3,t4);
	if(t1)
		add_key(&key,get_key_code(t1));

	if(*t2!='#'){
		add_key(&key,get_key_code(t2));
		if(*t3!='#'){
			add_key(&key,get_key_code(t3));
			if(*t4!='#')
				add_key(&key,get_key_code(t4));
		}
	}

	return key;
}


Uint16 get_key_code(char *key)
{
	int len=strlen(key);

	if(len==1)
			return tolower(key[0]);
	else
	{
		Uint32 crc=CRC32(key,len);
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


Uint32 CRC32(unsigned char *data, int len)
{
    unsigned int result=0;
    int i,j;
    unsigned char octet;

    for (i=0; i<len; i++){
        octet = *(data++);
        for (j=0; j<8; j++){
            if ((octet >> 7) ^ (result >> 31))
                result = (result << 1) ^ 0x04c11db7;
            else
                result = (result << 1);
            octet <<= 1;
        }
    }
    return ~result;
}


