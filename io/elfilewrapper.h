/*!
 * \file
 * \ingroup wrapper
 * \brief file i/o functions with support for zip and gzip files
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

/*!
 * \brief Inits the zip archive system.
 *
 * Inits the zip archive system. This function is not thread save and must be
 * called befor any other el file function.
 * \see el_open
 */
void init_zip_archives();

/*!
 * \brief Clears the zip archive system.
 *
 * Clears the zip archive system. This function is not thread save and must be
 * called after any other el file function.
 */
void clear_zip_archives();

/*!
 * \brief Unload the zip archive.
 *
 * Unload the zip file and removes it from the list where to search for a file
 * that is opend with el_open. This function is thread save.
 * \param file_name The file name of the zip file.
 * \see el_open
 */
void unload_zip_archive(const char* file_name);

/*!
 * \brief Loads the zip file
 *
 * Loads the zip file and adds it to the list where to search for a file that
 * is opend with el_open. This function is thread save.
 * \param file_name The file name of the zip file.
 * \see el_open
 */
void load_zip_archive(const char* file_name);

/*!
 * \brief Opens a file.
 *
 * Opens a file read only in binary mode. This function is thread save.
 * \param file_name The name of the file to open. This function is thread save.
 * \return Returns a valid el file pointer or zero on failure.
 */
el_file_ptr el_open(const char* file_name);

/*!
 * \brief Opens a file.
 *
 * Opens a file read only in binary mode. Also searches the
 * custom dir when trying to locate the file. This function is thread save.
 * \param file_name The name of the file to open.
 * \return Returns a valid el file pointer or zero on failure.
 */
el_file_ptr el_open_custom(const char* file_name);

/*!
 * \brief Opens a file.
 *
 * Opens a file read only in binary mode, searching also the 
 * configuration dir when trying to locate the file. This function is thread
 * save.
 * \param file_name The name of the file to open.
 * \return Returns a valid el file pointer or zero on failure.
 */
el_file_ptr el_open_anywhere(const char* file_name);

/*!
 * \brief Reads data from the file.
 *
 * Reads data from the file previously opend with el_open.
 * \param file The el file hande to use.
 * \param size The number of bytes to read.
 * \param buffer The buffer for the read data.
 * \return Returns the number of read bytes.
 * \see el_open
 */
Sint64 el_read(el_file_ptr file, Sint64 size, void* buffer);
#ifdef FASTER_STARTUP
/*!
 * \brief Read a float
 *
 * Read a float from the current position in the file and store it \a f.
 * \param file The file to read from
 * \param f    Place to store the float
 * \return 1 on success, 0 on failure
 */
int el_read_float(el_file_ptr file, float *f);
/*!
 * \brief Read an integer
 *
 * Read an integer from the current position in the file and store it \a i.
 * \param file The file to read from
 * \param f    Place to store the integer
 * \return 1 on success, 0 on failure
 */
int el_read_int(el_file_ptr file, int *i);
#endif

/*!
 * \brief Sets the position in the file.
 *
 * Sets the position in the file previously opend with el_open. If seek_type is SEEK_SET, the new
 * position is offset. If seek_type is SEEK_CUR, the new position is the old position plus the
 * offset. If seek_type is SEEK_END, the new position is the file size minus the offset.
 * \param file The file pointer.
 * \param offset The value used for the calculation for the new position.
 * \param seek_type The type of seek. Can only be SEEK_SET, SEEK_END or SEEK_CUR.
 * \return Returns the new position in the file.
 * \see el_open
 */
Sint64 el_seek(el_file_ptr file, Sint64 offset, int seek_type);

/*!
 * \brief Gets the position in the file.
 *
 * Gets the position in the file previously opend with el_open. This function
 * is thread save.
 * \param file The file pointer.
 * \return Returns the position in the file.
 * \see el_open
 */
Sint64 el_tell(el_file_ptr file);

/*!
 * \brief Gets the size of the file.
 *
 * Gets the size of the file previously opend with el_open. This function is
 * thread save.
 * \param file The file pointer.
 * \return Returns the size of the file.
 * \see el_open
 */
Sint64 el_get_size(el_file_ptr file);

/*!
 * \brief Closes a file.
 *
 * Closes a file previously opend with el_open.
 * \param file The file pointer.
 * \see el_open
 */
void el_close(el_file_ptr file);

/*!
 * \brief Gets a pointer to the file data.
 *
 * Gets a memory pointer of the file data previously opend with el_open. The
 * pointer is automaticly freed at closing the file. This function is thread
 * save.
 * \param file The file pointer.
 * \return Returns a memory pointer to the file data.
 * \see el_open
 */
void* el_get_pointer(el_file_ptr file);

/*!
 * \brief Check if a file exists.
 *
 * Check if the given file exists somewhere in the data or update directories.
 * This function is thread save.
 * \param file_name The name of the file.
 * \return Returns true if the file exists, else false.
 * \sa el_file_exists_anywhere()
 */
int el_file_exists(const char* file_name);

/*!
 * \brief Check if a file exists.
 *
 * Check if the given file exists somewhere in the data, update or custom
 * directories. This function is thread save.
 * \param file_name The name of the file.
 * \return Returns true if the file exists, else false.
 * \sa el_file_exists_anywhere()
 */
int el_custom_file_exists(const char* file_name);

/*!
 * \brief Check if a file exists.
 *
 * Check if the given file exists anywhere in the game directories. This
 * function is thread save.
 * \param file_name The name of the file.
 * \return Returns true if the file exists, else false.
 * \sa el_file_exists()
 */
int el_file_exists_anywhere(const char* file_name);

/*!
 * \brief Gets the file name.
 *
 * Gets the file name of the file previously opend with el_open. This function
 * is thread save.
 * \param file The file pointer.
 * \return Returns the file name.
 * \see el_open
 */
const char* el_file_name(const el_file_ptr file);

#ifdef FASTER_MAP_LOAD
/*!
 * \brief Return the checksum of a file
 *
 * Return the CRC32 checksum of the file data. If this has not been computed
 * yet, calculate it first.
 * \param file The file pointer.
 * \return Returns the checksum.
 */
Uint32 el_crc32(el_file_ptr file);

/*!
 * \brief Read a line
 *
 * Read the next line from file \a file and store it in buffer \a str of
 * length \a size. At most \a size-1 bytes are copied, the resulting string
 * is always zero-terminated.
 * \param str  The buffer in which the line is stored
 * \param size The length of \a str
 * \param file The file from which to read the line
 * \return \a str on success, NULL on failure
 */
char *el_fgets(char *str, int size, el_file_ptr file);
#endif // FASTER_MAP_LOAD

#ifdef __cplusplus
}
#endif

#endif	/* UUID_366fd032_5c72_48c3_9229_4089ad0bc93e */

