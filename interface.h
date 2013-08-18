/*!
 * \file
 * \ingroup interfaces
 * \brief Handling of the different interfaces in EL.
 */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// How long a username is allowed to be. This define allows for the trailing NULL
#define MAX_USERNAME_LENGTH (15 + 1)

extern int have_a_map;  /*!< flag indicating whether a map is available or not */
extern int auto_camera; /*!< if this is true, we are using the auto camera */

/*! \name Action types */
/*! @{ */
#define ACTION_WALK 0
#define ACTION_LOOK 1
#define ACTION_USE 2
#define ACTION_USE_WITEM 3
#define ACTION_TRADE 4
#define ACTION_ATTACK 5
#define ACTION_WAND 6
/*! @} */

extern int action_mode; /*!< contains the current action type */

/*! \name Mouse movement coordinates 
 * @{ */
extern int mouse_x; /*!< current x coordinate of the mouse */
extern int mouse_y; /*!< current y coordinate of the mouse */

/*! \name Mouse buttons 
 * @{ */
extern int right_click; /*!< indicates the right mouse button was clicked */
extern int middle_click; /*!< indicates the middle mouse button was clicked */
extern int left_click; /*!< indicates the left mouse button was clicked */
/*! @} */

extern int view_health_bar; /*!< indicates whether we should display the health bar or not */
extern int view_ether_bar; /*!< indicates whether we should display the ethereal bar or not */
extern int view_names; /*!< indicates whether the names of actors should be displayed or not */
extern int view_hp; /*!< indicates whether the current/max material points of an actor should be displayed or not */
extern int view_ether; /*!< indicates whether the current/max ethereal points of an actor should be displayed or not */
extern int view_chat_text_as_overtext; /*!< if this is true, then any text an actor is saying will be additionally displayed in a bubble over its head */
extern int view_mode_instance; /*!< indicates if we have instance mode turned on, it shows monsters and our hp only, no other players, overwrites all other actor banner display options */
extern float view_mode_instance_banner_height; /*!< factor, we use to setup how high is banner above your toon when using view_mode_instance */

//instance mode banners config:
extern int im_creature_view_names; /*!< indicates whether the names of creatures should be displayed or not in instance mode*/
extern int im_creature_view_hp; /*!< indicates whether health numbers of creatures should be displayed or not in instance mode*/
extern int im_creature_view_hp_bar; /*!< indicates whether health bars of creatures should be displayed or not in instance mode*/
extern int im_creature_banner_bg; /*!< indicates whether creatures banners background should be displayed or not in instance mode*/
extern int im_other_player_view_names; /*!< indicates whether the names of  other players should be displayed or not in instance mode*/
extern int im_other_player_view_hp; /*!< indicates whether health numbers of  other players should be displayed or not in instance mode*/
extern int im_other_player_view_hp_bar; /*!< indicates whether health bars of  other players should be displayed or not in instance mode*/
extern int im_other_player_banner_bg; /*!< indicates whether other players banners background should be displayed or not in instance mode*/
extern int im_other_player_show_banner_on_damage;  /*!< indicates whether  other players name and hp should appear for a while if player gets damage in instance mode*/

extern char username_box_selected; /*!< true, if the cursor is currently in the username input field */
extern char password_box_selected; /*!< true, if the cursor is currently in the password input field */

extern char username_str[20]; /*!< the username of the actor */
extern char password_str[20]; /*!< the password of the actor */
extern char display_password_str[20]; /*!< a string that will be displayed when entering a password */

extern int cons_text;
extern int icons_text;
extern int hud_text;

extern int ranging_lock;
extern int auto_disable_ranging_lock;

/*!
 * A flag for a mode, that show whether a mode is supported and/or selected.
 */
typedef struct
{
	int supported; /*!< 0 if this mode is supported, else != 0 */
	int selected; /*!< 0 if this mode is selected, else != 0 */
}mode_flag;

/*!
 * Defintions for the video modes
 */
typedef struct
{
	int width;
	int height;
	int bpp;
	char *name;
	mode_flag flags;
} video_mode_t;

extern video_mode_t video_modes[]; /*!< global array of available video modes */
extern const int video_modes_count;

extern Uint32 click_time;

extern int ati_click_workaround; /*!< if non-zero, arbitrarily multiply the read depth value by 256 to hopefully get a more reasonable value */

/*!
 * Stores the start and end coordinates of a map
 */
struct draw_map
{
	short cont;
	unsigned short x_start;
	unsigned short y_start;
	unsigned short x_end;
	unsigned short y_end;       
	char *name;
};

