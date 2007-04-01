#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define true 1
#define false 0
#ifdef WINDOWS
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifndef min
	#define min(x, y) (x<y?x:y)
#endif //min
#ifndef max
	#define max(x, y) (x>y?x:y)
#endif //max

#if defined (_MSC_VER) || defined (__MINGW32__)
	#define snprintf sane_snprintf
#else
	// should probably not be used
	#ifndef	__GNUC__	// or should we test for VC
		#define	snprintf _snprintf
	#endif
#endif

#include <SDL.h>
#include <SDL_endian.h>
//#include <SDL_opengl.h>
#include	<GL/gl.h>
#include	<GL/glu.h>
#include	<GL/glext.h>

#ifdef LINUX
#include <gtk/gtk.h>
#include "gui.h"
#include "gui_callbacks.h"
#include "gui_support.h"
#endif

#ifdef	ZLIB
#include	<zlib.h>
#endif

#include "../elc/cache.h"
#include "../elc/translate.h"
#include "../elc/elconfig.h"
#include "colors.h"
#include "errors.h"
#include "init.h"
#include "../elc/asc.h"
#include "e3d.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "tiles.h"
#include "lights.h"
#include "interface.h"
#include "editor.h"
#include "map_io.h"
#include "shadows.h"
#include "reflection.h"
#include "draw_scene.h"
#include "browser.h"
#include "events.h"
#include "frustum.h"
#include "misc.h"
#include "../elc/client_serv.h"
#include "textures.h"
#include "font.h"
#include "elwindows.h"
#include "o3dow.h"
#include "replace_window.h"
#include "edit_window.h"
#include "../elc/particles.h"
#include "particles_window.h"
#include "eye_candy_window.h"
#include "gl_init.h"
#define sector_size_x 15
#define sector_size_y 15

#ifdef X86_64
typedef long int point;
#else
typedef int point;
#endif

extern int font_text;

extern int map_meters_size_x;
extern int map_meters_size_y;

extern int elwin_mouse;
extern int window_width;
extern int window_height;
extern int bpp;


extern Uint32 cur_time, last_time;

extern float camera_x_end_point;
extern float camera_z_end_point;
extern int last_texture;

extern float gcr,gcg,gcb;

//debug
extern int texture2;
extern int texture3;
//end of debug

extern  Uint8 *e3d_file_mem;
extern  Uint8 *handle_e3d_file_mem;

typedef struct
{
	float r1;
	float g1;
	float b1;
	float r2;
	float g2;
	float b2;
	float r3;
	float g3;
	float b3;
	float r4;
	float g4;
	float b4;

} color_rgb;

extern color_rgb colors_list[25];


//colors
#define c_red 0
#define c_blue 1
#define c_green 2
#define c_yellow 3
#define c_orange 4
#define c_violet 5
#define c_light_red 6
#define c_light_blue 7
#define c_light_green 8
#define c_dark_red 9
#define c_dark_blue 10
#define c_dark_green 11
#define c_white 12
#define c_dark 13
#define c_gray 14
#define c_purple 15
#define c_brown 16
#define c_green_yellow 17
#define c_silver 18
#define c_gold 19
#define c_steel 20
#define c_bronze 21

#ifdef EL_BIG_ENDIAN
	#define SwapLEFloat(X) SwapFloat(X)
#else
	#define SwapLEFloat(X) (X)
#endif

extern int have_multitexture;
extern int have_arb_compression;
extern int have_s3_compression;
extern int poor_man;
extern int ground_detail_text;

extern float clouds_movement_u;
extern float clouds_movement_v;
extern Uint32 last_clear_clouds;
extern float texture_scale;
extern int clouds_shadows;

extern int icons_text;

extern int video_mode;
extern int auto_save_time;
extern int limit_fps;

#ifndef LINUX //extensions
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;
#endif

#ifndef POINT_SIZE_MIN_ARB
#define POINT_SIZE_MIN_ARB 0x8126
#endif

#ifndef COMPRESSED_RGBA_ARB
#define COMPRESSED_RGBA_ARB                             0x84EE
#endif

#ifndef COMPRESSED_RGBA_S3TC_DXT5_EXT
#define COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#endif

#define CHECK_GL_ERRORS()       //NOP

#endif
