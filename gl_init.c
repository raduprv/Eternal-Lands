#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "gl_init.h"
#include "asc.h"
#include "bbox_tree.h"
#include "cursors.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elconfig.h"
#include "errors.h"
#include "framebuffer.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "lights.h"
#include "mapwin.h"
#include "textures.h"
#include "translate.h"
#include "io/e3d_io.h"
#include "eye_candy_wrapper.h"
#include "minimap.h"
#include "sky.h"
#include "shader/shader.h"
#include "actor_init.h"
#ifdef	FSAA
#include "fsaa/fsaa.h"
#endif	/* FSAA */

Uint32 flags;

int window_width=640;
int window_height=480;

SDL_Window *el_gl_window = NULL;
static SDL_GLContext el_gl_context = NULL;
static SDL_Surface *icon_bmp = NULL;

int desktop_width;
int desktop_height;

int bpp=0;
int have_stencil=1;
int video_mode;
int video_user_width;
int video_user_height;
int disable_window_adjustment;
int full_screen;

int use_compiled_vertex_array = 0;
int use_vertex_buffers = 0;
int use_frame_buffer = 0;
int use_mipmaps = 0;
int use_draw_range_elements = 1;
float anisotropic_filter = 1.0f;
int disable_gamma_adjust = 0;
float gamma_var = 1.00f;
float perspective = 0.15f;
float near_plane = 0.1f; // don't cut off anything
float far_plane = 100.0;   // LOD helper. Cull distant objects. Lower value == higher framerates.
float far_reflection_plane = 100.0;   // LOD helper. Cull distant reflected objects. Lower value == higher framerates.
int gl_extensions_loaded = 0;

struct list {
	int i;
	struct list * next;
} * list;

void APIENTRY Emul_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	glDrawElements(mode, count, type, indices);
}

void setup_video_mode(int fs, int mode)
{
	/* Video mode 0 is user defined size (via video_user_width and video_user_height)
	 * Video mode 1 and above are defined in the video_modes array where mode 1 is at position 0
	 * Therefore we heve to adjust the index by one
	 */
	int index = mode - 1;

	/* Safe fallback
	 * If the user select an invalid mode (like a wrong number in the config file) fallback
	 * to safe 640x480 mode
	 */
	if (index < 0 || index >= video_modes_count)
		index = 0;

	if (fs) // Fullscreen
	{
		if (mode == 0)
		{
			window_width = video_user_width;
			window_height = video_user_height;
			bpp = 0;
		} else {
			window_width = video_modes[index].width;
			window_height = video_modes[index].height;
			bpp = video_modes[index].bpp;
		}
	} 
	else // Windowed mode
	{
		int new_width = video_modes[index].width;
		int new_height = video_modes[index].height;

		if (mode == 0)
		{
			new_width = video_user_width;
			new_height = video_user_height;
		} 
		else if (!disable_window_adjustment)
		{
#ifdef WINDOWS
			// Window size magic:
			// Try to get the work area and adjust the window to that size minus the border size

			HWND hwnd;
			HMONITOR monitor;
			MONITORINFO monitorInfo;
			int monitor_width = 0;
			int monitor_height = 0;

			hwnd = GetActiveWindow();
			// Get the monitor closest to the window (if we already have one)
			monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

			// Get the size of the work area
			monitorInfo.cbSize = sizeof(monitorInfo);
			GetMonitorInfo(monitor, &monitorInfo);
			monitor_width = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
			monitor_height = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

			// Clip the window size to the work area
			if (new_width >= monitor_width)
				new_width = monitor_width;
			if (new_height >= monitor_height)
				new_height = monitor_height;

			// try to move the window to make everything visible
			if (hwnd != NULL)
			{
				WINDOWPLACEMENT wpl;
				int dx, dy;

				wpl.length = sizeof(wpl);
				GetWindowPlacement(hwnd, &wpl);

				dx = wpl.rcNormalPosition.left + new_width - monitorInfo.rcWork.right;
				dy = wpl.rcNormalPosition.top + new_height - monitorInfo.rcWork.bottom;

				if (dx < 0) dx = 0;
				if (dy < 0) dy = 0;

				if (dx || dy) {
					wpl.rcNormalPosition.left -= dx;
					wpl.rcNormalPosition.top -= dy;
					wpl.showCmd = SW_SHOWNORMAL;
					SetWindowPlacement(hwnd, &wpl);
				}
			}

			// Adjust the window size to fit into the border and title bar
			new_width -= 2 * GetSystemMetrics(SM_CXFIXEDFRAME);
			new_height -= 2 * GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
#else
			new_width -= 10;
			new_height -= 55;
#endif
		}

		if (window_width != new_width || window_height != new_height)
		{
			char modestr[100];
			char str[100];
			safe_snprintf(modestr, sizeof(modestr), "%dx%d", new_width, new_height);
			safe_snprintf(str, sizeof(str), window_size_adjusted_str, modestr);
			LOG_TO_CONSOLE(c_yellow1,str);
			LOG_DEBUG("%s",str);
		}

		window_width = new_width;
		window_height = new_height;
		bpp = 0; // autodetect
	}
#ifndef WINDOWS
	bpp=0;//under X, we can't change the desktop BPP
#endif
}

