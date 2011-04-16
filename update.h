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

// structure for requesting a file via http+thread
struct http_get_struct {
	char   server[128];
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
 * \brief   Check the results of having downloaded the updates file
 *
 * 		The function checks the results of the having downloaded the updates file and either
 *  starts downloading again, start processing the file, or stops the process
 */
void    handle_update_download(struct http_get_struct *get);

/*!
 * \ingroup	update
 * \brief   Starts checking what files need updating
 *
 * 		The function starts the background thread fo checking MD5's against the update
 *  file list to see what files need downloading. As a file is found to be incorrect it
 *  enqueues it to be downloaded.
 */
void    do_updates();

/*!
 * \ingroup	update
 * \brief   The background update & MD5 handler
 *
 * 		The function to do the actual background processing of the update list file
 */
int    do_threaded_update(void *ptr);

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

/*!
 * \ingroup	update
 * \brief	A threaded implementation of the http-GET
 *
 * 		The function gets the given file from the server and writes it to the open file pointed to by fp.
 *  This version does the get in a separate thread and sends an event when done. it is up to the EVENT handler
 *	to close the handle & free the spec pointer in data1
 *
 * \param	server The server you're getting the file from
 * \param	path The path on the server
 * \param	fp The file you're saving the file to...
 * \param   event The SDL event to issue when the download is complete
 */
void http_threaded_get_file(char *server, char *path, FILE *fp, Uint8 *md5, Uint32 event);

/*!
 * \ingroup	update
 * \brief	The background download handler
 *
 * 		The function does the actual download and returns the event & spec when complete
 *
 * \param   spec The spec & event data to return when finished
 */
int http_get_file_thread_handler(void *specs);


/*!
 * \ingroup	update
 * \brief	A simple implementation of the http-GET
 *
 * 		The function gets the given file from the server and writes it to the open file pointed to by fp.
 *
 * \param	server The server you're getting the file from
 * \param	path The path on the server
 * \param	fp The file you're saving the file to...
 */
int http_get_file(char *server, char *path, FILE *fp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