extern int cur_map; /*!< id of the map we are currently on */
extern GLuint legend_text;

extern struct draw_map *continent_maps; /*!< global array of maps for the continents */

extern GLuint inspect_map_text;
extern int show_continent_map_boundaries;

extern float mapmark_zoom; /*!< scaling factor for the mapmark text */

/*!
 * \ingroup loadsave
 * \brief Read the map info file
 *
 *	Reads the mapinfo file which contains the information on where 
 *	the game maps are located on the continent map
 *
 * \callgraph
 */
void read_mapinfo ();


/*!
 * \brief   Store the current OpenGL viewport and modelview and projection matrices
 *
 *      Store the current OpenGL viewport and modelview and projection matrices.
 *
 * \callgraph
 */
void save_scene_matrix ();

/*!
 * \ingroup interfaces
 * \brief   Gets the world \c x and \c y coordinates from window (mouse click) coordinates.
 *
 *	Gets the world \c x and \c y coordinates from window (mouse click) coordinates. This function will be used when a player is walking.
 *
 * \param scene_x Pointer to the \c x coordinate to be retrieved.
 * \param scene_y Pointer to the \c y coordinate to be retrieved.
 *
 * \callgraph
 */
void get_world_x_y (short *scene_x, short *scene_y);

/*!
 * \ingroup interfaces
 * \brief   Gets the world \c x and \c y coordinates from window (mouse click) coordinates.
 *
 *	Gets the world \c x and \c y coordinates from window (mouse click) coordinates. In contrast to \ref get_world_x_y this functions gets only called when walking is forced, i.e. the CTRL key is pressed when clicking, or when the "Mouse Bug" option in the configuration is set.
 *
 * \callgraph
 *
 * \pre If there is no active actor, this function returns immediately.
 */
void get_old_world_x_y (short *scene_x, short *scene_y);

//void check_menus_out_of_screen();

/*!
 * \ingroup interfaces
 * \brief   Puts the client into 2D mode.
 *
 *      Puts the client into 2D mode. Stores the current attributes for lighting and depth tests and then disables them, then stores the projection matrix and performs an orthographic projection and finally stores the modelview matrix.
 *
 */
void Enter2DMode();
void Enter2DModeExtended(int width, int height);

/*!
 * \ingroup interfaces
 * \brief   Puts the client back into 3D mode.
 *
 *      Puts the client back into 3D mode. Restores the modelview and projection matrices as well as the attributes, saved with \ref Enter2DMode and resets the viewport.
 *
 */
void Leave2DMode();

/*!
 * \ingroup other
 * \brief   Checks the available video modes and initializes the \ref video_modes array.
 *
 *      Checks the available video modes and initializes the \ref video_modes array accordingly.
 *
 * \sa init_video
 */
void build_video_mode_array();

/*!
 * \ingroup interfaces
 * \brief   Sets the texture given in \a which_texture and draws it.
 *
 *      Sets the texture given in \a which_texture by calling \ref get_and_set_texture_id and then draws a full window picture of the texture.
 *
 * \param which_texture the texture to draw
 *
 * \callgraph
 */
void draw_console_pic(int which_texture);

/*!
 * \ingroup interfaces
 * \brief   Adds the char \a ch to the \ref username_str.
 *
 *      Adds the char \a ch to the \ref username_str. If \a ch is either of delete or backspace key, the last char in \ref username_str will get deleted.
 *
 * \param ch    the char to add to \ref username_str
 */
void add_char_to_username(unsigned char ch);

/*!
 * \ingroup interface
 * \brief   Adds the char \a ch to the \ref password_str.
 *
 *      Adds the char \a ch to the \ref password_str. If \a ch is either of delete or backspace key, the last char in \ref password_str will get deleted.
 *
 * \param ch    the char to add to \ref password_str
 */
void add_char_to_password(unsigned char ch);

/*!
 * \ingroup display_2d
 * \brief   Draws a rectangular area with the given texture and scene coordinates.
 *
 *      Draws a rectangular area with the given texture and scene coordinates.
 *
 * \param u_start   u coordinate of the textures start
 * \param v_start   v coordinate of the textures start
 * \param u_end     u coordinate of the textures end
 * \param v_end     v coordinate of the textures end
 * \param x_start   x coordinate of the scene start
 * \param y_start   y coordinate of the scene start
 * \param x_end     x coordinate of the scene end
 * \param y_end     y coordinate of the scene end
 */
void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start, 
int y_start,int x_end,int y_end);