static void load_window_icon(void)
{
	char *icon_name = "icon.bmp";
	size_t str_len = strlen(datadir) + strlen(icon_name) + 1;
	char *str_buf = malloc(str_len);
	safe_strncpy(str_buf, datadir, str_len);
	safe_strcat(str_buf, icon_name, str_len);
	icon_bmp = SDL_LoadBMP(str_buf);
	if (icon_bmp == NULL)
		LOG_ERROR("Failed to load window icon: %s\n",str_buf);
	else
		SDL_SetWindowIcon(el_gl_window, icon_bmp);
	free(str_buf);
}

void init_video()
{
	char str[400];
	int rgb_size[3];

	setup_video_mode(full_screen, video_mode);

	/* Detect the display depth */
	if(!bpp)
		{
			SDL_DisplayMode current;
			SDL_GetCurrentDisplayMode(0, &current);
			if ( SDL_BITSPERPIXEL(current.format) <= 8 )
				{
					bpp = 8;
				}
			else
				if ( SDL_BITSPERPIXEL(current.format) <= 16 )
					{
						bpp = 16;  /* More doesn't seem to work */
					}
				else bpp=32;
		}

	//adjust the video mode accordingly
	if (video_mode == 0)
	{
		//do nothing
	} else if(bpp==16) {
		if(!(video_mode%2))
			video_mode-=1;
	} else {
		if(video_mode%2)
			video_mode+=1;
	}

	/* Initialize the display */
	switch (bpp) {
	case 8:
		rgb_size[0] = 2;
		rgb_size[1] = 3;
		rgb_size[2] = 3;
		break;
	case 15:
	case 16:
		rgb_size[0] = 5;
		rgb_size[1] = 5;
		rgb_size[2] = 5;
		break;
	default:
		rgb_size[0] = 8;
		rgb_size[1] = 8;
		rgb_size[2] = 8;
		break;
	}

	//    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and 5-5-5 RGB for 16-bit screens
#ifndef OSX
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
#endif

	// enable V-SYNC, choosing active as a preference
	if (SDL_GL_SetSwapInterval(-1) < 0)
		SDL_GL_SetSwapInterval(1);

	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#ifdef	FSAA
	if (fsaa > 1)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	flags = SDL_WINDOW_OPENGL;
	if(full_screen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

#ifdef	FSAA
	if (fsaa > 1)
	{
		el_gl_window = SDL_CreateWindow("Eternal Lands", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (full_screen)?0:window_width, (full_screen)?0:window_height, flags);
		if (el_gl_window == NULL)
		{
			safe_snprintf(str, sizeof(str), "Can't use fsaa mode x%d, disabling it.", fsaa);
			LOG_TO_CONSOLE(c_yellow1, str);
			LOG_WARNING("%s\n", str);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			fsaa = 0;
		}
	}
#endif	/* FSAA */

	//try to find a stencil buffer (it doesn't always work on Linux)
	if (el_gl_window == NULL)
	{
		el_gl_window = SDL_CreateWindow("Eternal Lands", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (full_screen)?0:window_width, (full_screen)?0:window_height, flags);
		if (el_gl_window == NULL)
		{
			LOG_TO_CONSOLE(c_red1,no_hardware_stencil_str);
			LOG_ERROR("%s\n",no_hardware_stencil_str);
			if(bpp!=32)
			{
                   LOG_TO_CONSOLE(c_grey1,suggest_24_or_32_bit);
                   LOG_ERROR("%s\n",suggest_24_or_32_bit);
            }
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,0);
			el_gl_window = SDL_CreateWindow("Eternal Lands", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (full_screen)?0:window_width, (full_screen)?0:window_height, flags);
			if (el_gl_window == NULL)
			{
				LOG_ERROR("%s: %s\n", fail_opengl_mode, SDL_GetError());
				SDL_Quit();
				exit(1);
			}
			have_stencil=0;
		}
	}

	el_gl_context = SDL_GL_CreateContext(el_gl_window);
	SDL_GetWindowSize(el_gl_window, &window_width, &window_height);

	SDL_SetWindowMinimumSize(el_gl_window, 640,  480);
	SDL_SetWindowResizable(el_gl_window, SDL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);
	glClearStencil(0);

#ifdef ANTI_ALIAS
	if (anti_alias) {
		glHint(GL_POINT_SMOOTH_HINT,   GL_NICEST);	
		glHint(GL_LINE_SMOOTH_HINT,    GL_NICEST);	
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);	
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
	} else {
		glHint(GL_POINT_SMOOTH_HINT,   GL_FASTEST);	
		glHint(GL_LINE_SMOOTH_HINT,    GL_FASTEST);	
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);	
		glDisable(GL_POINT_SMOOTH);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
	}
