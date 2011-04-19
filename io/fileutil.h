/*!
 * \file
 * \ingroup io
 * \brief file i/o functions with support for xz
 */
#ifndef UUID_962d4106_e9b5_41a2_876f_caa6888adf30
#define UUID_962d4106_e9b5_41a2_876f_caa6888adf30

#include "../platform.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize the crc tables.
 */
void init_crc_tables();

/**
 * @brief Reads a file to memory.
 *
 * Reads a file from the given file to memory. If the file is a xz file,
 * the data is uncompressed before it is written to memory.
 * @param file The file to read from.
 * @param file_size The size of the file to read from.
 * @param buffer The pointer to the var where the memeory buffer should be
 * placed.
 * @param size The pointer to the var where the size of the memory buffer
 * should be placed.
 * @return Zero if no error, else one.
 */
Uint32 file_read(FILE* file, const Uint64 file_size, void** buffer,
	Uint64* size);

/**
 * @brief Reads and uncompress a xz file to memory.
 *
 * Reads and uncompress a xz file from the given file to memory.
 * @param file The file to read from.
 * @param buffer The pointer to the var where the memeory buffer should be
 * placed.
 * @param size The pointer to the var where the size of the memory buffer
 * should be placed.
 * @return Zero if no error, else one.
 */
Uint32 xz_file_read(FILE* file, void** buffer, Uint64* size);

#ifdef __cplusplus
}
#endif

#endif	/* UUID_962d4106_e9b5_41a2_876f_caa6888adf30 */

