/*!
 * \file
 * \ingroup sound
 * \brief Music and sound effects and their handling in EL.
 */
#ifndef __SOUND_H__
#define __SOUND_H__

#define max_buffers 9 /*!< max number of buffers */
#define max_sources 16 /*!< max. number of sources */

#define BUFFER_SIZE (4096 * 16) /*!< size of one buffer */
#define SLEEP_TIME 500 /*! sleep time in ms, between music or sound effects */

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
 * \param i index
 * \return None
 */
void stop_sound(int i);

/*!
 * \ingroup sound_effects
 * \brief add_sound_object
 *
 *      add_sound_object(int,int,int,int,int)
 *
 * \param sound_file    A handle for the sound file to use
 * \param x             x
 * \param y             y
 * \param positional    positional
 * \param loops         loops
 * \return int          int
 */
int add_sound_object(int sound_file,int x, int y,int positional,int loops);

/*!
 * \ingroup sound_effects
 * \brief update_position
 *
 *      update_position()
 *
 * \return None
 */
void update_position();

/*!
 * \ingroup sound_effects
 * \brief kill_local_sounds
 *
 *      kill_local_sounds()
 *
 * \return None
 */
void kill_local_sounds();

/*!
 * \ingroup sound_effects
 * \brief turns off playback of sound.
 *
 *      Turns off the playback of sound (effects).
 *
 * \return None
 */
void turn_sound_off();

/*!
 * \ingroup sound_effects
 * \brief turns on playback of sound
 *
 *      Turns on the playback of sound (effects).
 *
 * \return None
 */
void turn_sound_on();

/*!
 * \ingroup other
 * \brief initializes the sound system of EL
 *
 *      Initializes the sound system of EL
 *
 * \return None
 */
void init_sound();

/*!
 * \ingroup other
 * \brief closes the sound system of EL
 *
 *      Shuts down the sound system of EL.
 *
 * \return None
 */
void destroy_sound();

/*!
 * \ingroup sound
 * \brief reallocates the sources of playlist entries.
 *
 *      Reallocates the sources of playlist entries.
 *
 * \return int
 */
int realloc_sources();

/*!
 * \ingroup sound
 * \brief retrieves a previously loaded buffer at the given index.
 *
 *      Retrieves a previously loaded buffer at the given index.
 *
 * \param i index
 * \return ALuint
 */
ALuint get_loaded_buffer(int i);


/*!
 * \ingroup music
 * \brief Retrieves the playlist for the current map.
 *
 *      Retrieves the playlist, associated with the current map.
 *
 * \return None
 */
void get_map_playlist();

/*!
 * \ingroup music
 * \brief plays the ogg file with the given filename.
 *
 *      Plays an ogg file, specified by a filename
 *
 * \param file_name name of the ogg file to play
 * \return None
 */
void play_ogg_file(char *file_name);

/*!
 * \ingroup music
 * \brief loads the ogg file with the given filename.
 *
 *      Loads an ogg file, specified by a filename.
 *
 * \param file_name name of the ogg file to load
 * \return None
 */
void load_ogg_file(char *file_name);

/*!
 * \ingroup music
 * \brief starts playing music according the entries in the given playlist
 *
 *      Start music playback, according the playlist entries in the given playlist.
 *
 * \param list  a handle for the playlist to use for playback.
 * \return None
 */
void play_music(int list);

/*!
 * \ingroup music
 * \brief update_music
 *
 *      update_music(void*)
 *
 * \param dummy currently not used
 * \return int
 */
int update_music(void *dummy);

/*!
 * \ingroup music
 * \brief creates a sound stream from the data in buffer
 *
 *      Creates a sound stream, ready for playback, from the given data.
 *
 * \param buffer    handle to a buffer with the sound data
 * \return None
 */
void stream_music(ALuint buffer);

/*!
 * \ingroup music
 * \brief Turns music off and stops playback of music.
 *
 *      Turns music off and stops playback of music.
 *
 * \return None
 */
void turn_music_off();

/*!
 * \ingroup music
 * \brief Turns music on and starts playback of music
 *
 *      Turns music on and starts playback of music.
 *
 * \return None
 */
void turn_music_on();

/*!
 * \ingroup music
 * \brief creates an error with the specified code.
 *
 *      Creates an ogg error with the specified code.
 *
 * \param code  error code used to create this error.
 * \return None
 */
void ogg_error(int code);

/*!
 * \ingroup mutex
 * \name Sound thread synchronization
 */
/*! @{ */
#define	lock_sound_list()	SDL_LockMutex(sound_list_mutex)
#define	unlock_sound_list()	SDL_UnlockMutex(sound_list_mutex);
/*! @} */

#endif

