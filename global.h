#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef WINDOWS
#include <windows.h>
#include <al.h>
#include <alut.h>
#ifndef	GNUC	// or should we test for VC
#define	snprintf _snprintf
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define atan2f atan2
#define acosf acos
#endif
#elif defined(OSX)
#include <OpenAL/alut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#else
#include <AL/al.h>
#include <AL/alut.h>
#endif //WINDOWS

#ifndef OSX
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

//SDL has to be before ogg
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_thread.h>

#ifndef	NO_MUSIC
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#endif	//NO_MUSIC

#include "cache.h"
#include "encyclopedia.h"
#include "questlog.h"
#include "buddy.h"
#include "knowledge.h"
#include "elc_private.h"
#include "asc.h"
#include "md2.h"
#include "actors.h"
#include "new_actors.h"
#include "actor_scripts.h"
#include "pathfinder.h"
#include "e3d.h"
#include "errors.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "tiles.h"
#include "lights.h"
#include "client_serv.h"
#include "multiplayer.h"
#include "text.h"
#include "interface.h"
#include "hud.h"
#include "map_io.h"
#include "reflection.h"
#include "shadows.h"
#include "particles.h"
#include "spells.h"
#include "sound.h"
#include "ignore.h"
#include "filter.h"
#include "weather.h"
#include "stats.h"
#include "items.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "colors.h"
#include "console.h"
#include "cursors.h"
#include "events.h"
#include "font.h"
#include "gl_init.h"
#include "manufacture.h"
#include "misc.h"
#include "paste.h"
#include "textures.h"
#include "trade.h"
#include "new_character.h"
#include "init.h"
#include "pm_log.h"
#include "options.h"
#include "translate.h"
#include "elconfig.h"
#ifdef CAL3D
#include "cal3dwrap.h"
#endif

//some prototypes, that won't fit somewhere else
int SphereInFrustum(float x, float y, float z, float radius);
int check_tile_in_frustrum(float x,float y);
void read_command_line(); //from main.c

#endif

