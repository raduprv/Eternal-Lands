/*!
 * \file
 * \ingroup hotkey
 * \brief handling of hotkeys.
 */
#ifndef __KEYS_H__
#define __KEYS_H__

/*!
 * \name Key Modifiers
 */
/*! @{ */
#define SHIFT (1<<31)
#define CTRL (1<<30)
#define ALT (1<<29)
/*! @} */

/*!
 * \name Predefined keys
 */
/*! @{ */
extern Uint32 K_CAMERAUP;
extern Uint32 K_CAMERADOWN;
extern Uint32 K_ZOOMOUT;
extern Uint32 K_ZOOMIN;
extern Uint32 K_TURNLEFT;
extern Uint32 K_TURNRIGHT;
extern Uint32 K_ADVANCE;
extern Uint32 K_HEALTHBAR;
extern Uint32 K_VIEWNAMES;
extern Uint32 K_VIEWHP;
extern Uint32 K_STATS;
extern Uint32 K_WALK;
extern Uint32 K_LOOK;
extern Uint32 K_USE;
extern Uint32 K_OPTIONS;
extern Uint32 K_REPEATSPELL;
extern Uint32 K_SIGILS;
extern Uint32 K_MANUFACTURE;
extern Uint32 K_ITEMS;
extern Uint32 K_MAP;
extern Uint32 K_ROTATELEFT;
extern Uint32 K_ROTATERIGHT;
extern Uint32 K_FROTATELEFT;
extern Uint32 K_FROTATERIGHT;
extern Uint32 K_BROWSER;
extern Uint32 K_ESCAPE;
extern Uint32 K_CONSOLE;
extern Uint32 K_SHADOWS;
extern Uint32 K_KNOWLEDGE;
extern Uint32 K_ENCYCLOPEDIA;
extern Uint32 K_HELP;
extern Uint32 K_HIDEWINS;
extern Uint32 K_ITEM1;
extern Uint32 K_ITEM2;
extern Uint32 K_ITEM3;
extern Uint32 K_ITEM4;
extern Uint32 K_ITEM5;
extern Uint32 K_ITEM6;
extern Uint32 K_VIEWTEXTASOVERTEXT;
extern Uint32 K_AFK;
extern Uint32 K_SIT;
/*! @} */

/*!
 * \ingroup loadsave
 * \brief
 *
 *      Detail
 *
 * \callgraph
 */
void read_key_config();

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup misc_utils
// * \brief
// *
// *      Detail
// *
// * \param data
// * \param len
// * \retval unsigned int
// *
// * \sa get_key_code
// */
//unsigned int CRC32(unsigned char *data, int len);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup hotkey
// * \brief
// *
// *      Detail
// *
// * \param key
// * \retval unsigned short
// *
// * \sa CRC32
// */
//unsigned short get_key_code(char *key);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup hotkey
// * \brief
// *
// *      Detail
// *
// * \param s
// * \retval unsigned int
// * \callgraph
// */
//unsigned int parse_key_string(char *s);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup hotkey
// * \brief
// *
// *      Detail
// *
// * \param key
// * \param n
// *
// * \sa parse_key_string
// */
//void add_key(unsigned int *key,unsigned int n);

#endif	//__KEYS_H__