#endif
	last_texture=-1;	//no active texture
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	load_window_icon();

	check_options();
}

#ifdef	GL_EXTENSION_CHECK
void evaluate_extension()
{
	char str[1024];
	int has_arb_texture_env_add;
	int has_arb_texture_env_crossbar;
	int has_arb_texture_rectangle;
	int has_arb_fragment_shader_shadow;
	int has_ati_fragment_shader;
	int has_ati_texture_env_combine3;
	int has_nv_texture_env_combine4;
	int has_nv_texture_shader;
	int has_nv_texture_shader2;
	int options;
	char* extensions;

	extensions = (char*)glGetString(GL_EXTENSIONS);

	has_arb_texture_env_add = strstr(extensions, "GL_ARB_texture_env_add") > 0;
	has_arb_texture_env_crossbar = strstr(extensions, "GL_ARB_texture_env_crossbar") > 0;
	has_arb_texture_rectangle = strstr(extensions, "GL_ARB_texture_rectangle") > 0;
	has_arb_fragment_shader_shadow = strstr(extensions, "GL_ARB_fragment_program_shadow") > 0;
	has_ati_fragment_shader = strstr(extensions, "GL_ATI_fragment_shader") > 0;
	has_ati_texture_env_combine3 = strstr(extensions, "GL_ATI_texture_env_combine3") > 0;
	has_nv_texture_env_combine4 = strstr(extensions, "GL_NV_texture_env_combine4") > 0;
	has_nv_texture_shader = strstr(extensions, "GL_NV_texture_shader") > 0;
	has_nv_texture_shader2 = strstr(extensions, "GL_NV_texture_shader2") > 0;

	options = (get_texture_units() >= 2) && has_arb_texture_env_add &&
		have_extension(arb_texture_env_combine) && have_extension(arb_vertex_program) &&
		have_extension(arb_texture_compression) && have_extension(arb_vertex_buffer_object) &&
		have_extension(ext_texture_compression_s3tc);

	if (!options)
	{
		safe_snprintf(str,sizeof(str),"%s%s%s","Your graphic card/driver don't support the minimum",
			" requirements for the next EL release. Please upgrade your driver.",
			" If this don't help, you need a better graphic card.");
        LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR("%s\n",str);
		return;
	}

	if (!have_extension(arb_vertex_shader) || (have_extension(arb_fragment_program) &&
		!have_extension(arb_fragment_shader)) || (has_ati_fragment_shader &&
		!have_extension(arb_fragment_program) && !have_extension(arb_fragment_shader)))
	{
        safe_snprintf(str,sizeof(str),"Please update your graphic card driver!");
		LOG_TO_CONSOLE(c_yellow1, str);
		LOG_WARNING("%s\n",str);
	}

	options = ((has_ati_texture_env_combine3 && has_arb_texture_env_crossbar) ||
		has_nv_texture_env_combine4) && (get_texture_units() >= 4) &&
		have_extension(ext_draw_range_elements) && have_extension(arb_shadow) &&
		have_extension(arb_point_parameters) && have_extension(arb_point_sprite);

	if (!options)
	{
		safe_snprintf(str,sizeof(str),"%s%s%s","Your graphic card supports the absolute minimum",
			" requirements for the next EL release, but don't expect that you can use",
			" all features.");
        LOG_TO_CONSOLE(c_yellow1, str);
		LOG_DEBUG("%s\n",str);
        
	}
	else
	{
		options = (has_ati_fragment_shader || (has_nv_texture_shader &&
			has_nv_texture_shader2)) && have_extension(arb_occlusion_query) &&
			has_arb_texture_rectangle && have_extension(ext_framebuffer_object);
		if (!options)
		{
            safe_snprintf(str,sizeof(str),"%s%s","Your graphic card supports the default ",
				"requirements for the next EL release.");
			LOG_TO_CONSOLE(c_green2, str);
			LOG_DEBUG("%s\n",str);
		}
		else
		{
			if (have_extension(arb_fragment_shader) &&
				have_extension(arb_shader_objects) &&
				have_extension(arb_vertex_shader) &&
				have_extension(arb_shading_language_100))
			{
				safe_snprintf(str,sizeof(str),"%s%s","Your graphic card supports all ",
					"features EL will use in the future.");
                LOG_TO_CONSOLE(c_blue2, str);
                LOG_DEBUG("%s\n",str);
			}
			else
			{
                safe_snprintf(str,sizeof(str),"%s%s","Your graphic card supports more than the",
				" default requirements for the next EL release.");
				LOG_TO_CONSOLE(c_blue2, str);
                LOG_DEBUG("%s\n",str);
			}
		}
	}
}
#endif	//GL_EXTENSION_CHECK

