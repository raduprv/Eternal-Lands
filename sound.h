/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 * \sa soundpage
 */
#ifndef __SOUND_H__
#define __SOUND_H__

#ifdef NEW_SOUND

#include "platform.h"
#include "actors.h"

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SOUNDS_NONE 0
#define SOUNDS_ENVIRO 1
#define SOUNDS_MAP 2
#define SOUNDS_ACTOR 3
#define SOUNDS_WALKING 4
#define SOUNDS_CROWD 5
#define SOUNDS_GAMEWIN 6
#define SOUNDS_WARNINGS 7
#define SOUNDS_CLIENT 8

extern int have_sound; /*!< flag indicating whether sound is available */
extern int have_music; /*!< flag indicating whether music is available */
extern int no_sound; /*!< flag indicating whether sounds are initialised and to be processed */
extern int sound_on; /*!< flag indicating whether sound is enabled */
extern int music_on; /*!< flag indicating whether music is enabled */

extern ALfloat sound_gain; /*!< gain for sound effects */
extern ALfloat music_gain; /*!< gain for playing music */
extern ALfloat crowd_gain; /*!< gain for crowd sound effects */
extern ALfloat enviro_gain; /*!< gain for environmental and map sound effects */
extern ALfloat actor_gain; /*!< gain for actor sound effects */
extern ALfloat walking_gain; /*!< gain for walking sound effects */
extern ALfloat gamewin_gain; /*!< gain for game window (items/inv etc) sound effects */
extern ALfloat client_gain; /*!< gain for client sound effects */
extern ALfloat warnings_gain; /*!< gain for user configured text warning sound effects */

extern char sound_device[30];
extern int afk_snd_warning;

#define MAX_SOUND_NAME_LENGTH 40

typedef unsigned long int SOUND_COOKIE;

extern int have_sound_config; /*!< flag indicating whether the sound config was found */
#define SOUND_CONFIG_PATH "sound/sound_config.xml"
#define SOUND_WARNINGS_PATH "sound_warnings.txt"

#ifdef DEBUG
void print_sound_types();
void print_sound_samples();
void print_sounds_list();
void print_sound_sources();
#endif // DEBUG
						   
/*!
 * \ingroup other
 * \brief Initializes the sound system of EL
 *
 *      Initializes the sound system of EL
 *
 * \callgraph
 */
void init_sound();

/*!
 * \ingroup other
 * \brief Closes the sound system of EL
 *
 *      Shuts down the sound system of EL.
 *
*/
void destroy_sound();

/*!
 * \ingroup sound_effects
 * \brief Turns off playback of sound.
 *
 *      Turns off the playback of sound (effects).
 *
 */
void turn_sound_off();

/*!
 * \ingroup sound_effects
 * \brief Turns on playback of sound
 *
 *      Turns on the playback of sound (effects).
 *
 */
void turn_sound_on();

/*!
 * \ingroup sound_effects
 * \brief Toggles the sound
 *
 *      Toggles the status of the sound option in the options dialog and starts or stops the sound.
 *
 */
void toggle_sounds(int *var);

/*!
 * \ingroup sound_effects
 * \brief Enables or disables the sound system
 *
 *      Toggles the status of the no_sound option in the options dialog and starts or stops the sound
 *      and music depending on their settings.
 *
 */
void disable_sound(int *var);

void setup_map_sounds (int map_num);

/*!
 * \ingroup sound_effects
 * \brief Adds \a sound_type at the position (\a x, \a y) to the list of sounds to play.
 *
 *      Adds \a sound_type at the position (\a x, \a y) to the list of sounds to play. The parameter \a positional determines whether we should use positional sound.
 *
 * \param sound_type    A handle for the sound type to play
 * \param x             the x coordinate of the position where the sound should be audible.
 * \param y             the y coordinate of the position where the sound should be audible.
 * \param me			Is this sound from my actor? (bool - 0 or 1)
 * \callgraph
 */
unsigned int add_sound_object(int type, int x, int y, int me);

unsigned int add_walking_sound(int type, int x, int y, int me, float scale);
unsigned int add_map_sound(int type, int x, int y);
unsigned int add_particle_sound(int type, int x, int y);
unsigned int add_spell_sound(int spell);
unsigned int add_death_sound(actor * act);
unsigned int add_battlecry_sound(actor * act);
unsigned int add_sound_object_gain(int type, int x, int y, int me, float initial_gain);
void initial_sound_init(void);
void final_sound_exit(void);

