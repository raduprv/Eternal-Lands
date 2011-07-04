/****************************************************************************
 *            elloggingwrapper.h
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	UUID_f1a7fcd3_705c_45f3_b3df_7d572d295698
#define	UUID_f1a7fcd3_705c_45f3_b3df_7d572d295698

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	llt_error = 0,
	llt_warning = 1,
	llt_info = 2,
	llt_debug = 3,
	llt_debug_verbose = 4
} LogLevelType;

/**
 * @ingroup logging
 *
 * Inits the logging.
 */
void init_logging(const char* log_file_name);

/**
 * @ingroup logging
 *
 * Finish the logging.
 */
void exit_logging();

/**
 * @ingroup logging
 *
 * Returns the current log level.
 */
LogLevelType get_log_level();

/**
 * @ingroup logging
 *
 * Sets the current log level.
 * @param log_level The new log level.
 */
void set_log_level(const LogLevelType log_level);

/**
 * @ingroup logging
 *
 * Logs the given error.
 * @param file File of the error.
 * @param line Line of the error.
 * @param message Error message.
 */
void log_error(const char* file, const Uint32 line, const char* message, ...);

/**
 * @ingroup logging
 *
 * Logs the given warning.
 * @param file File of the warning.
 * @param line Line of the warning.
 * @param message Warning message.
 */
void log_warning(const char* file, const Uint32 line, const char* message, ...);

/**
 * @ingroup logging
 *
 * Logs the given info.
 * @param file File of the info.
 * @param line Line of the info.
 * @param message Info message.
 */
void log_info(const char* file, const Uint32 line, const char* message, ...);

/**
 * @ingroup logging
 *
 * Logs the given debug message.
 * @param file File of the debug message.
 * @param line Line of the debug message.
 * @param message Debug message.
 */
void log_debug(const char* file, const Uint32 line, const char* message, ...);

/**
 * @ingroup logging
 *
 * Logs the given debug verbose message.
 * @param file File of the debug verbose message.
 * @param line Line of the debug verbose message.
 * @param message Debug verbose message.
 */
void log_debug_verbose(const char* file, const Uint32 line,
	const char* message, ...);

/**
 * @ingroup logging
 *
 * Enters the given debug mark.
 * @param file File of the debug mark.
 * @param line Line of the debug mark.
 * @param name Name of the debug mark.
 */
void enter_debug_mark(const char* file, const Uint32 line,
	const char* name);

/**
 * @ingroup logging
 *
 * Leaves the given debug mark.
 * @param file File of the debug mark.
 * @param line Line of the debug mark.
 * @param name Name of the debug mark.
 */
void leave_debug_mark(const char* file, const Uint32 line,
	const char* name);

/**
 * @ingroup logging
 *
 * Prints and changes the current log level.
 * @param text The new log level to use or empty
 * @param len The length of the text.
 */
int command_log_level(char *text, int len);

void init_thread_log(const char* name);

#define LOG_ERROR(msg, args ...) log_error(__FILE__, __LINE__, msg, ## args)
#define LOG_WARNING(msg, args ...) log_warning(__FILE__, __LINE__, msg,	\
	## args)
#define LOG_INFO(msg, args ...) log_info(__FILE__, __LINE__, msg, ## args)
#ifdef FASTER_MAP_LOAD
#define LOG_DEBUG(msg, args ...)\
	do\
	{\
		if (get_log_level() >= llt_debug)\
			log_debug(__FILE__, __LINE__, msg, ## args);\
	} while(0)
#define LOG_DEBUG_VERBOSE(msg, args ...)\
	do\
	{\
		if (get_log_level() >= llt_debug_verbose)\
			log_debug_verbose(__FILE__, __LINE__, msg, ## args);\
	} while(0)
#else  // FASTER_MAP_LOAD
#define LOG_DEBUG(msg, args ...) log_debug(__FILE__, __LINE__, msg, ## args)
#define LOG_DEBUG_VERBOSE(msg, args ...) log_debug_verbose(__FILE__,	\
	__LINE__, msg, ## args)
#endif // FASTER_MAP_LOAD
#define ENTER_DEBUG_MARK(name) enter_debug_mark(__FILE__, __LINE__, name)
#define LEAVE_DEBUG_MARK(name) leave_debug_mark(__FILE__, __LINE__, name)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif	/* UUID_f1a7fcd3_705c_45f3_b3df_7d572d295698 */

