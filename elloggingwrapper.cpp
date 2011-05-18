/****************************************************************************
 *            elloggingwrapper.cpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "elloggingwrapper.h"
#include "engine/logging.hpp"
#include "io/elpathwrapper.h"
#include "asc.h"
#include <cassert>

namespace el = eternal_lands;

extern "C" void init_logging(const char* log_file_name)
{
	char str[1024];

	snprintf(str, sizeof(str), "%s%s", get_path_config(), log_file_name);

	el::init_logging(str);
}

extern "C" void exit_logging()
{
	el::exit_logging();
}

extern "C" LogLevelType get_log_level()
{
	return static_cast<LogLevelType>(el::get_log_level());
}

extern "C" void set_log_level(const LogLevelType log_level)
{
	el::set_log_level(static_cast<el::LogLevelType>(log_level));
}

extern "C" void log_error(const char* file, const Uint32 line,
	const char* message, ...)
{
	va_list ap;
	char err_msg[512];

	assert(message != 0);
	assert(strlen(message) > 0);

	if (el::get_log_level() < el::llt_error)
	{
		return;
	}

	memset(err_msg, 0, sizeof(err_msg));

	if (message != 0)
	{
		va_start(ap, message);

        	vsnprintf(err_msg, sizeof(err_msg), message, ap);
        	err_msg[sizeof(err_msg) - 1] = '\0';

		va_end(ap);
	}

	el::log_message(el::llt_error, err_msg, file, line);
}

extern "C" void log_warning(const char* file, const Uint32 line,
	const char* message, ...)
{
	va_list ap;
	char err_msg[512];

	assert(message != 0);
	assert(strlen(message) > 0);

	if (el::get_log_level() < el::llt_warning)
	{
		return;
	}

	memset(err_msg, 0, sizeof(err_msg));

	if (message != 0)
	{
		va_start(ap, message);

        	vsnprintf(err_msg, sizeof(err_msg), message, ap);
        	err_msg[sizeof(err_msg) - 1] = '\0';

		va_end(ap);
	}

	el::log_message(el::llt_warning, err_msg, file, line);
}

extern "C" void log_info(const char* file, const Uint32 line,
	const char* message, ...)
{
	va_list ap;
	char err_msg[512];

	assert(message != 0);
	assert(strlen(message) > 0);

	if (el::get_log_level() < el::llt_info)
	{
		return;
	}

	memset(err_msg, 0, sizeof(err_msg));

	if (message != 0)
	{
		va_start(ap, message);

        	vsnprintf(err_msg, sizeof(err_msg), message, ap);
        	err_msg[sizeof(err_msg) - 1] = '\0';

		va_end(ap);
	}

	el::log_message(el::llt_info, err_msg, file, line);
}

extern "C" void log_debug(const char* file, const Uint32 line,
	const char* message, ...)
{
	va_list ap;
	char err_msg[512];

	assert(message != 0);
	assert(strlen(message) > 0);

	if (el::get_log_level() < el::llt_debug)
	{
		return;
	}

	memset(err_msg, 0, sizeof(err_msg));

	if (message != 0)
	{
		va_start(ap, message);

        	vsnprintf(err_msg, sizeof(err_msg), message, ap);
        	err_msg[sizeof(err_msg) - 1] = '\0';

		va_end(ap);
	}

	el::log_message(el::llt_debug, err_msg, file, line);
}

extern "C" void log_debug_verbose(const char* file, const Uint32 line,
	const char* message, ...)
{
	va_list ap;
	char err_msg[512];

	assert(message != 0);
	assert(strlen(message) > 0);

	if (el::get_log_level() < el::llt_debug_verbose)
	{
		return;
	}

	memset(err_msg, 0, sizeof(err_msg));

	if (message != 0)
	{
		va_start(ap, message);

        	vsnprintf(err_msg, sizeof(err_msg), message, ap);
        	err_msg[sizeof(err_msg) - 1] = '\0';

		va_end(ap);
	}

	el::log_message(el::llt_debug_verbose, err_msg, file, line);
}

extern "C" void enter_debug_mark(const char* file, const Uint32 line,
	const char* name)
{
	assert(name != 0);
	assert(strlen(name) > 0);

	el::enter_debug_mark(name, file, line);
}

extern "C" void leave_debug_mark(const char* file, const Uint32 line,
	const char* name)
{
	assert(name != 0);
	assert(strlen(name) > 0);

	el::leave_debug_mark(name, file, line);
}

extern "C" void init_thread_log(const char* name)
{
	assert(name != 0);
	assert(strlen(name) > 0);

	el::init_thread_log(name);
}

