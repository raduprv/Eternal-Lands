/*!
 * \file
 * \ingroup options_win
 * \brief handles the display of the options window.
 */
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

/*!
 * option_struct is used to store information related to one single option.
 */
typedef struct
{
	char * name; /*!< the name of the option */
	char * desc; /*!< description of the option */
	int type; /*!< type of the option */
	int column;
	void (*func)(int*,int*); /*!< pointer to a callback function executed when the option is clicked */
	int * data_1; /*!< ***reserved??? */
	int * data_2; /*!< ***reserved??? */
} option_struct;

/*!
 * a list of \see option_struct options.
 */
struct options_struct
{
	int no; /*!< current number of options in \a option */
	option_struct * option[25]; /*!< fixed size array of \see option_struct options. The actual number of used options is stored in \a no. */
};

/*!
 * \name windows handlers
 */
/*! @{ */
extern int options_win; /*!< options windows handler */
/*! @} */

extern int options_menu_x;
extern int options_menu_y;
extern int options_menu_x_len;
extern int options_menu_y_len;
//extern int options_menu_dragged;

/*!
 * \ingroup options_win
 * \brief displays the options window.
 * 
 *      Displays the options window.
 *
 * \return None
 */
void display_options_menu();

/*!
 * \ingroup options_win
 * \brief initializes the data for the options window.
 *
 *      Initializes the data for the options window.
 *
 * \return None
 */
void init_display_options_menu();

/*!
 * \ingroup options_win
 * \brief adds the given option to the list of known options.
 *
 *      Adds the given option to the list of known options.
 *
 * \param type      type of the option
 * \param name      name of the option
 * \param desc      description for this option
 * \param func      pointer to the callback implementation used for this option
 * \param data_1    ***reserved???
 * \param data_2    ***reserved???
 * \param column
 * \return None
 */
void add_option(int type, char * name, char * desc, void * func, int * data_1, int * data_2, int column);

/*!
 * \ingroup options_win
 * \brief changes the option to the data given in \a data_1 and \a data_2.
 *
 *      Changes the option to the data given in \a data_1 and \a data_2.
 *
 * \param data_1
 * \param data_2
 * \return None
 */
void change_option(int * data_1, int * data_2);

/*!
 * \ingroup options_win
 * \brief switches the client into fullscreen mode.
 *
 *      Switches the client into fullscreen mode.
 *
 * \param unused
 * \param unused2
 * \return None
 */
void move_to_full_screen(int  * unused, int * unused2);

/*!
 * \ingroup options_win
 * \brief switches the video mode to the one given in \a mode.
 *
 *      Switches video mode to the given \a mode.
 *
 * \param unused
 * \param mode      the new mode to use
 * \return None
 */
void switch_video_modes(int * unused, int * mode);

/*!
 * \ingroup options_win
 * \brief toggles sound on/off
 *
 *      Toggles sound on or off.
 *
 * \param unused
 * \param unused2
 * \return None
 * \sa sound_effects
 */
void change_sound(int  * unused, int * unused2);

/*!
 * \ingroup options_win
 * \brief toggles music on/off.
 *
 *      Toggles music on or off.
 *
 * \param unused
 * \param unused2
 * \return None
 * \sa music
 */
void change_music(int  * unused, int * unused2);

/*!
 * \name Option Modes
 */
/*! @{ */
#define NONE 0		//0000b
#define OPTION 1	//0001b
#define VIDEO_MODE 2 	//0010b
/*! @} */

#endif

