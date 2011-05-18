/****************************************************************************
 *            logging.hpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#ifndef	UUID_043a52e6_d0d2_4327_9877_06d299f16726
#define	UUID_043a52e6_d0d2_4327_9877_06d299f16726

#ifndef	__cplusplus
#error	"Including C++ header in C translation unit!"
#endif	/* __cplusplus */

#include "../platform.h"
#include <string>

/**
 * @file
 */
namespace eternal_lands
{

	enum LogLevelType
	{
		llt_error = 0,
		llt_warning = 1,
		llt_info = 2,
		llt_debug = 3,
		llt_debug_verbose = 4
	};

	void init_logging(const std::string &log_file_name);
	void exit_logging();
	LogLevelType get_log_level();
	void set_log_level(const LogLevelType log_level);
	void log_message(const LogLevelType log_level,
		const std::string &message, const std::string &file,
		const Uint32 line);
	void init_thread_log(const std::string &name);
	void enter_debug_mark(const std::string &name,
		const std::string &file, const Uint32 line);
	void leave_debug_mark(const std::string &name,
		const std::string &file, const Uint32 line);

}

#endif	/* UUID_043a52e6_d0d2_4327_9877_06d299f16726 */