void init_gl_extensions()
{
	char str[1024];

	init_opengl_extensions();

	/*	GL_ARB_multitexture			*/
	if (have_extension(arb_multitexture))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_multitexture");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_multitexture");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_multitexture			*/

	/*	GL_ARB_texture_env_combine		*/
	if (have_extension(arb_texture_env_combine))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_texture_env_combine");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_texture_env_combine");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_texture_env_combine		*/

	/*	GL_EXT_compiled_vertex_array		*/
	if (have_extension(ext_compiled_vertex_array))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_compiled_vertex_array");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_compiled_vertex_array");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_compiled_vertex_array		*/

	/*	GL_ARB_point_sprite			*/
	if (have_extension(arb_point_sprite))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_point_sprite");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_point_sprite");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_point_sprite		*/

	/*	GL_ARB_texture_compression		*/
	if (have_extension(arb_texture_compression))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_texture_compression");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_texture_compression");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_texture_compression		*/

	/*	GL_EXT_texture_compression_s3tc		*/
	if (have_extension(ext_texture_compression_s3tc))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_texture_compression_s3tc");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_texture_compression_s3tc");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_texture_compression_s3tc		*/
	
	/*	GL_SGIS_generate_mipmap			*/
	if (have_extension(sgis_generate_mipmap))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_SGIS_generate_mipmap");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_SGIS_generate_mipmap			*/

	/*	GL_ARB_shadow				*/
	if (have_extension(arb_shadow))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_shadow");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_shadow");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_shadow				*/

	/*	GL_ARB_vertex_buffer_object		*/
	if (have_extension(arb_vertex_buffer_object))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_vertex_buffer_object");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_vertex_buffer_object");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_vertex_buffer_object		*/

	/*	GL_EXT_framebuffer_object		*/
	if (have_extension(ext_framebuffer_object))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_framebuffer_object");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_framebuffer_object");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_framebuffer_object		*/
	
	/*	GL_EXT_draw_range_elements		*/
	if (have_extension(ext_draw_range_elements))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_draw_range_elements");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_draw_range_elements");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_draw_range_elements		*/

	/*	GL_ARB_texture_non_power_of_two		*/
	if (have_extension(arb_texture_non_power_of_two))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_texture_non_power_of_two");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_texture_non_power_of_two");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_texture_non_power_of_two		*/

	/*	GL_ARB_fragment_program			*/
	if (have_extension(arb_fragment_program))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_fragment_program");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_fragment_program");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_fragment_program			*/

	/*	GL_ARB_vertex_program			*/
	if (have_extension(arb_vertex_program))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_vertex_program");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_vertex_program");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_vertex_program			*/

	/*	GL_ARB_fragment_shader			*/
	if (have_extension(arb_fragment_shader))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_fragment_shader");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_fragment_shader");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_fragment_shader			*/

	/*	GL_ARB_vertex_shader			*/
	if (have_extension(arb_vertex_shader))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_vertex_shader");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_vertex_shader");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_vertex_shader			*/

	/*	GL_ARB_shader_objects			*/
	if (have_extension(arb_shader_objects))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_shader_objects");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_shader_objects");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_shader_objects			*/

	/*	GL_ARB_shading_language_100		*/
	if (have_extension(arb_shading_language_100))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ARB_shading_language_100");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_shading_language_100");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_shading_language_100		*/

	/*	GL_ARB_texture_mirrored_repeat		*/
	if (have_extension(arb_texture_mirrored_repeat))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found_not_used, "GL_ARB_texture_mirrored_repeat");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_texture_mirrored_repeat");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_texture_mirrored_repeat		*/

	/*	GL_ARB_texture_rectangle		*/
	if (have_extension(arb_texture_rectangle))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found_not_used, "GL_ARB_texture_rectangle");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ARB_texture_rectangle");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ARB_texture_rectangle		*/

	/*	GL_EXT_fog_coord			*/
	if (have_extension(ext_fog_coord))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found_not_used, "GL_EXT_fog_coord");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_fog_coord");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_fog_coord			*/

	/*	GL_ATI_texture_compression_3dc		*/
	if (have_extension(ati_texture_compression_3dc))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_ATI_texture_compression_3dc");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_ATI_texture_compression_3dc");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_ATI_texture_compression_3dc		*/

	/*	GL_EXT_texture_compression_latc		*/
	if (have_extension(ext_texture_compression_latc))
	{
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_texture_compression_latc");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_texture_compression_latc");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_texture_compression_latc		*/

	/*	GL_EXT_texture_filter_anisotropic	*/
	if (have_extension(ext_texture_filter_anisotropic))
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropic_filter);
		safe_snprintf(str, sizeof(str), gl_ext_found, "GL_EXT_texture_filter_anisotropic");
		LOG_TO_CONSOLE(c_green2, str);
		LOG_DEBUG("%s\n",str);
	}
	else
	{
		anisotropic_filter = 1.0f;
		safe_snprintf(str, sizeof(str), gl_ext_not_found, "GL_EXT_texture_filter_anisotropic");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_DEBUG("%s\n",str);
	}
	/*	GL_EXT_texture_filter_anisotropic	*/

