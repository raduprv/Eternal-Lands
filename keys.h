/*!
 * \file
 * \ingroup hotkey
 * \brief Handling of hotkeys.
 */
#ifndef __KEYS_H__
#define __KEYS_H__

#include <stddef.h>
#include <SDL_types.h>
#include <SDL_keycode.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Function key definitiion
 */
/*! @{ */
#define KEY_DEF_NAME_SIZE 25
typedef struct
{
	SDL_Keycode key_code;			/*!< the SDL virtual key code; see SDL_Keycode */
	Uint16 key_mod;					/*!< the key modifiers; see SDL_Keymod */
	char name[KEY_DEF_NAME_SIZE];	/*!< the \#K_xxx name for the file */
} el_key_def;
/*! @} */

/*!
 * \name Macro to compare a #K_xx definition with supplied SDL_Keymod and SDL_Keymod values, true if they match.
 */
/*! @{ */
#define KEY_DEF_CMP(key_def, key_code, key_mod) ((key_def.key_code == key_code) && \
	(key_def.key_mod == (key_mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI | KMOD_MODE))))
/*! @} */

/*!
 * \name Predefined keys
 */
/*! @{ */
extern el_key_def K_QUIT;               /*!< key for closing the game */
extern el_key_def K_QUIT_ALT;           /*!< alternative key for closing the game */
extern el_key_def K_CAMERAUP;           /*!< key for changing the view angle of the camera up */
extern el_key_def K_CAMERADOWN;         /*!< key for changing the view angle of the camera down */
extern el_key_def K_ZOOMOUT;            /*!< key used for zooming out */
extern el_key_def K_ZOOMIN;             /*!< key used for zooming in */
extern el_key_def K_TURNLEFT;           /*!< key used to turn left */
extern el_key_def K_TURNRIGHT;          /*!< key used to turn right */
extern el_key_def K_ADVANCE;            /*!< key used to move one step (tile) forward */
extern el_key_def K_HEALTHBAR;          /*!< key used to toggle display of the healthbar */
extern el_key_def K_VIEWNAMES;          /*!< key used to toggle display of names */
extern el_key_def K_VIEWHP;             /*!< key used to toggle display of hitpoints (material points) */
extern el_key_def K_VIEWETHER;          /*!< key used to toggle display of ether pointer */
extern el_key_def K_ETHERBARS;          /*!< key used to toggle display of ether bar */
extern el_key_def K_STATS;              /*!< key to toggle display of \ref stats_window */
extern el_key_def K_QUESTLOG;           /*!< key to toggle display of \ref questlog_win */
extern el_key_def K_SESSION;            /*!< key to toggle display of session tab in stats_window */
extern el_key_def K_WALK;               /*!< key used to switch to walk cursor */
extern el_key_def K_LOOK;               /*!< key used to switch to look cursor */
extern el_key_def K_USE;                /*!< key used to switch to use cursor */
extern el_key_def K_OPTIONS;            /*!< key used to toggle display of \ref options_window */
extern el_key_def K_REPEATSPELL;        /*!< key used to repeat the last spell used */
extern el_key_def K_SIGILS;             /*!< key used to toggle display of \ref spells_window */
extern el_key_def K_MANUFACTURE;        /*!< key used to toggle display of \ref manufacture_window */
extern el_key_def K_ITEMS;              /*!< key used to toggle display of \ref items_window */
extern el_key_def K_MAP;                /*!< key used to go into \ref interface_map mode */
extern el_key_def K_MINIMAP;            /*!< key used to open minimap window */
extern el_key_def K_ROTATELEFT;         /*!< key used to rotate left (counter clockwise) */
extern el_key_def K_ROTATERIGHT;        /*!< key used to rotate right (clockwise) */
extern el_key_def K_FROTATELEFT;        /*!< key used to fine rotate left (counter clockwise) */
extern el_key_def K_FROTATERIGHT;       /*!< key used to fine rotate right (clockwise) */
extern el_key_def K_BROWSER;            /*!< key used to open the browser using the last displayed URL */
extern el_key_def K_BROWSERWIN;         /*!< key used to open the URL list window */
extern el_key_def K_ESCAPE;             /*!< key used to try fleeing in combat */
extern el_key_def K_CONSOLE;            /*!< key used to toggle \ref interface_console mode */
extern el_key_def K_SHADOWS;            /*!< key used to toggle rendering of shadows */
extern el_key_def K_KNOWLEDGE;          /*!< key used to toggle display of \ref knowledge_window */
extern el_key_def K_ENCYCLOPEDIA;       /*!< key used to toggle display of \ref encyclopedia_window */
extern el_key_def K_HELP;               /*!< key used to toggle display of \ref help_window */
extern el_key_def K_NOTEPAD;            /*!< key used to toggle display of \ref notepad_window */
extern el_key_def K_HIDEWINS;           /*!< key used to hide all open windows */
extern el_key_def K_SCREENSHOT;         /*!< key used to make a screenshot */
extern el_key_def K_VIEWTEXTASOVERTEXT; /*!< key used to toggle display of text bubbles (overtext) */
extern el_key_def K_AFK;                /*!< key used to display AFK messages */
extern el_key_def K_SIT;                /*!< key used to toggle sitting status, i.e. sit down/stand up */
extern el_key_def K_RANGINGLOCK;        /*!< key used to toggle ranging-lock status */
extern el_key_def K_BUDDY;              /*!< key used to toggle the buddy window */
extern el_key_def K_NEXT_CHAT_TAB;      /*!< key used to switch to next chat tab */
extern el_key_def K_PREV_CHAT_TAB;      /*!< key used to switch to previous tab */
extern el_key_def K_RULES;              /*!< key used to toggle display of \ref rules_window */
extern el_key_def K_TABCOMPLETE;        /*!< key used to autocomplete commands/buddy names */
extern el_key_def K_WINDOWS_ON_TOP;     /*!< key used to toggle the windows_on_top option */
extern el_key_def K_MARKFILTER;         /*!< key used to toggle the TAB map mark filtering function */
extern el_key_def K_OPAQUEWIN;          /*!< key used to toggle window opacity */
extern el_key_def K_GRAB_MOUSE;         /*!< key used to toggle whether mouse is in HUD or camera control mode */
extern el_key_def K_FIRST_PERSON;       /*!< key used to toggle first person/third person view mode*/
extern el_key_def K_EXTEND_CAM;         /*!< key used to toggle extended camera mode*/
extern el_key_def K_CUT;                /*!< key used to cut text and copy it into a clipboard */
extern el_key_def K_COPY;               /*!< key used to copy text into a clipboard */
extern el_key_def K_PASTE;              /*!< key used to copy text from a clipboard into EL */
extern el_key_def K_COPY_ALT;           /*!< alternative key used to copy text into a clipboard */
extern el_key_def K_PASTE_ALT;          /*!< alternative key used to copy text from a clipboard into EL */
#ifdef ECDEBUGWIN
extern el_key_def K_ECDEBUGWIN;         /*!< open Eye Candy debug window */
#endif /* ECDEBUGWIN */
extern el_key_def K_EMOTES;             /*!< key used to toggle display of \ref emotes_win */
extern el_key_def K_RANGINGWIN;         /*!< key used to toggle display of ranging win */
extern el_key_def K_TARGET_CLOSE;       /*!< toggle target_close_clicked_creature option */
extern el_key_def K_COUNTERS;           /*!< key used to toggle display of counters window  */
extern el_key_def K_HELPSKILLS;         /*!< key used to toggle display of help window skills tab  */
extern el_key_def K_WINSCALEUP;         /*!< key used to increase custom window scale factor */
extern el_key_def K_WINSCALEDOWN;       /*!< key used to decrease custom window scale factor */
extern el_key_def K_WINSCALEDEF;        /*!< key used to reset custom window scale factor to default */
extern el_key_def K_WINSCALEINIT;       /*!< key used to reset custom window scale factor to initial value */
extern el_key_def K_SUMMONINGMENU;      /*!< key used to open summoning menu for last summomed creature */
extern el_key_def K_CHAT;               /*!< key used to toggle chat display */
/*! @} */