/*!
 * \ingroup sound_effects
 * \brief Maps a server sound type to a local sound type.
 *
 *      Maps a sound type defined in client_serv.h to a sound type from our sound def.
 *
 * \param sound_type    The number of the server sound type
 * \param x             the x coordinate of the position where the sound should be audible.
 * \param y             the y coordinate of the position where the sound should be audible.
 * \param gain			The gain the sound should be played at (used for rain)
 * \callgraph
 */
unsigned int add_server_sound(int type, int x, int y, int gain);

/*!
 * \ingroup sound_effects
 * \brief Informs the sound subsystem that \a ms milliseconds have passed since the previous update.
 *
 *      Informs the sound subsystem that \a ms milliseconds have passed since the previous update.
 *
 * \param ms		    The time, in ms, since the last update
 * \callgraph
 */
void update_sound(int ms);

/*!
 * \ingroup sound_effects
 * \brief Stops the specified source.
 *
 *      Searches for a source_data object for source \a source and stops it playing.
 *
 * \param cookie	   The cookie for the sound source to stop
 * \callgraph
 */
void stop_sound(unsigned long int cookie);

/*!
 * \ingroup sound_effects
 * \brief Deletes a source at the given location
 *
 *      Searches for a source_data object with the location ( \a x \a y ) and deletes it.
 *
 * \param x			The x coordinate of the location
 * \param y			The y coordinate of the location
 * \callgraph
 */
void stop_sound_at_location(int x, int y);

/*!
 * \ingroup sound_effects
 * \brief Stop all sound & music playback
 *
 *      Stop all sound & music playback; useful when we change maps, etc.
 *
 * \callgraph
 */
void stop_all_sounds();

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the named sound type.
 *
 *      Searches for a sound type which matches \a name. The search is not case-sensitive. A return of -1 indicates no match.
 *
 * \param name		    The sound type to find
 * \callgraph
 */
int get_index_for_sound_type_name(const char *name);

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the sound for the named particle sound.
 *
 *      Searches for a particle sound which matches \a name, stripping the directory ./particles/ and extension .part. The search is not case-sensitive. A return of -1 indicates no match.
 *
 * \param name		    The particle sound to find
 * \callgraph
 */
int get_sound_index_for_particle_file_name(const char *name);

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the sound for the given special effect num.
 *
 *      Searches for a special effect sound which matches \a sfx. A return of -1 indicates no match.
 *
 * \param sfx		    The special effect to find the sound for
 * \callgraph
 */
int get_sound_index_for_sfx(int sfx);

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the sound for the image id
 *
 *      Searches for a sound which matches the given \a image_id. A return of -1 indicates no match.
 *
 * \param image_id	    The image_id to search for
 * \callgraph
 */
int get_index_for_inv_use_item_sound(int image_id);

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the sound for the image id's
 *
 *      Searches for a sound which matches the given image_id's. A return of -1 indicates no match.
 *
 * \param use_image_id	    	The first image_id (used with)
 * \param with_image_id	    	The second image_id
 * \callgraph
 */
int get_index_for_inv_usewith_item_sound(int use_image_id, int with_image_id);

/*!
 * \ingroup sound_effects
 * \brief Gets the index of the sound source for the given cookie.
 *
 *      Searches for a sound source which matches the cookie \a cookie. A return of -1 indicates no match.
 *		Note: This result must not be stored, but used immediately!
 *
 * \param name		    The cookie to find the sound source for
 * \callgraph
 */
int find_sound_source_from_cookie(unsigned int cookie);

/*!
 * \ingroup sound_effects
 * \brief Sets the gain of a sound source
 *
 *	Sets the gain of a sound source. Only works properly on non-positional sounds.
 *
 * \param source	The sound to be affected
 * \param gain  The gain, relative to \see sound_gain
 * \callgraph
 */
void sound_source_set_gain(unsigned long int cookie, float gain);