#if	0
	// Disabled because of bad drivers
	if (have_extension(ext_framebuffer_object))
	{
		check_fbo_formats();
	}
#endif
	init_shaders();

#ifdef	GL_EXTENSION_CHECK
	evaluate_extension();
#endif	//GL_EXTENSION_CHECK

	gl_extensions_loaded = 1;

	CHECK_GL_ERRORS();
}

void resize_root_window()
{
	float window_ratio;
	//float hud_x_adjust=0;
	//float hud_y_adjust=0;

	if (window_height==0)window_height=1;			// Prevent A Divide By Zero

	//glViewport(0, hud_y, window_width-hud_x, window_height);	// Reset The Current Viewport
	//glViewport(0, 0, window_width-hud_x, -(window_height-hud_y));	// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	window_ratio=(GLfloat)(window_width-hud_x)/(GLfloat)(window_height-hud_y);

	//hud_y_adjust=(2.0/window_height)*hud_y;
	//hud_x_adjust=(2.0/window_width)*hud_x;
	//Setup matrix for the sky. If we don't do this the sky looks unhinged when perspective changes.
	glLoadIdentity();
	glFrustum(-perspective*window_ratio*near_plane,
			   perspective*window_ratio*near_plane,
			  -perspective*near_plane,
			   perspective*near_plane,
			   near_plane, 1000.0);
	glGetDoublev(GL_PROJECTION_MATRIX, skybox_view);
	glLoadIdentity(); // Reset The Projection Matrix

	//new zoom
	if (isometric)
	{
		glOrtho( -1.0*zoom_level*window_ratio, 1.0*zoom_level*window_ratio, -1.0*zoom_level, 1.0*zoom_level, -near_plane*zoom_level, 60.0 );
	}
	else
	{
		glFrustum(-perspective*window_ratio*near_plane,
				   perspective*window_ratio*near_plane,
				  -perspective*near_plane,
				   perspective*near_plane,
				  near_plane, far_plane);
		if (!first_person)
		{
			glTranslatef(0.0, 0.0, zoom_level*camera_distance);
			glTranslatef(0.0, 0.0, -zoom_level/perspective);
		}
	}

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix
	last_texture=-1;	//no active texture
}

