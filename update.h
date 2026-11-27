/*!
 * \file
 * \ingroup	update
 * \brief	Auto update and download functions.
 */
#ifndef __UPDATE_H__
#define __UPDATE_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// structure for server/port information from mirrors.lst
struct update_server_struct {
	char   host[128];
	int    port;
};

// structure for requesting a file via http+thread
struct http_get_struct {
	struct update_server_struct server;
	char   path[256];
	Uint8  *md5;
	FILE   *fp;
	int    event;
	int    status;
	int    allow_restart;
	size_t thread_index;
};

extern int update_root_win;			/* Handle for the update window */
extern int update_countdown;		/* Counter until the client restarts after an update */

/*!
 * \ingroup	update
 * \brief	Initialize the auto update system
 *
 * 		The function initializes the auto update system and starts downloading if enabled
 */
void    init_update();

/*!
 * \ingroup	update
 * \brief	fill update_server_struct from string
 *
 * 		The function initializes an update_server_struct server_port info from a string
 */
void    fill_update_server_struct(struct update_server_struct *update_server, const char *buf);

/*!
 * \ingroup	update
 * \brief	Clean up the auto update system
 *
 * 		The function cleans up the auto update system on client exit
 */
void    clean_update();

/*!
 * \ingroup	update
 * \brief   Check the results of having downloaded the updates file
 *
 * 		The function checks the results of the having downloaded the updates file and either
 *  starts downloading again, start processing the file, or stops the process
 */
void    handle_update_download(struct http_get_struct *get);

/*!
 * \ingroup	update
 * \brief   Adds a file to the download queue
 *
 *      The functions adds a file to the download queue so that it can be downloaded
 */
 void   add_to_download(const char *filename, const Uint8 *md5);
 
/*!
 * \ingroup	update
 * \brief   handle the results of a background download
 *
 * 		The function handles the result of a download for updating
 */
void    handle_file_download(struct http_get_struct *get);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