/*! \name Quickbar access keys */
/*! @{ */
extern el_key_def K_ITEM1;
extern el_key_def K_ITEM2;
extern el_key_def K_ITEM3;
extern el_key_def K_ITEM4;
extern el_key_def K_ITEM5;
extern el_key_def K_ITEM6;
extern el_key_def K_ITEM7;
extern el_key_def K_ITEM8;
extern el_key_def K_ITEM9;
extern el_key_def K_ITEM10;
extern el_key_def K_ITEM11;
extern el_key_def K_ITEM12;
/*! @} */

/*! \name Magic quickbar access keys */
/*! @{ */
extern el_key_def K_SPELL1;
extern el_key_def K_SPELL2;
extern el_key_def K_SPELL3;
extern el_key_def K_SPELL4;
extern el_key_def K_SPELL5;
extern el_key_def K_SPELL6;
extern el_key_def K_SPELL7;
extern el_key_def K_SPELL8;
extern el_key_def K_SPELL9;
extern el_key_def K_SPELL10;
extern el_key_def K_SPELL11;
extern el_key_def K_SPELL12;
/*! @} */

/*!
 * \ingroup loadsave
 * \brief   Reads the key configuration from the default key.ini file.
 *
 *      Reads the shortcut key configuration from the default key.ini file.
 *
 * \callgraph
 */
void read_key_config(void);


/*!
 * \brief   Returns a string describing the specified keydef.
 *
 *      Returns (in the buffer provided) a string describing the specified keydef.
 *
 * \callgraph
 */
const char *get_key_string(el_key_def keydef, char *buf, size_t buflen);

/*!
 * \brief   Returns the value of the specified keydef.
 *
 *      Returns the key value or 0 if not found.
 *
 * \callgraph
 */
el_key_def get_key_value(const char* name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__KEYS_H__
