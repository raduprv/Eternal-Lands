/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 * \sa soundpage
 */
#ifndef __SOUND_H__
#define __SOUND_H__
#include "global.h"

#ifdef WINDOWS //lib location platform checking
	#include <al.h>
	#include <alc.h>
	#include <alut.h>
#elif defined(OSX)
	#include <OpenAL/alut.h>
	#include <OpenAL/alc.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
	#include <AL/alut.h>
#endif //lib location platform checking

#ifndef	NO_MUSIC
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#endif	//NO_MUSIC

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
extern int playing_music; /*!< flag indicating if music is currently playing */

extern ALfloat sound_gain; /*!< gain for sound effects */
extern ALfloat music_gain; /*!< gain for playing music */

ALCdevice *mSoundDevice;			// These lines may need to be removed again in the patch.
ALCcontext *mSoundContext;			// Please check.

#ifdef NEW_SOUND
	#define MAX_SOUND_NAME_LENGTH 15
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
 * \param sound_config_path  the path of the sound-type configuration XML file.
 *
 * \callgraph
 */
#ifdef NEW_SOUND
	void init_sound(char *sound_config_path);
#else
	void init_sound();
#endif	//NEW_SOUND
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
 * \brief Adds \a sound_type at the position (\a x, \a y) to the list of sounds to play.
 *
 *      Adds \a sound_type at the position (\a x, \a y) to the list of sounds to play. The parameter \a positional determines whether we should use positional sound.
 *
 * \param sound_type    A handle for the sound type to play
 * \param x             the x coordinate of the position where the sound should be audible.
 * \param y             the y coordinate of the position where the sound should be audible.
 * \callgraph
 */
#ifdef NEW_SOUND
	unsigned int add_sound_object(int sound_type,int x, int y);
#else
	int add_sound_object(int sound_file,int x, int y,int positional,int loops);
#endif	//NEW_SOUND


/*!
 * \ingroup sound_effects
 * \brief Gets the index of the named sound type.
 *
 *      Searches for a sound type which matches \a name. The search is not case-sensitive. A return of < 0 indicates no match.
 *
 * \param name		    The sound type to find
 * \callgraph
 */
#ifdef NEW_SOUND
	int get_index_for_sound_type_name(char *name);
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
	void remove_sound_object(int sound);
#endif	//NEW_SOUND


/*!
 * \ingroup sound_effects
 * \brief Stop all sound & music playback
 *
 *      Stop all sound & music playback; usefull when we change maps, etc.
 *
 * \callgraph
 */
void stop_all_sounds();
#ifndef NEW_SOUND
	void kill_local_sounds();
#endif	//NEW_SOUND
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

void toggle_sounds(int *var);





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
int update_music(void *dummy);

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

#ifndef NEW_SOUND
	ALuint get_loaded_buffer(int i);
#endif	//NEW_SOUND

int display_song_name();

void change_sounds(int * var, int value);

void toggle_music(int * var);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__SOUND_H__