/*!
 * \ingroup other
 * \brief Loads the configuration of the sound system for EL
 *
 *	Loads the configuration of the sound system for EL.
 *
 * \param path		The path of the sounds configuration XML file.
 * \callgraph
 */
void load_sound_config_data (const char *path);

void clear_sound_data();

void handle_walking_sound(actor *pActor, int def_snd);
int check_sound_loops(unsigned int cookie);
void check_sound_alerts(const Uint8* text, size_t len, Uint8 channel);
#ifdef DEBUG_MAP_SOUND
void print_sound_boundaries(int map);
#endif // DEBUG_MAP_SOUND

static __inline__ void do_click_sound(){
	add_sound_object(get_index_for_sound_type_name("Button Click"), 0, 0, 1);
}
static __inline__ void do_drag_item_sound(){
	add_sound_object(get_index_for_sound_type_name("Drag Item"), 0, 0, 1);
}
static __inline__ void do_alert1_sound(){
	add_sound_object(get_index_for_sound_type_name("alert1"), 0, 0, 1);
}
static __inline__ void do_drop_item_sound(){
	add_sound_object(get_index_for_sound_type_name("Drop Item"), 0, 0, 1);
}
static __inline__ void do_get_item_sound(){
	add_sound_object(get_index_for_sound_type_name("Get Item"), 0, 0, 1);
}
static __inline__ void do_window_close_sound(){
	add_sound_object(get_index_for_sound_type_name("Window Close"), 0, 0, 1);
}
static __inline__ void do_icon_click_sound(){
	add_sound_object(get_index_for_sound_type_name("Icon Click"), 0, 0, 1);
}
static __inline__ void do_error_sound(){
	add_sound_object(get_index_for_sound_type_name("Error"), 0, 0, 1);
}
static __inline__ void do_disconnect_sound(){
	add_sound_object(get_index_for_sound_type_name("Disconnected"), 0, 0, 1);
}
static __inline__ void do_connect_sound(){
	add_sound_object(get_index_for_sound_type_name("Connected"), 0, 0, 1);
}
static __inline__ void do_afk_sound(){
	add_sound_object(get_index_for_sound_type_name("AFK Message"), 0, 0, 1);
}

/////// MUSIC FUNCTIONALITY ///////////
///////////////////////////////////////

/*!
 * \ingroup music
 * \brief Retrieves the playlist for the current map.
 *
 *      Retrieves the playlist, associated with the current map.
 *
 */
void get_map_playlist();

/*!
 * \ingroup music
 * \brief Starts playing music according the entries in the given playlist
 *
 *      Start music playback, according the playlist entries in the given playlist.
 *
 * \param list  a handle for the playlist to use for playback.
 *
 * \callgraph
 */
void play_music(int list);

/*!
 * \ingroup music
 * \brief Updates the music and brings the sound system in sync.
 *
 *      Updates the music and brings the sound system in sync.
 *
 * \param dummy currently not used
 * \retval int  always returns 0.
 * \callgraph
 */
int update_streams(void *dummy);

/*!
 * \ingroup music
 * \brief Turns music off and stops playback of music.
 *
 *      Turns music off and stops playback of music.
 */
void turn_music_off();

/*!
 * \ingroup music
 * \brief Turns music on and starts playback of music
 *
 *      Turns music on and starts playback of music.
 */
void turn_music_on();

/*!
 * \ingroup music
 * \brief Displays song title
 *
 *      Prints the details of the currently playing song in the console.
 *
 */
int display_song_name();

/*!
 * \ingroup music
 * \brief Toggles the music
 *
 *      Toggles the status of the music option in the options dialog and starts or stops the music.
 *
 */
void toggle_music(int * var);


#ifdef __cplusplus
} // extern "C"
#endif

#else
static __inline__ void do_click_sound(){}
static __inline__ void do_drag_item_sound(){}
static __inline__ void do_alert1_sound(){}
static __inline__ void do_drop_item_sound(){}
static __inline__ void do_get_item_sound(){}
static __inline__ void do_window_close_sound(){}
static __inline__ void do_icon_click_sound(){}
static __inline__ void do_error_sound(){}
static __inline__ void do_disconnect_sound(){}
static __inline__ void do_connect_sound(){}
static __inline__ void do_afk_sound(){}
#endif // NEW_SOUND
#endif // __SOUND_H__
