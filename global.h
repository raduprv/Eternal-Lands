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

#ifdef	WINDOWS
	#include <windows.h>
	#ifdef	_MSC_VER	// now we do test for VC
		// Lachesis: Make sure snprintf is declared before we #define it to be something else, 
		// else we'll eventually break C++ headers that use it
		#include <stdio.h>

		#define stat _stat
		#define	snprintf sane_snprintf
		#define strncasecmp _strnicmp
		#define strcasecmp _stricmp
		#if _MSC_VER < 1400 // VC 2003 needs these defines, VC 2005 will error with them included
			#define atan2f atan2
			#define acosf acos
			#define ceilf ceil
			#define floorf floor
			#define fabsf fabs
		#endif  // _MSC_VER < 1400
		#define rint(X) floor(X+0.5f)
	#endif
	#ifdef __MINGW32__
		// Lachesis: Make sure snprintf is declared before we #define it to be something else, 
		// else we'll eventually break C++ headers that use it
		#include <stdio.h>

		#define snprintf sane_snprintf
	#endif
#elif defined(OSX)
	#ifndef NO_MUSIC
		#define __MACOSX__	//necessary for Ogg on Macs
	#endif
	#ifdef __BIG_ENDIAN__
		#define EL_BIG_ENDIAN
	#endif
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
#endif //WINDOWS

#if defined(CUSTOM_UPDATE) && !defined(AUTO_UPDATE)
#undef CUSTOM_UPDATE
#endif

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

#ifdef __cplusplus
extern "C" {
#endif
#ifdef X86_64
typedef long int point;
#else
typedef int point;
#endif

extern Uint32 cur_time, last_time; /*!< timestamps to check whether we need to resync */
#ifdef __cplusplus
} // extern "C"
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
#include "map.h"
#include "reflection.h"
#include "shadows.h"
#include "particles.h"
#include "spells.h"
#ifndef MAP_EDITOR
#include "sound.h"
#endif  // MAP_EDITOR
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
#include "session.h"
#include "serverpopup.h"
#ifdef MINES
 #include "mines.h"
#endif // MINES

#ifdef COUNTERS
#include "counters.h"
#endif

#ifdef NOTEPAD
	#include "notepad.h"
#endif //NOTEPAD
#ifdef MINIMAP
	#include "minimap.h"
#endif  //MINIMAP
#include "alphamap.h"
#include "highlight.h"

#include "vmath.h"

#include "framebuffer.h"
#include "bbox_tree.h"

#ifdef NEW_FILE_IO
#include "io/elpathwrapper.h"
#endif //NEW_FILE_IO

#ifdef	OLC
	#include "olc.h"
#endif	//OLC

#endif
