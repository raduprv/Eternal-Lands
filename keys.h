/*!
 * \file
 * \ingroup hotkey
 * \brief Handling of hotkeys.
 */
#ifndef __KEYS_H__
#define __KEYS_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name Key Modifiers
 */
/*! @{ */
#define SHIFT (1 << 31)   /*!< Shift modifier is pressed */
#define CTRL (1 << 30)    /*!< Ctrl modifier is pressed */
#define ALT (1 << 29)     /*!< Alt modifier is pressed */
/*! @} */

/*!
 * \name Predefined keys
 */
/*! @{ */
extern Uint32 K_QUIT;           /*!< key for closing the game */
extern Uint32 K_QUIT_ALT;       /*!< alternative key for closing the game */
extern Uint32 K_CAMERAUP;       /*!< key for changing the view angle of the camera up */
extern Uint32 K_CAMERADOWN;     /*!< key for changing the view angle of the camera down */
extern Uint32 K_ZOOMOUT;        /*!< key used for zooming out */
extern Uint32 K_ZOOMIN;         /*!< key used for zooming in */
extern Uint32 K_TURNLEFT;       /*!< key used to turn left */
extern Uint32 K_TURNRIGHT;      /*!< key used to turn right */
extern Uint32 K_ADVANCE;        /*!< key used to move one step (tile) forward */
extern Uint32 K_HEALTHBAR;      /*!< key used to toggle display of the healthbar */
extern Uint32 K_VIEWNAMES;      /*!< key used to toggle display of names */
extern Uint32 K_VIEWHP;         /*!< key used to toggle display of hitpoints (material points) */
extern Uint32 K_STATS;          /*!< key to toggle display of \ref stats_window */
extern Uint32 K_QUESTLOG;       /*!< key to toggle display of \ref questlog_window */
extern Uint32 K_SESSION;        /*!< key to toggle display of session tab in stats_window */
extern Uint32 K_WALK;           /*!< key used to switch to walk cursor */
extern Uint32 K_LOOK;           /*!< key used to switch to look cursor */
extern Uint32 K_USE;            /*!< key used to switch to use cursor */
extern Uint32 K_OPTIONS;        /*!< key used to toggle display of \ref options_window */
extern Uint32 K_REPEATSPELL;    /*!< key used to repeat the last spell used */
extern Uint32 K_SIGILS;         /*!< key used to toggle display of \ref spells_window */
extern Uint32 K_MANUFACTURE;    /*!< key used to toggle display of \ref manufacture_window */
extern Uint32 K_ITEMS;          /*!< key used to toggle display of \ref items_window */
extern Uint32 K_MAP;            /*!< key used to go into \ref interface_map mode */
extern Uint32 K_MINIMAP;		/*!< key used to open minimap window */
extern Uint32 K_ROTATELEFT;     /*!< key used to rotate left (counter clockwise) */
extern Uint32 K_ROTATERIGHT;    /*!< key used to rotate right (clockwise) */
extern Uint32 K_FROTATELEFT;    /*!< key used to fine rotate left (counter clockwise) */
extern Uint32 K_FROTATERIGHT;   /*!< key used to fine rotate right (clockwise) */
extern Uint32 K_BROWSER;        /*!< key used to open the browser using the last displayed URL */
extern Uint32 K_BROWSERWIN;     /*!< key used to open the URL list window */
extern Uint32 K_ESCAPE;         /*!< key used to try fleeing in combat */
extern Uint32 K_CONSOLE;        /*!< key used to toggle \ref interface_console mode */
extern Uint32 K_SHADOWS;        /*!< key used to toggle rendering of shadows */
extern Uint32 K_KNOWLEDGE;      /*!< key used to toggle display of \ref knowledge_window */
extern Uint32 K_ENCYCLOPEDIA;   /*!< key used to toggle display of \ref encyclopedia_window */
extern Uint32 K_HELP;           /*!< key used to toggle display of \ref help_window */
extern Uint32 K_RULES;          /*!< key used to toggle display of \ref rules_window */
extern Uint32 K_NOTEPAD;        /*!< key used to toggle display of \ref notepad_window */
extern Uint32 K_HIDEWINS;       /*!< key used to hide all open windows */
extern Uint32 K_SCREENSHOT;		/*!< key used to make a screenshot */
extern Uint32 K_VIEWTEXTASOVERTEXT; /*!< key used to toggle display of text bubbles (overtext) */
extern Uint32 K_AFK;                /*!< key used to display AFK messages */
extern Uint32 K_SIT;                /*!< key used to toggle sitting status, i.e. sit down/stand up */
extern Uint32 K_RANGINGLOCK;        /*!< key used to toggle ranging-lock status */
extern Uint32 K_BUDDY;              /*!< key used to toggle the buddy window */
extern Uint32 K_NEXT_CHAT_TAB;      /*!< key used to switch to next chat tab */
extern Uint32 K_PREV_CHAT_TAB;      /*!< key used to switch to previous tab */
extern Uint32 K_TABCOMPLETE;        /*!< key used to autocomplete commands/buddy names */
extern Uint32 K_WINDOWS_ON_TOP;     /*!< key used to toggle the windows_on_top option */
extern Uint32 K_MARKFILTER;         /*!< key used to toggle the TAB map mark filtering function */
extern Uint32 K_OPAQUEWIN;          /*!< key used to toggle window opacity */
extern Uint32 K_GRAB_MOUSE;         /*!< key used to toggle whether mouse is in HUD or camera control mode */
extern Uint32 K_FIRST_PERSON;       /*!< key used to toggle first person/third person view mode*/
extern Uint32 K_EXTEND_CAM;			/*!< key used to toggle extended camera mode*/
extern Uint32 K_CUT;                /*!< key used to cut text and copy it into a clipboard */ 
extern Uint32 K_COPY;               /*!< key used to copy text into a clipboard */ 
extern Uint32 K_PASTE;              /*!< key used to copy text from a clipboard into EL */
extern Uint32 K_COPY_ALT;           /*!< alternative key used to copy text into a clipboard */ 
extern Uint32 K_PASTE_ALT;          /*!< alternative key used to copy text from a clipboard into EL */
#ifdef ECDEBUGWIN
extern Uint32 K_ECDEBUGWIN;         /*!< open Eye Candy debug window */
#endif /* ECDEBUGWIN */
extern Uint32 K_EMOTES;             /*!< key used to toggle display of \ref emotes_window */
extern Uint32 K_RANGINGWIN;         /*!< key used to toggle display of ranging win */
/*! @} */