int switch_video(int mode, int full_screen)
{
	video_mode=mode;
	setup_video_mode(full_screen, mode);
	SDL_RestoreWindow(el_gl_window);
	if (full_screen)
		SDL_SetWindowFullscreen(el_gl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
	{
		SDL_SetWindowFullscreen(el_gl_window, 0);
		SDL_SetWindowSize(el_gl_window, window_width, window_height);
		SDL_SetWindowMinimumSize(el_gl_window, 640,  480);
		SDL_SetWindowResizable(el_gl_window, SDL_TRUE);
	}
	resize_all_root_windows(window_width, window_height);
	return 1;
}

void toggle_full_screen()
{
	full_screen=!full_screen;
	switch_video(video_mode, full_screen);
}

int print_gl_errors(const char *file, int line)
{
	int	glErr, anyErr=GL_NO_ERROR;

	while ((glErr=glGetError()) != GL_NO_ERROR )
	 {
		anyErr=glErr;
//#ifdef	GLUT
//FIXME: this appears to be a GLU call, not GLUT, and we link with GLU normally...
//unless this causes an error on some other compiler, the commented parts should be removed
		log_error(file, line, "OpenGL %s", gluErrorString(glErr));
//#else
//		log_error_detailed("OpenGL error %d", file, func, line, glErr);
//#endif // GLUT
	}
	return anyErr;
}

