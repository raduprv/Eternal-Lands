/*!
 * \file
 * \ingroup misc_utils
 * \brief generating descriptive error messages and write them to specific targets.
 */
#ifndef __ERRORS_H__
#define __ERRORS_H__

/*!
 * \ingroup misc_utils
 * \brief   empties the error_log.txt file
 *
 *      Clears the error_log.txt file.
 *
 * \return None
 */
void clear_error_log();

/*!
 * \ingroup misc_utils
 * \brief   logs the given \a message to the error_log.txt file.
 *
 *      Logs the given \a message to the error_log.txt file.
 *
 * \param message   the message to log
 * \return None
 */
void log_error(const Uint8 *message);

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
 * \return None
 */
void log_error_detailed(const Uint8 *message, const Uint8 *file, const Uint8 *function, Uint32 line);

/*!
 * \ingroup misc_utils
 * \brief       empties the connection_log.txt file
 *
 *      Clears the connection_log.txt file
 *
 * \return None
 */
void clear_conn_log();

/*!
 * \ingroup misc_utils
 * \brief       logs connection data to the connection_log.txt file.
 *
 *      Logs connection data to the connection_log.txt file.
 *
 * \param in_data           the data to write to the log
 * \param data_lenght       the length of \a in_data
 * \return None
 */
void log_conn(const Uint8 *in_data, Uint32 data_lenght);

/*!
 * \name    LogError macro
 */
/*! @{ */
#ifdef	DEBUG
#define	LogError(msg)	log_error_detailed(msg, __FILE__, __FUNCTION__, __LINE__) /*!< detailed log of error */
#else
#define	LogError(msg)	log_error(msg) /*! log the error */
#endif	//DEBUG
/*! @} */


#ifdef EXTRA_DEBUG
/*!
 * \name    ERR macro
 */
#define ERR() log_func_err(__FILE__, __FUNCTION__, __LINE__); /*!< additional macro to log function error messages */

/*!
 * \ingroup
 * \brief   logs error messages to the function_log.txt file.
 *
 *      Logs error mesages to the function_log.txt file. The \a file and \a function name as well as the \a line in the source file where the error occurred are reported.
 *
 * \param file      filename of the source file where the error occurred
 * \param func      function name where the error occurred
 * \param line      line in the source file where the error occurred
 * \return None
 */
void log_func_err(const Uint8 * file, const Uint8 * func, Uint32 line);
#endif

#endif
