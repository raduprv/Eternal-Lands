/*!
 * \file
 * \ingroup interfaces
 * \brief handling of the different interfaces in EL.
 */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

extern int have_a_map;
extern int auto_camera; /*!< if this is true, we are using the auto camera */

/*!
 * \name Action types
 */
/*! @{ */
#define action_walk 0
#define action_look 1
#define action_use 2
#define action_use_witem 3
#define action_trade 4
#define action_attack 5
/*! @} */

extern int action_mode; /*!< contains the current action type */

/*! \name mouse movement coordinates 
 * @{ */
extern int mouse_x; /*!< current x coordinate of the mouse */
extern int mouse_y; /*!< current y coordinate of the mouse */
extern int mouse_delta_x; /*!< current difference between the last saved mouse_x and the current mouse positions x coordinate */
extern int mouse_delta_y; /*!< current difference between the last saved mouse_y and the current mouse positions y coordinate */

/*! \name mouse buttons 
 * @{ */
extern int right_click; /*!< indicates the right mouse button was clicked */
extern int middle_click; /*!< indicates the middle mouse button was clicked */
extern int left_click; /*!< indicates the left mouse button was clicked */
/*! @} */

extern int view_health_bar; /*!< indicates whether we should display the health bar or not */
extern int view_names; /*!< indicates whether the names of actors should be displayed or not */
extern int view_hp; /*!< indicates whether the current/max material points of an actor should be displayed or not */
extern int view_chat_text_as_overtext; /*!< if this is true, then any text an actor is saying will be additionally displayed in a bubble over its head */

extern int login_screen_menus;

/*!
 * \name Interface types
 */
/*! @{ */
#define interface_game 0
#define interface_log_in 1
#define interface_new_char 2
#define interface_console 3
#define interface_opening 4
#define interface_map 5
#define interface_cont 6
#define interface_rules 7
/*! @} */

extern char interface_mode; /*!< current interface mode we are in */
extern char username_box_selected; /*!< true, if the cursor is currently in the username input field */
extern char password_box_selected; /*!< true, if the cursor is currently in the password input field */

extern char username_str[16]; /*!< the username of the actor */
extern char password_str[16]; /*!< the password of the actor */
extern char display_password_str[16]; /*!< a string that will be displayed when entering a password */
extern int username_text_lenght; /*!< actual length of the \see username_str */
extern int password_text_lenght; /*!< actual length of the \see password_str */

extern int font_text;
extern int cons_text;
extern int icons_text;
extern int hud_text;
extern int open_text;
extern int login_text;

extern int selected_3d_object; /*!< if a 3d object is selected by the actor, this variable will contain the id of this object */
extern int selected_inventory_object; /*!< if an inventory object is selected by the actor, this variable will contain the id of this object */

/*!
 * a flag for a mode, that show whether a mode is supported and/or selected.
 */
typedef struct
{
	int supported; /*!< 0 if this mode is supported, else != 0 */
	int selected; /*!< 0 if this mode is selected, else != 0 */
}mode_flag;

extern mode_flag video_modes[10]; /*!< global array of available video modes */

extern Uint32 click_time;
extern int click_speed;

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void get_world_x_y();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void get_old_world_x_y();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \retval int
 * \callgraph
 */
int check_drag_menus();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \retval int
 * \callgraph
 */
int check_scroll_bars();

//void check_menus_out_of_screen();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void check_mouse_click();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 */
void Enter2DMode();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 */
void Leave2DMode();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \sa init_video
 * \sa toggle_full_screen
 */
void build_video_mode_array();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param which_texture
 *
 * \callgraph
 */
void draw_console_pic(int which_texture);

/*!
 * \ingroup interface_opening
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void init_opening_interface();

/*!
 * \ingroup interface_login
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void draw_login_screen();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \sa HandleEvent
 */
void add_char_to_username(unsigned char ch);

/*!
 * \ingroup interface
 * \brief
 *
 *      Detail
 *
 * \param ch
 *
 * \sa HandleEvent
 */
void add_char_to_password(unsigned char ch);

/*!
 * \ingroup display_2d
 * \brief
 *
 *      Detail
 *
 * \param u_start
 * \param v_start
 * \param u_end
 * \param v_end
 * \param x_start
 * \param y_start
 * \param x_end
 * \param y_end
 */
void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start, 
int y_start,int x_end,int y_end);

/*!
 * \ingroup display_2d
 * \brief
 *
 *      Detail
 *
 * \param u_start
 * \param v_start
 * \param u_end
 * \param v_end
 * \param x_start
 * \param y_start
 * \param x_end
 * \param y_end
 *
 * \sa draw_hud_frame
 */
void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
int y_start,int x_end,int y_end);

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void draw_ingame_interface();

/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \retval int
 * \callgraph
 */
int switch_to_game_map();

/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 */
void switch_from_game_map();

#ifdef WINDOW_CHAT
/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \param map
 * \param mouse_mini
 * \sa draw_scene
 * \callgraph
 */
void draw_game_map (int map, int mouse_mini);
#else
/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \param map
 * \sa draw_scene
 * \callgraph
 */
void draw_game_map (int map);
#endif

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param x_len
 *
 * \callgraph
 */
void draw_menu_title_bar(int x, int y, int x_len);

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 */
void save_markings();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void delete_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 */
void put_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param name
 *
 * \callgraph
 */
void put_mark_on_current_position(char *name);

/*!
 * stores the start and end coordinates of a map
 */
struct draw_map{
       unsigned short x_start;
       unsigned short y_start;
       unsigned short x_end;
       unsigned short y_end;       
       const char * name;
};

extern int cur_map; /*!< id of the map we are currently on */
extern GLuint cont_text;
extern GLuint legend_text;
extern GLuint map_text;
extern const struct draw_map seridia_maps[]; /*!< global array of maps for the continet seridia */

#endif
