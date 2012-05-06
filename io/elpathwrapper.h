/**
 * @file
 * @ingroup wrapper
 * @brief file i/o wrapping and checking for the config_dir
 */

#ifndef EL_PATH_WRAPPER_H
#define EL_PATH_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif //C++

#include <stdio.h>
#include <sys/types.h>

/**
 * @brief Gets the base directory for config files
 *
 * Get the base config directory. Most config and update files are stored in sub-directories of this one.
 * @return Returns a string with the path on success, or an empty string (indicating the use of the current directory, usually data_dir) on failure
 */
const char * get_path_config_base(void);

/**
 * @brief Gets the directory for config files
 *
 * Get the directory where we should be storing config files
 * @return Returns a string with the path on success, or an empty string (indicating the use of the current directory, usually data_dir) on failure
 */
const char * get_path_config(void);

/**
 * @brief Gets the directory for auto-updated files
 *
 * Get the directory where we should be storing auto-updated files
 * @return Returns a string with the path on success, or an empty string (indicating an error) on failure
 */
const char * get_path_updates(void);

/**
 * @brief Gets the directory for updated custom files
 *
 * Get the directory where we should be storing updated custom files
 * @return Returns a string with the path on success, or an empty string (indicating an error) on failure
 */
const char * get_path_custom(void);

/**
 * @brief fopen()s a config file
 *
 * Gets the config dir, based on platform, and attempts to open the given filename.
 * If file not found, fallback and try to open the filename in the current directory.
 * @param filename The name of the file in the config_dir to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, or a NULL on failure
 */
FILE * open_file_config(const char* filename, const char* mode);

/**
 * @brief fopen()s a config file
 *
 * Gets the config dir, based on platform, and attempts to open the given filename.
 * If file not found, no current directory fallback is tried.
 * @param filename The name of the file in the config_dir to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, or a NULL on failure
 */
FILE * open_file_config_no_local(const char* filename, const char* mode);

/**
 * @brief fopen()s a file in the directory datadir
 *
 * Attempts to open the given filename in datadir
 * @param filename The name of the file to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, or a NULL on failure
 */
FILE * open_file_data(const char* filename, const char* mode);

/**
 * @brief fopen()s a file in the directory: datadir/languages/lang
 *
 * Attempts to open the given filename in datadir/languages/lang
 * @param base_path The name of the directory to attempt to open the file in
 * @param filename The name of the file to open
 * @param mode The file mode to use to open the file (read/write, binary/text, etc)
 * @return Returns a FILE* to the opened file on success, either in 'lang' or in 'en' (as a failover), or a NULL on failure
 */
FILE * open_file_lang(const char* filename, const char* mode);

/**
 * @brief Creates the given path
 *
 * Attempts to create the given path, stepping through the given directory names
 * @param path The path to attempt to create
 * @param relative_only If non-zero, create only paths relative to the 
 *                      current directory
 * @return Returns 1 on success, 0 on failure
 */
int mkdir_tree(const char *path, int relative_only);

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
 * @param from_file The name of the file to move
 * @param to_file The name of the file to create
 * @return Returns the result of the internal rename() call
 */
int move_file_to_updates(const char* from_file, char* to_file, int custom);

/**
 * @brief Check if temp file is valid
 *
 * Checks the MD5 checksum of the downloaded temporary file configdir/tmp to assure the
 * download was accurate.
 *
 * @param filename	The name of the file to the md5 of
 * @param md5		The checksum given in the updates list to indicate the downloaded version
 * @return Returns 1 if valid, 0 if not
 */
int file_temp_check(const char * filename, const unsigned char * md5);

/**
 * @brief Check if file update is needed
 *
 * Checks the MD5 checksum of files in datadir and configdir/updates/(ver)/ or configdir/custom/
 * depending on the flag custom. If the version in datadir is correct, attempts to remove the file
 * in configdir. If neither is correct, returns 1 to indicate an update is required.
 *
 * @param filename	The name of the file to check for updates
 * @param md5		The checksum given in the updates list to indicate the newest version
 * @param custom	Flag specifying the configdir/custom/ directory instead of the default
 * @return Returns 1 if an update is needed, 0 if not
 */
int file_update_check(char * filename, const unsigned char * md5, int custom);

/**
 * @brief Check for valid datadir
 *
 * Checks if we can stat() datadir. If not, failover to current directory.
 */
void file_check_datadir(void);

/**
 * @brief Removes old auto-update files
 *
 * Removes old versions of configdir/updates
 */
void file_update_clear_old(void);

/**
 * @brief Attempts to remove a file from configdir/updates/(ver)/ or configdir/custom/
 *
 * Removes a given file from configdir/updates/(ver)/ or configdir/custom/ depending on input flag
 * Note: We never remove a file from the original install!
 *
 * @param filename	The name of the file to remove
 * @param custom	Flag specifying the configdir/custom/ directory instead of the default
 * @return As per remove()
 */
void remove_file_updates(char * filename, int custom);

/**
 * @brief Check for valid configdir
 *
 * Checks if we can stat() configdir.
 */
int check_configdir(void);

/**
 * @brief Copies a file
 *
 * Copies a file, given the source and destination filenames. Does not overwrite the target file (ie, only
 * copies if target file does not exist).
 *
 * @param source	The source file name 
 * @param custom	The destination file name
 * @return 0 on success, -1 if target file exists, -2 if invalid source file, -3 if cannot create target file, -4 if IO error.
 */
int copy_file(const char *source, const char *dest);

/**
 * @brief Check if a specified file exists in the config_dir
 *
 * @param filename The name of the file in the config_dir to check
 * @return 1 if file exists in config_dir, 0 if it does not exist, -1 if some error occurred
 */
int file_exists_config( const char *filename );

/**
 * @brief Get the size of the specified file from the config_dir
 *
 * @param filename The name of the file in the config_dir
 * @return size if file exists in config_dir, -1 if the size check fails (may not exist)
 */
off_t get_file_size_config( const char *filename );

/**
 * @brief Rename a specified file in the config_dir
 *
 * @param old_filename The original name of the file in the config_dir
 * @param new_filename The new name for the file
 * @return 0 if successful, -1 if some error occurred
 */
int file_rename_config( const char *old_filename, const char *new_filename );

/**
 * @brief Remove a specified file in the config_dir
 *
 * @param filename The name of the file in the config_dir we wish to remove
 * @return 0 if successful, -1 if some error occurred
 */
int file_remove_config( const char *filename );


#ifdef __cplusplus
}
#endif //C++


#endif //EL_PATH_WRAPPER_H
