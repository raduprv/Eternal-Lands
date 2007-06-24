/**
 * @file
 * @ingroup wrapper
 * @brief file i/o wrapping and checking for the config_dir
 */

#ifndef EL_PATH_WRAPPER_H
#define EL_PATH_WRAPPER_H
#ifdef NEW_FILE_IO

#ifdef __cplusplus
extern "C" {
#endif //C++

#include <stdio.h>

/**
 * @brief Gets the directory for config files
 *
 * Get the directory where we should be storing config files
 * Remember to free() the returned data when finished (Ideally, you should use open_file_* functions instead)
 * @return Returns a string with the path on success, or an empty string (indicating the use of the current directory, usually data_dir) on failure
 */
char * get_path_config(void);

/**
 * @brief fopen()s a config file
 *
 * Gets the config dir, based on platform, and attempts to open the given filename
 * @param filename The name of the file in the config_dir to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, or a NULL on failure
 */
FILE * open_file_config(const char* filename, const char* mode);

/**
 * @brief fopen()s a file in the directory base_path
 *
 * Attempts to open the given filename in base_path
 * @param base_path The name of the directory to attempt to open the file in
 * @param filename The name of the file to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, or a NULL on failure
 */
FILE * open_file_data(const char* base_path, const char* filename, const char* mode);

/**
 * @brief fopen()s a file in the directory: base_path/languages/lang
 *
 * Attempts to open the given filename in base_path/languages/lang
 * @param base_path The name of the directory to attempt to open the file in
 * @param filename The name of the file to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @param lang The language directory to attempt to use
 * @return Returns a FILE* to the opened file on success, either in 'lang' or in 'en' (as a failover), or a NULL on failure
 */
FILE * open_file_lang(const char* base_path, const char* filename, const char* mode, const char* lang);

/**
 * @brief Creates the given path
 *
 * Attempts to create the given path, stepping through the given directory names
 * @param path The path to attempt to create
 * @return Returns 1 on success, 0 on failure
 */
int mkdir_tree(const char *path);

/**
 * @brief Creates the given folder in configdir
 *
 * Attempts to create the given folder in configdir
 * @param path The path to attempt to create
 * @return Returns the result of the internal mkdir() call
 */
int mkdir_config(const char *path);

/**
 * @brief rename()s a file from configdir to datadir
 *
 * Attempts to move a file in configdir into datadir
 * @param base_path The name of the directory to attempt to move the file from
 * @param from_file The name of the file to move
 * @param to_file The name of the file to create
 * @return Returns the result of the internal rename() call
 */
int move_file_to_data(const char* base_path, const char* from_file, const char* to_file);

/**
 * @brief Check if file update is needed
 *
 * Checks the MD5 checksum of files in datadir and configdir/updates; if the newest version
 * of the file is in /updates, attempts to move to datadir; if the version in datadir is
 * correct, attempts to remove the file in configdir/updates. If neither is correct,
 * returns 1 to indicate an update is required.
 *
 * @param filename The name of the file to check for updates
 * @param md5 The checksum given in the updates list to indicate the newest version
 * @return Returns 1 if an update is needed, 0 if not
 */
int file_update_check(const char * base_path, const char * filename, const unsigned char * md5);

/**
 * @brief Removes old auto-update files
 *
 * Removes old versions of configdir/updates
 */
void file_update_clear_old(void);

/**
 * @brief Attempts to remove a file from datadir
 *
 * Removes a given file from datadir and configdir/updates
 *
 * @param base_path The datadir in use
 * @param filename The name of the file to remove
 * @return As per remove()
 */
void remove_file_data(const char* base_dir, const char * filename);

#ifdef __cplusplus
}
#endif //C++


#endif //NEW_FILE_IO
#endif //EL_PATH_WRAPPER_H