/*!
 * \ingroup display_2d
 * \brief   Draws a rectangular area with the given texture and scene coordinates in reverse mode.
 *
 *      Drasw a rectangular area with the given texture and scene coordinates in reverse mode. This function is only used to draw the hud frame.
 *
 * \param u_start   u coordinate of the textures start
 * \param v_start   v coordinate of the textures start
 * \param u_end     u coordinate of the textures end
 * \param v_end     v coordinate of the textures end
 * \param x_start   x coordinate of the scene start
 * \param y_start   y coordinate of the scene start
 * \param x_end     x coordinate of the scene end
 * \param y_end     y coordinate of the scene end
 *
 */
void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
int y_start,int x_end,int y_end);

/*!
 * \ingroup interfaces
 * \brief   Draws the in-game interface.
 *
 *      Draws the in-game interface by calling \ref draw_hud_frame and \ref display_spells_we_have. If a bag is currently being closed, this function will send an \ref S_CLOSE_BAG command to the server before actually drawing anything.
 *
 * \callgraph
 */
void draw_ingame_interface();

/*!
 * \ingroup interface_map
 * \brief   Switches to the map interface.
 *
 *      Switches to the map interface and changes the cursor to \ref CURSOR_ARROW. The map will be stored in \ref map_text.
 *
 * \retval int  0, if there's no map available for the current place, else 1.
 * \callgraph
 */
int switch_to_game_map();

/*!
 * \ingroup interface_map
 * \brief   Switches back from map interface to game interface.
 *
 *      Switches back from map interface to game interface, by deleting the texture in \ref map_text.
 *
 */
void switch_from_game_map();

/*!
 * \ingroup interface_map
 * \brief   Draws the map interface.
 *
 *      Draws the map interface. Also responsible for drawing the user defined markings on a map as well as the current position of the actor.
 *
 * \param map           if true, the actual map will be drawn in the big window else the continent map will be drawn in the big window
 * \param mouse_mini    if true, the mouse will be drawn smaller than usual
 * \sa draw_scene
 * \callgraph
 */
void draw_game_map (int map, int mouse_mini);

/*!
 * \ingroup interfaces
 * \brief   Saves the user defined markings on maps.
 *
 *      Saves the user defined markings on maps. The markings are stored on a per map basis, i.e. each map gets its own save file, based on the maps .elm filename.
 *
 */
void save_markings();

/*!
 * \ingroup interfaces
 * \brief   Deletes the mark at the current mouse position and saves the changes.
 *
 *      Deletes the mark at the current mouse position and saves the changes by calling \ref save_markings.
 *
 * \callgraph
 *
 * \pre If the mouse is outside the map area, this function will return without performing any actions.
 */
void delete_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief   Adds a mark at the given position.
 *
 *      Adds a mark at the given position.
 *
 * \param map_x x coordinate of tile to be marked
 * \param map_y y coordinate of tile to be marked
 * \param name  name for the mark to set
 * \retval int returns 1 if the location is valid and there is a free slot, and 0 otherwise.
 *
 * \callgraph
 *
 * \pre If the position is outside the map area, this function will return without performing any actions.
 */
int put_mark_on_position(int map_x, int map_y, char * name);

/*!
 * \ingroup interfaces
 * \brief   Adds a mark at the current mouse position.
 *
 *      Adds a mark at the current mouse position.
 *
 * \pre If the mouse is outside the map area, this function will return without performing any actions.
 */
void put_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief   Adds a mark at the actors current position.
 *
 *      Adds a mark with the text \a name at the actors current position.
 *
 * \param name  the text for the mark.
 * \retval int returns 1 if there is a free slot, and 0 otherwise.
 *
 * \callgraph
 *
 * \pre If we don't have an active actor, this function won't perform any actions further actions.
 */
int put_mark_on_current_position(char *name);

/*!
 * \ingroup interfaces
 * \brief   Hides all of the root windows if necessary.
 *
 *      Hides all the root windows, if necessary, i.e. if the associated *_root_win variables are greater than or equal to zero, by calling \ref hide_window for each of them.
 *
 * \callgraph
 */
void hide_all_root_windows ();

/*!
 * \ingroup interfaces
 * \brief   Resizes all of the root windows to the new width \a w and height \a h, if necessary.
 *
 *      Resizes all of the root windows to the new width \a w and height \a h, if necessary, i.e if the associated *_root_win variables are greater than or equal to zero, by calling \ref resize_window for each of them.
 *
 * \param w     the new width of the root windows
 * \param h     the new height of the root windows
 *
 * \callgraph
 */
void resize_all_root_windows (Uint32 w, Uint32 h);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
