/**
 * @file
 * @ingroup wrapper
 * @brief file i/o functions with support for zip and gzip files
 */
#ifndef _ELFILEWRAPPER_H_
#define _ELFILEWRAPPER_H_

#ifdef _MSC_VER
typedef unsigned int uint_fast32_t;
typedef signed int int_fast32_t;
#else
#include <stdint.h>
#endif //MSVC

#ifdef __cplusplus
extern "C"
{
#endif

struct el_file;

typedef struct el_file* el_file_ptr;

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
 * @return Returns a valid el file pointer or zero on failur.
 */
extern el_file_ptr el_open(const char* file_name);

/**
 * @brief Opens a file without decompressing it.
 *
 * Opens a file read only in binary mode without decompressing the data.
 * @param file_name The name of the file to open.
 * @return Returns a valid el file pointer or zero on failur.
 */
extern el_file_ptr el_open_no_decompress(const char* file_name);

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
extern int_fast32_t el_read(el_file_ptr file, int_fast32_t size, void* buffer);

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
extern int_fast32_t el_seek(el_file_ptr file, int_fast32_t offset, int_fast32_t seek_type);

/**
 * @brief Gets the position in the file.
 *
 * Gets the position in the file previously opend with el_open.
 * @param file The file pointer.
 * @return Returns the position in the file.
 * @see el_open
 */
extern int_fast32_t el_tell(el_file_ptr file);

/**
 * @brief Gets the size of the file.
 *
 * Gets the size of the file previously opend with el_open.
 * @param file The file pointer.
 * @return Returns the size of the file.
 * @see el_open
 */
extern int_fast32_t el_get_size(el_file_ptr file);

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
 * Check if the given file exists.
 * @param file_name The name of the file.
 * @return Returns true if the file exists, else false.
 */
extern int_fast32_t el_file_exists(const char* file_name);

#ifdef __cplusplus
}
#endif

#endif	// _ELFILEWRAPPER_H_