/*! \name Quickbar access keys */
/*! @{ */
extern Uint32 K_ITEM1;
extern Uint32 K_ITEM2;
extern Uint32 K_ITEM3;
extern Uint32 K_ITEM4;
extern Uint32 K_ITEM5;
extern Uint32 K_ITEM6;
extern Uint32 K_ITEM7;
extern Uint32 K_ITEM8;
extern Uint32 K_ITEM9;
extern Uint32 K_ITEM10;
extern Uint32 K_ITEM11;
extern Uint32 K_ITEM12;
/*! @} */

/*! \name Magic quickbar access keys */
/*! @{ */
extern Uint32 K_SPELL1;
extern Uint32 K_SPELL2;
extern Uint32 K_SPELL3;
extern Uint32 K_SPELL4;
extern Uint32 K_SPELL5;
extern Uint32 K_SPELL6;
extern Uint32 K_SPELL7;
extern Uint32 K_SPELL8;
extern Uint32 K_SPELL9;
extern Uint32 K_SPELL10;
extern Uint32 K_SPELL11;
extern Uint32 K_SPELL12;
/*! @} */

/*!
 * \ingroup loadsave
 * \brief   Reads the key configuration from the default key.ini file.
 *
 *      Reads the shortcut key configuration from the default key.ini file.
 *
 * \callgraph
 */
void read_key_config();


/*!
 * \brief   Returns a string describing the specified keydef.
 *
 *      Returns (in the buffer provided) a string describing the specified keydef.
 *
 * \callgraph
 */
const char *get_key_string(Uint32 keydef, char *buf, size_t buflen);

/*!
 * \brief   Returns the value of the specified keydef.
 * 
 *      Returns the key value or 0 if not found.
 *
 * \callgraph
 */
Uint32 get_key_value(const char* name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif	//__KEYS_H__
