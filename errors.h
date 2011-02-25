/*!
 * \file
 * \ingroup misc_utils
 * \brief generating descriptive error messages and write them to specific targets.
 */
#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <SDL_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup misc_utils
 *
 *      Creates the mutex for the error logs.
 *
 */
void create_error_mutex();

/*!
 * \ingroup misc_utils
 *
 *      Destroys the mutex for the error logs.
 *
 */
void destroy_error_mutex();

/*!
 * \ingroup misc_utils
 * \brief   empties the error_log.txt file
 *
 *      Clears the error_log.txt file.
 *
 */
void clear_error_log();

/*!
 * \ingroup misc_utils
 * \brief   logs the given \a message to the error_log.txt file.
 *
 *      Logs the given \a message to the error_log.txt file.
 *
 * \param message   the message to log
 */
void log_error(const char *message, ...);

/*!
 * \ingroup misc_utils
 * \brief   logs a detailed \a message, that enables location of the error within the source.
 *
 *      Logs a detailed \a message, that enables location of the error. The parameters logged are the \a file and \a function name as well as the line where the error occurred.
 *
 * \param message       the message to log
 * \param file          filename of the source file where the error occurred
 * \param function      function in which the error occurred
 * \param line          line in the source file where the error occrred
 */
void log_error_detailed(const char *message, const char *file, const char *function, unsigned line, ...);

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

/*!
 * \name    LOG_ERROR macro
 */
/*! @{ */
#ifdef	DEBUG
 #ifdef _MSC_VER
  #define LOG_ERROR log_error //MSVC doesn't support variadic macros.
 #else
  #define LOG_ERROR(msg, args ...) log_error_detailed(msg, __FILE__, __FUNCTION__, __LINE__, ## args) /*!< detailed log of error */
 #endif //_MSC_VER
#else
 #define LOG_ERROR log_error /*! log the error */
#endif	//DEBUG
/*! @} */


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

/*!
 * \ingroup misc_utils
 * \brief   logs the given \a message to the infos.log file.
 *
 *      Logs the given \a message to the infos.log file.
 *
 * \param message   the message to log
 */
void log_info(const char* message, ...);

#ifdef	EXTRA_DEBUG
#define LOG_EXTRA_INFO log_info
#endif	// EXTRA_DEBUG

#ifdef __cplusplus
} // extern "C"
#endif

#endif
