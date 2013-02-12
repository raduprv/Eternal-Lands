/*!
 * \file
 * \ingroup url
 * \brief Store, use and display URLs seen in chat.
 */
#ifndef __URL_H__
#define __URL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup url
 * \brief   Clears the list of stored URL freeing all allocated memory.
 *
 */
void destroy_url_list(void);

/*!
 * \ingroup url
 * \brief   Implements the #url command.
 *
 * \param text any text that follows the command
 * \param len the number of characters in the string.
 *
 */
int url_command(const char *text, int len);

/*!
 * \ingroup url
 * \brief   Opens the last seen URL.
 *
 */
void open_last_seen_url(void);

/*!
 * \ingroup url
 * \brief   Saves the number of URL for comparison later using num_new_url().
 * 
 */
void save_url_count(void);

/* return the number of url since last*/
/*!
 * \ingroup url
 * \brief   Return the number of URL seen since last save_url_count() call.
 *
 */
int num_new_url(void);

/*!
 * \ingroup url
 * \brief   Stores any URL found in \a source_string.
 *
 * \param source_string the string that potentaill contains URLs
 * \param len           the length of \a source_string.
 *
 */
void find_all_url(const char *source_string, const int len);

/*!
 * \ingroup url
 * \brief   Displays the window of stored URLs.
 *
 */
void fill_url_window(void);

/*!
 * \ingroup url
 * \brief Opens a url in the configured browser
 *
 */
void open_web_link(const char * url);

extern char browser_name[120];	/*!< a buffer that contains the name of the browser we should use */
extern int url_win_x; 			/*!< the current x coordinate value of the url window */
extern int url_win_y; 			/*!< the current y coordinate value of the url window */
extern int url_win; 			/*!< the id of the url window */
extern char LOGO_URL_LINK[128];		/*!< the link clicking the EL logo sends you to */

#ifdef __cplusplus
} // extern "C"
#endif

#endif
