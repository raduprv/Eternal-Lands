/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 * \sa soundpage
 */
#ifndef __SOUND_H__
#define __SOUND_H__

/*!
 * playlist_entry is used to read and write the entries for a playlist.
 */
typedef struct {
	char file_name[64]; /*!< the filename of the sound file for this entry */
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time; /*!< duration of this sound file */
} playlist_entry;

extern int have_sound; /*!< flag indicating whether sound is available */
extern int have_music; /*!< flag indicating whether music is available */
extern int sound_on; /*!< flag indicating whether sound is enabled */
extern int music_on; /*!< flag indicating whether music is enabled */
extern int no_sound; /*!< flag inidicating whether the #no_sound ini file directive is used */
extern int playing_music; /*!< flag indicating if music is currently playing */

extern ALfloat sound_gain; /*!< gain for sound effects */
extern ALfloat music_gain; /*!< gain for playing music */

/*!
 * \ingroup sound_effects
 * \brief   stops playing the specified sound effect.
 *
 *      Stops playing the sound that is specified by the given index.
 *
 * \param i index of the sound to stop
 */
void stop_sound(int i);

/*!
 * \ingroup sound_effects
 * \brief adds the file \a sound_file at the position (\a x, \a y) to the list of sounds to play.
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
 * \brief updates the position change of sound played.
 *
 *      Updates the position change of sound played.
 *
 */
void update_position();

/*!
 * \ingroup sound_effects
 * \brief kill all sounds that loop infinitely
 *
 *      kill all the sounds that loop infinitely; usefull when we change maps, etc.
 *
 * \callgraph
 */
void kill_local_sounds();

/*!
 * \ingroup sound_effects
 * \brief turns off playback of sound.
 *
 *      Turns off the playback of sound (effects).
 *
 */
void turn_sound_off();

/*!
 * \ingroup sound_effects
 * \brief turns on playback of sound
 *
 *      Turns on the playback of sound (effects).
 *
 */
void turn_sound_on();

/*!
 * \ingroup other
 * \brief initializes the sound system of EL
 *
 *      Initializes the sound system of EL
 *
 */
void init_sound();

/*!
 * \ingroup other
 * \brief closes the sound system of EL
 *
 *      Shuts down the sound system of EL.
 *
 */
void destroy_sound();

/*!
 * \ingroup sound
 * \brief reallocates the sources of playlist entries.
 *
 *      Reallocates the sources of playlist entries.
 *
 * \retval int  the number of sound sources still in use.
 */
int realloc_sources();

/*!
 * \ingroup sound
 * \brief retrieves a previously loaded buffer at the given index.
 *
 *      Retrieves a previously loaded buffer at the given index. If the sound file at the index \a i is not buffered already, it will be loaded and buffered for future use.
 *
 * \param i index of the file to load
 * \retval ALuint   a handle into the sound buffer.
 */
ALuint get_loaded_buffer(int i);


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
 * \brief plays the ogg file with the given filename.
 *
 *      Plays an ogg file, specified by a filename
 *
 * \param file_name name of the ogg file to play
 *
 * \callgraph
 */
void play_ogg_file(char *file_name);

/*!
 * \ingroup music
 * \brief loads the ogg file with the given filename.
 *
 *      Loads an ogg file, specified by a filename.
 *
 * \param file_name name of the ogg file to load
 */
void load_ogg_file(char *file_name);

/*!
 * \ingroup music
 * \brief starts playing music according the entries in the given playlist
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
 * \brief updates the music and brings the sound system in sync.
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
 * \brief creates a sound stream from the data pointed to by \a buffer
 *
 *      Creates a sound stream, ready for playback, from the data pointed to by \a buffer.
 *
 * \param buffer    handle to a buffer with the sound data
 *
 * \sa ogg_error
 */
void stream_music(ALuint buffer);

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
 * \brief creates an error with the specified code.
 *
 *      Creates an ogg error with the specified code.
 *
 * \param code  error code used to create this error.
 */
void ogg_error(int code);

//*!
// * \ingroup mutex
// * \name Sound thread synchronization -> moved to sound.c
// */
//*! @{ */
//#define	LOCK_SOUND_LIST()	SDL_LockMutex(sound_list_mutex)
//#define	UNLOCK_SOUND_LIST()	SDL_UnlockMutex(sound_list_mutex);
//*! @} */

#endif
