/*!
 * \file
 * \ingroup misc_utils
 * \brief generating descriptive error messages and write them to specific targets.
 */
#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <SDL_types.h>
#include "elloggingwrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup misc_utils
 * \brief       empties the connection_log.txt file
 *
 *      Clears the connection_log.txt file
 *
 */
void clear_conn_log();

/*!
 * \ingroup misc_utils
 * \brief       logs connection data to the connection_log.txt file.
 *
 *      Logs connection data to the connection_log.txt file.
 *
 * \param in_data           the data to write to the log
 * \param data_length       the length of \a in_data
 */
void log_conn(const Uint8 *in_data, Uint16 data_length);

#ifdef EXTRA_DEBUG
/*! \name    ERR macro 
 * @{ */
#define ERR() log_func_err(__FILE__, __FUNCTION__, __LINE__); /*!< additional macro to log function error messages */
/* @} */

/*!
 * \ingroup misc_utils
 * \brief   logs error messages to the function_log.txt file.
 *
 *      Logs error mesages to the function_log.txt file. The \a file and \a function name as well as the \a line in the source file where the error occurred are reported.
 *
 * \param file      filename of the source file where the error occurred
 * \param func      function name where the error occurred
 * \param line      line in the source file where the error occurred
 *
 * \callgraph
 */
void log_func_err(const char * file, const char * func, unsigned line);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
