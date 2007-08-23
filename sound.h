/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 * \sa soundpage
 */
#ifndef __SOUND_H__
#define __SOUND_H__

#include "platform.h"

#ifdef OGG_VORBIS
	#include <ogg/ogg.h>
	#include <vorbis/codec.h>
	#include <vorbis/vorbisenc.h>
	#include <vorbis/vorbisfile.h>
#endif	//OGG_VORBIS

#ifdef __cplusplus
extern "C" {
#endif

#define SOUNDS_NONE 0
#define SOUNDS_ENVIRO 1
#define SOUNDS_ACTOR 2
#define SOUNDS_WALKING 3

extern int have_sound; /*!< flag indicating whether sound is available */
extern int have_music; /*!< flag indicating whether music is available */
extern int sound_opts; /*!< flag indicating what sounds are enabled */
extern int sound_on; /*!< flag indicating whether sound is enabled */
extern int music_on; /*!< flag indicating whether music is enabled */
#ifndef NEW_SOUND
extern int playing_music; /*!< flag indicating if music is currently playing */
#endif // !NEW_SOUND

extern ALfloat sound_gain; /*!< gain for sound effects */
extern ALfloat music_gain; /*!< gain for playing music */

#ifdef NEW_SOUND
	#define MAX_SOUND_NAME_LENGTH 40
	typedef unsigned long int SOUND_COOKIE;
	#define SOUND_CONFIG_PATH "sound/sound_config.xml"

	#ifdef DEBUG
		void print_sound_types();
		void print_sound_samples();
		void print_sound_sources();
	#endif
#else
	#ifdef DEBUG
		void print_sound_objects ();
	#endif
#endif	//NEW_SOUND
						   
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

#ifdef NEW_SOUND
/*!
 * \ingroup sound_effects
 * \brief Changes sound types played
 *
 *      Changes the types of sound effects being played (environmental only, env + character, etc).
 *
 */
void change_sounds(int * var, int value);
#else // NEW_SOUND
/*!
 * \ingroup sound_effects
 * \brief Toggles the sound
 *
 *      Toggles the status of the sound option in the options dialog and starts or stops the sound.
 *
 */
void toggle_sounds(int *var);
#endif // NEW_SOUND

#ifdef NEW_SOUND
void setup_map_sounds (int map_num);
#endif // NEW_SOUND

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
#ifdef NEW_SOUND
unsigned int add_sound_object(int sound_type,int x, int y, int me);
#else
int add_sound_object(int sound_file,int x, int y,int positional,int loops);
#endif	//NEW_SOUND

#ifdef NEW_SOUND
/*!
 * \ingroup sound_effects
 * \brief Maps a server sound type to a local sound type.
 *
 *      Maps a sound type defined in client_serv.h to a sound type from our sound def.
 *
 * \param sound_type    The number of the server sound type
 * \param x             the x coordinate of the position where the sound should be audible.
 * \param y             the y coordinate of the position where the sound should be audible.
 * \callgraph
 */
unsigned int add_server_sound(int type,int x, int y);
#endif // NEW_SOUND

/*!
 * \ingroup sound_effects
 * \brief Informs the sound subsystem that \a ms milliseconds have passed since the previous update.
 *
 *      Informs the sound subsystem that \a ms milliseconds have passed since the previous update.
 *
 * \param ms		    The time, in ms, since the last update
 * \callgraph
 */
#ifdef NEW_SOUND
void update_sound(int ms);
#else
void update_position();
#endif	//NEW_SOUND


/*!
 * \ingroup sound_effects
 * \brief Stops the specified source.
 *
 *      Searches for a source_data object for source \a source and stops it playing.
 *
 * \param cookie	   The cookie for the sound source to stop
 * \callgraph
 */
#ifdef NEW_SOUND
void stop_sound(unsigned long int cookie);
#else
void stop_sound(int i);

/*!
 * \ingroup sound_effects
 * \brief Deletes the specified source.
 *
 *      Searches for a source_data object for source \a source and deletes it.
 *
 * \param cookie	   The cookie for the sound source to stop
 * \callgraph
 */
void remove_sound_object(int sound);
#endif	//NEW_SOUND

#ifdef NEW_SOUND
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
#endif // NEW_SOUND

/*!
 * \ingroup sound_effects
 * \brief Stop all sound & music playback
 *
 *      Stop all sound & music playback; useful when we change maps, etc.
 *
 * \callgraph
 */
#ifdef NEW_SOUND
void stop_all_sounds();
#else // NEW_SOUND
void kill_local_sounds();
#endif	//NEW_SOUND

#ifdef NEW_SOUND
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
 * \brief Gets the index of the sound for the string
 *
 *      Searches for a sound which matches the item image_id's passed in the input string. A return of -1 indicates no match.
 *
 * \param name		    The string to parse for item image_id's
 * \callgraph
 */
int get_index_for_inv_item_sound_name(const char *name);

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
#endif	//NEW_SOUND

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
#ifdef NEW_SOUND
void sound_source_set_gain(unsigned long int cookie, float gain);
#else
void sound_source_set_gain(int sound, float gain);
#endif	//NEW_SOUND


#ifdef NEW_SOUND
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
#endif	//NEW_SOUND




/////// MUSIC FUNCTIONALITY ///////////
///////////////////////////////////////

#ifndef NEW_SOUND
ALuint get_loaded_buffer(int i);
#endif	//NEW_SOUND

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
#ifdef NEW_SOUND
int update_streams(void *dummy);
#else // NEW_SOUND
int update_music(void *dummy);
#endif //NEW_SOUND

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

#endif //__SOUND_H__
