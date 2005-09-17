/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 * \sa soundpage
 */
#ifndef __SOUND_H__
#define __SOUND_H__

extern int have_sound; /*!< flag indicating whether sound is available */
extern int have_music; /*!< flag indicating whether music is available */
extern int sound_on; /*!< flag indicating whether sound is enabled */
extern int music_on; /*!< flag indicating whether music is enabled */
extern int no_sound; /*!< flag inidicating whether the #no_sound ini file directive is used */
extern int playing_music; /*!< flag indicating if music is currently playing */

extern ALfloat sound_gain; /*!< gain for sound effects */
extern ALfloat music_gain; /*!< gain for playing music */

#ifdef DEBUG
void print_sound_objects ();
#endif // def DEBUG

/*!
 * \ingroup sound_effects
 * \brief   Stops playing the specified sound effect.
 *
 *      Stops playing the sound that is specified by the given index.
 *
 * \param i index of the sound to stop
 */
void stop_sound(int i);

/*!
 * \ingroup sound_effects
 * \brief Adds the file \a sound_file at the position (\a x, \a y) to the list of sounds to play.
 *
 *      Adds the file \a sound_file at the position (\a x, \a y) to the list of sounds to play. The parameter \a positional determines whether we should use positional sound. The parameter \a loops is a flag to indicicate whether we should play the sound in a loop or not.
 *
 * \param sound_file    A handle for the sound file to use
 * \param x             the x coordinate of the position where the sound should be audible.
 * \param y             the y coordinate of the position where the sound should be audible.
 * \param positional    boolean flag, indicating whether we shall play the sound positional.
 * \param loops         boolean flag, indicating whether we shall play the sound in a loop.
 * \retval int          int returns the added sound_object
 * \callgraph
 */
int add_sound_object(int sound_file,int x, int y,int positional,int loops);

/*!
 * \ingroup sound_effects
 * \brief Removes a sound from the sound list
 *
 *	Removes a sound from the sound list, and frees up the space associated with it.
 *
 * \param sound	The sound to be removed
 * \callgraph
 */
void remove_sound_object (int sound);

/*!
 * \ingroup sound_effects
 * \brief Sets the gain of a sound object
 *
 *	Sets the gain of a sound object. Only works properly on non-positional sounds.
 *
 * \param sound	The sound to be affected
 * \param gain  The gain, relative to \see sound_gain
 * \callgraph
 */
void sound_object_set_gain(int sound, float gain);

/*!
 * \ingroup sound_effects
 * \brief Sets the gain of a sound object
 *
 *	Sets the gain of a sound object. Only works properly on non-positional sounds.
 *
 * \param sound	The sound to be affected
 * \param gain  The gain, relative to \see sound_gain
 * \callgraph
 */
void sound_object_set_gain(int sound, float gain);

/*!
 * \ingroup sound_effects
 * \brief Updates the position change of sound played.
 *
 *      Updates the position change of sound played.
 *
 */
void update_position();

/*!
 * \ingroup sound_effects
 * \brief Kill all sounds that loop infinitely
 *
 *      kill all the sounds that loop infinitely; usefull when we change maps, etc.
 *
 * \callgraph
 */
void kill_local_sounds();

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
 * \ingroup other
 * \brief Initializes the sound system of EL
 *
 *      Initializes the sound system of EL
 *
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

ALuint get_loaded_buffer(int i);

#endif
