/*!
 * \file
 * \ingroup init
 * \brief global include file
 */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <math.h>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif //M_PI

//only ever use WINDOWS anywhere else, in case we need to add another 'catch' to enable WINDOWS
#if defined(_WIN32) || defined(_WIN64)
	#ifndef	WINDOWS
		#define	WINDOWS
	#endif	//!WINDOWS
#endif	//_WIN32 || _WIN64
#ifdef OSX86	//Most i386 = PPC, but not all
	#define OSX
#endif

#ifdef	WINDOWS
	#include <windows.h>
	#include <al.h>
	#include <alut.h>
	#ifdef	_MSC_VER	// now we do test for VC
		#define stat _stat
		#define	snprintf sane_snprintf
		#define strncasecmp _strnicmp
		#define strcasecmp _stricmp
		#if _MSC_VER < 1400 // VC 2003 needs these defines, VC 2005 will error with them included
			#define atan2f atan2
			#define acosf acos
		#endif  // _MSC_VER < 1400
		#define ceilf ceil
		#define rint(X) floor(X+0.5f)
	#endif
	#ifdef __MINGW32__
		#define snprintf sane_snprintf
	#endif
#elif defined(OSX)
	#define NO_MUSIC
	#include <OpenAL/alut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
#else
	#include <AL/al.h>
	#include <AL/alut.h>
#endif //WINDOWS

#ifdef EL_BIG_ENDIAN
	#define SwapLEFloat(X) SwapFloat(X)
#else
	#define SwapLEFloat(X) (X)
#endif

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

#ifdef X86_64
typedef long int point;
#else
typedef int point;
#endif

#include "client_serv.h"
#ifdef MEMORY_DEBUG
 #include <stdlib.h> // make sure this is loaded before elmemory bc it defines malloc/calloc
 #include "elmemory.h"
#endif //MEMORY_DEBUG
#include "cal_types.h"
#include "cache.h"
#include "encyclopedia.h"
#include "questlog.h"
#include "buddy.h"
#include "knowledge.h"
#include "elc_private.h"
#include "asc.h"
//#include "md2.h"
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
#include "translate.h"
#include "elconfig.h"
#include "timers.h"
#include "rules.h"
#include "sector.h"
#include "help.h"
#include "tabs.h"
#include "gamewin.h"
#include "consolewin.h"
#include "mapwin.h"
#include "loginwin.h"
#include "openingwin.h"
#include "books.h"
#include "skills.h"
#include "chat.h"
#include "bags.h"
#include "storage.h"
#include "cal.h"
#include "kills.h"

#ifdef NOTEPAD
	#include "notepad.h"
#endif //NOTEPAD
#ifdef  MINIMAP
    #include "minimap.h"
#endif  //MINIMAP
#include "alphamap.h"
#include "highlight.h"

#include "vmath.h"

#ifdef TERRAIN
	#include "terrain.h"
	#include "normals.h"
#endif

#ifdef	USE_FRAMEBUFFER
	#include "framebuffer.h"
#endif
#ifdef	NEW_FRUSTUM
	#include "bbox_tree.h"
#endif
#endif
