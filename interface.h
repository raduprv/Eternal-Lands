/*!
 * \file
 * \ingroup interfaces
 * \brief handling of the different interfaces in EL.
 */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

extern int have_a_map;
extern int auto_camera;

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

extern int action_mode;

extern int mouse_x;
extern int mouse_y;
extern int mouse_delta_x;
extern int mouse_delta_y;

extern int right_click;
extern int middle_click;
extern int left_click;

extern int view_health_bar;
extern int view_names;
extern int view_hp;
extern int view_chat_text_as_overtext;

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

extern char interface_mode;
extern char username_box_selected;
extern char password_box_selected;

extern char username_str[16];
extern char password_str[16];
extern char display_password_str[16];
extern int username_text_lenght;
extern int password_text_lenght;

extern int font_text;
extern int cons_text;
extern int icons_text;
extern int hud_text;
extern int open_text;
extern int login_text;

extern int selected_3d_object;
extern int selected_inventory_object;

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
 * \return None
 */
void get_world_x_y();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return int
 */
int check_drag_menus();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return int
 */
int check_scroll_bars();

//void check_menus_out_of_screen();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void check_mouse_click();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void Enter2DMode();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void Leave2DMode();

/*!
 * \ingroup other
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void build_video_mode_array();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param which_texture
 * \return None
 */
void draw_console_pic(int which_texture);

/*!
 * \ingroup interface_opening
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void init_opening_interface();

/*!
 * \ingroup interface_login
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void draw_login_screen();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
 */
void add_char_to_username(unsigned char ch);

/*!
 * \ingroup interface
 * \brief
 *
 *      Detail
 *
 * \param ch
 * \return None
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
 * \return None
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
 * \return None
 */
void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
int y_start,int x_end,int y_end);

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void draw_ingame_interface();

/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \return int
 */
int switch_to_game_map();

/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void switch_from_game_map();

/*!
 * \ingroup interface_map
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void draw_game_map();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param x
 * \param y
 * \param x_len
 * \return None
 */
void draw_menu_title_bar(int x, int y, int x_len);

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void save_markings();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void delete_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \return None
 */
void put_mark_on_map_on_mouse_position();

/*!
 * \ingroup interfaces
 * \brief
 *
 *      Detail
 *
 * \param name
 * \return None
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
};

extern int cur_map;
extern GLuint cont_text;
extern GLuint legend_text;
extern GLuint map_text;
extern const struct draw_map seridia_maps[]; /*!< global array of maps for the continet seridia */

#endif
