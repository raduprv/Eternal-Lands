/**
 * @file
 * @ingroup wrapper
 * @brief file i/o functions with support for zip and gzip files
 */
#ifndef UUID_366fd032_5c72_48c3_9229_4089ad0bc93e
#define UUID_366fd032_5c72_48c3_9229_4089ad0bc93e

#include "../platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct el_file_t el_file_t;

typedef el_file_t* el_file_ptr;

/**
 * @brief Adds a zip file to the search list for files.
 *
 * Adds a zip file to the list where to search for a file that is opend with
 * el_open.
 * @param file_name The file name of the zip file.
 * @see el_open
 */
extern void add_zip_archive(const char* file_name);

/**
 * @brief Opens a file.
 *
 * Opens a file read only in binary mode.
 * @param file_name The name of the file to open.
 * @return Returns a valid el file pointer or zero on failure.
 */
extern el_file_ptr el_open(const char* file_name);

/**
 * @brief Opens a file.
 *
 * Opens a file read only in binary mode. Also searches the
 * custom dir when trying to locate the file.
 * @param file_name The name of the file to open.
 * @return Returns a valid el file pointer or zero on failure.
 */
extern el_file_ptr el_open_custom(const char* file_name);

/**
 * @brief Opens a file.
 *
 * Opens a file read only in binary mode, searching also the 
 * configuration dir when trying to locate the file.
 * @param file_name The name of the file to open.
 * @return Returns a valid el file pointer or zero on failure.
 */
extern el_file_ptr el_open_anywhere(const char* file_name);

/**
 * @brief Reads data from the file.
 *
 * Reads data from the file previously opend with el_open.
 * @param file The el file hande to use.
 * @param size The number of bytes to read.
 * @param buffer The buffer for the read data.
 * @return Returns the number of read bytes.
 * @see el_open
 */
extern Sint64 el_read(el_file_ptr file, Sint64 size, void* buffer);

/**
 * @brief Sets the position in the file.
 *
 * Sets the position in the file previously opend with el_open. If seek_type is SEEK_SET, the new
 * position is offset. If seek_type is SEEK_CUR, the new position is the old position plus the
 * offset. If seek_type is SEEK_END, the new position is the file size minus the offset.
 * @param file The file pointer.
 * @param offset The value used for the calculation for the new position.
 * @param seek_type The type of seek. Can only be SEEK_SET, SEEK_END or SEEK_CUR.
 * @return Returns the new position in the file.
 * @see el_open
 */
extern Sint64 el_seek(el_file_ptr file, Sint64 offset, int seek_type);

/**
 * @brief Gets the position in the file.
 *
 * Gets the position in the file previously opend with el_open.
 * @param file The file pointer.
 * @return Returns the position in the file.
 * @see el_open
 */
extern Sint64 el_tell(el_file_ptr file);

/**
 * @brief Gets the size of the file.
 *
 * Gets the size of the file previously opend with el_open.
 * @param file The file pointer.
 * @return Returns the size of the file.
 * @see el_open
 */
extern Sint64 el_get_size(el_file_ptr file);

/**
 * @brief Closes a file.
 *
 * Closes a file previously opend with el_open.
 * @param file The file pointer.
 * @see el_open
 */
extern void el_close(el_file_ptr file);

/**
 * @brief Gets a pointer to the file data.
 *
 * Gets a memory pointer of the file data previously opend with el_open. The
 * pointer is automaticly freed at closing the file.
 * @param file The file pointer.
 * @return Returns a memory pointer to the file data.
 * @see el_open
 */
extern void* el_get_pointer(el_file_ptr file);

/**
 * @brief Check if a file exists.
 *
 * Check if the given file exists somewhere in the data or update directories.
 * @param file_name The name of the file.
 * @return Returns true if the file exists, else false.
 * @sa el_file_exists_anywhere()
 */
extern int el_file_exists(const char* file_name);

/**
 * @brief Check if a file exists.
 *
 * Check if the given file exists somewhere in the data, update or custom directories.
 * @param file_name The name of the file.
 * @return Returns true if the file exists, else false.
 * @sa el_file_exists_anywhere()
 */
extern int el_custom_file_exists(const char* file_name);

/**
 * @brief Check if a file exists.
 *
 * Check if the given file exists anywhere in the game directories.
 * @param file_name The name of the file.
 * @return Returns true if the file exists, else false.
 * @sa el_file_exists()
 */
extern int el_file_exists_anywhere(const char* file_name);

/**
 * @brief Gets the file name.
 *
 * Gets the file name of the file previously opend with el_open.
 * @param file The file pointer.
 * @return Returns the file name.
 * @see el_open
 */
extern const char* el_file_name(el_file_ptr file);

#ifdef __cplusplus
}
#endif

#endif	/* UUID_366fd032_5c72_48c3_9229_4089ad0bc93e */

