/****************************************************************************
 *            logging.cpp
 *
 * Author: 2011  Daniel Jungmann <el.3d.source@googlemail.com>
 * Copyright: See COPYING file that comes with this distribution
 ****************************************************************************/

#include "logging.hpp"
#include <bitset>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <vector>
#include <map>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_thread.h>
#include "../elc_private.h"

namespace eternal_lands
{

	namespace
	{

		class DebugMark
		{
			public:
				std::string m_name;
				Uint64 m_log_file_pos;

		};

		class ThreadData
		{
			public:
				std::vector<DebugMark> m_debug_marks;
				std::string m_name;
				std::string m_last_message;
				Uint32 m_last_message_count;
				Uint32 m_message_level;
				int m_log_file;

		};

		typedef std::map<Uint32, ThreadData> ThreadDatas;

		std::string log_dir;
		SDL_mutex* log_mutex;
		ThreadDatas thread_datas;
		volatile LogLevelType log_levels = llt_info;

		std::string get_str(const LogLevelType log_level)
		{
			switch (log_level)
			{
				case llt_error:
					return "Error";
				case llt_warning:
					return "Warning";
				case llt_info:
					return "Info";
				case llt_debug:
				case llt_debug_verbose:
					return "Debug";
				default:
					return "Unkown";
			}
		}

		void log_message(const std::string &type,
			const std::string &message, const std::string &file,
			const Uint32 line, ThreadData &thread)
		{
			char buffer[128];
			std::stringstream str, log_stream;
			std::time_t raw_time;

			if (thread.m_log_file == -1)
			{
				return;
			}

			std::time(&raw_time);
			memset(buffer, 0, sizeof(buffer));
			std::strftime(buffer, sizeof(buffer), "%X",
				std::localtime(&raw_time));

			str << ", " << file << ":" << line << "] " << type;
			str << ": " << message;

			if (str.str() == thread.m_last_message)
			{
				thread.m_last_message_count++;
				return;
			}

			if (thread.m_last_message_count > 0)
			{
				log_stream << "[" << buffer;

				if (log_levels >= llt_debug_verbose)
				{
					log_stream << ", " << __FILE__ << ":";
					log_stream << __LINE__;
				}

				log_stream << "] ";
				log_stream << "Last message repeated ";
				log_stream << thread.m_last_message_count;
				log_stream << " time";

				if (thread.m_last_message_count > 1)
				{
					log_stream << "s";
				}

				log_stream << "\n";
			}

			log_stream << "[" << buffer << str.str();

			if (message.rbegin() != message.rend())
			{
				if (*message.rbegin() != '\n')
				{
					log_stream << "\n";
				}
			}
			else
			{
				log_stream << "\n";
			}

			thread.m_last_message = str.str();
			thread.m_last_message_count = 0;

			ssize_t ret = write(thread.m_log_file,
				log_stream.str().c_str(), log_stream.str().length());

			if (ret != static_cast<ssize_t>(log_stream.str().length()))
				std::cerr << "Failed to write the log file: "
					<< log_stream.str(); // newline included
		}

		void do_log_message(const LogLevelType log_level,
			const std::string &message, const std::string &file,
			const Uint32 line, ThreadData &thread_data)
		{
			Uint32 level;

			log_message(get_str(log_level), message, file,
				line, thread_data);

			if (log_level < llt_debug)
			{
				level = thread_data.m_debug_marks.size();
				thread_data.m_message_level = std::max(level,
					thread_data.m_message_level);
			}
		}

		void do_enter_debug_mark(const std::string &name,
			const std::string &file, const Uint32 line,
			ThreadData &thread_data)
		{
			DebugMark debug_mark;
			std::stringstream str;

			debug_mark.m_name = name;
			debug_mark.m_log_file_pos = lseek(
				thread_data.m_log_file, 0, SEEK_CUR);

			thread_data.m_debug_marks.push_back(debug_mark);

			str << "Enter debug mark '" << name << "'";

			do_log_message(llt_debug, str.str(), file, line,
				thread_data);
		}

		void do_leave_debug_mark(const std::string &name,
			const std::string &file, const Uint32 line,
			ThreadData &thread_data)
		{
			std::stringstream str;
			Uint64 size, pos;
			Uint32 level;

			if (thread_data.m_debug_marks.rbegin() ==
				thread_data.m_debug_marks.rend())
			{
				if (log_levels >= llt_debug)
				{
					str << "Can't leave debug mark '";
					str << name << "', because no debug ";
					str << "mark " << "entered.";

					do_log_message(llt_error, str.str(),
						file, line, thread_data);
				}

				return;
			}

			if (thread_data.m_debug_marks.rbegin()->m_name != name)
			{
				str << "Can't leave debug mark '";
				str << name << "', because current debug mark";
				str << " is '";
				str << thread_data.m_debug_marks.rbegin(
					)->m_name << "'.";

				do_log_message(llt_error, str.str(), file,
					line, thread_data);

				thread_data.m_debug_marks.pop_back();

				return;
			}

			if (log_levels < llt_debug_verbose)
			{
				size = thread_data.m_debug_marks.rbegin(
					)->m_log_file_pos;

				if (thread_data.m_message_level <
					thread_data.m_debug_marks.size())
				{
					pos = lseek(thread_data.m_log_file, 0,
						SEEK_CUR);
					lseek(thread_data.m_log_file, size,
						SEEK_SET);

					if ((pos + 1024) < size)
					{
						if (ftruncate(
							thread_data.m_log_file,
							size) < 0)
						std::cerr << "Failed to truncate log file: "
							<< strerror(errno) << std::endl;
					}
				}
			}
			else
			{
				str << "Leave debug mark '" << name << "'";

				do_log_message(llt_debug_verbose, str.str(),
					file, line, thread_data);
			}

			thread_data.m_debug_marks.pop_back();

			level = thread_data.m_debug_marks.size();
			thread_data.m_message_level = std::min(level,
				thread_data.m_message_level);
		}

		std::string get_local_time_string()
		{
			char buffer[512];
			std::time_t raw_time;

			std::time(&raw_time);

			memset(buffer, 0, sizeof(buffer));
			std::strftime(buffer, sizeof(buffer), "%c %Z",
				std::localtime(&raw_time));

			return buffer;
		}

		void init(const std::string &name)
		{
			ThreadDatas::iterator found;
			std::stringstream file_name;
			std::stringstream str;
			Uint32 id;

			id = SDL_ThreadID();

			str << name << " (" << std::hex << id << ")";

			found = thread_datas.find(id);

			if (found != thread_datas.end())
			{
				found->second.m_name = str.str();

				return;
			}

			file_name << log_dir << name << "_" << std::hex << id;
			file_name << ".log";

			thread_datas[id].m_name = str.str();
			thread_datas[id].m_last_message_count = 0;
			thread_datas[id].m_message_level = 0;
			thread_datas[id].m_log_file = open(file_name.str(
				).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

			log_message("Log started at", get_local_time_string(),
				__FILE__, __LINE__, thread_datas[id]);

			log_message("version", FILE_VERSION, __FILE__,
				__LINE__, thread_datas[id]);
		}

		void clear_dir(const std::string &dir)
		{
			struct dirent *dp;
			DIR *dirp;
			std::string file_name;

			dirp = opendir(dir.c_str());

			if (dirp == 0)
			{
				return;
			}

			dp = readdir(dirp);

			while (dp != 0)
			{
				file_name = dir;
				file_name += "/";
				file_name += dp->d_name;
				std::remove(file_name.c_str());
				dp = readdir(dirp);
			}

			closedir(dirp);
		}

	}

	void init_logging(const std::string &dir)
	{
		std::string str;

		log_mutex = SDL_CreateMutex();

		log_dir = dir + "/";

		clear_dir(dir);
#ifdef	WINDOWS
		mkdir(dir.c_str());
#else	/* WINDOWS */
		mkdir(dir.c_str(), S_IRWXU | S_IRWXG);
#endif	/* WINDOWS */

		init_thread_log("main");
	}

	void exit_logging()
	{
		ThreadDatas::iterator it, end;
		Uint64 pos;

		end = thread_datas.end();

		for (it = thread_datas.begin(); it != end; ++it)
		{
			pos = lseek(it->second.m_log_file, 0, SEEK_CUR);

			if (ftruncate(it->second.m_log_file, pos) < 0)
				std::cerr << "Failed to truncate log file: "
					<< strerror(errno) << std::endl;

			close(it->second.m_log_file);
		}

		SDL_DestroyMutex(log_mutex);
	}

	LogLevelType get_log_level()
	{
		return log_levels;
	}

	void set_log_level(const LogLevelType log_level)
	{
		log_levels = log_level;
	}

	void init_thread_log(const std::string &name)
	{
		SDL_LockMutex(log_mutex);

		init(name);

		SDL_UnlockMutex(log_mutex);
	}

	void log_message(const LogLevelType log_level,
		const std::string &message, const std::string &file,
		const Uint32 line)
	{
		ThreadDatas::iterator found;

		if (log_levels < log_level)
		{
			return;
		}

		SDL_LockMutex(log_mutex);

		found = thread_datas.find(SDL_ThreadID());

		if (found != thread_datas.end())
		{
			do_log_message(log_level, message, file, line,
				found->second);
		}

		SDL_UnlockMutex(log_mutex);
	}

	void enter_debug_mark(const std::string &name,
		const std::string &file, const Uint32 line)
	{
		ThreadDatas::iterator found;

		if (log_levels < llt_debug)
		{
			return;
		}

		SDL_LockMutex(log_mutex);

		found = thread_datas.find(SDL_ThreadID());

		if (found != thread_datas.end())
		{
			do_enter_debug_mark(name, file, line, found->second);
		}

		SDL_UnlockMutex(log_mutex);
	}

	void leave_debug_mark(const std::string &name,
		const std::string &file, const Uint32 line)
	{
		ThreadDatas::iterator found;

		if (log_levels < llt_debug)
		{
			return;
		}

		SDL_LockMutex(log_mutex);

		found = thread_datas.find(SDL_ThreadID());

		if (found != thread_datas.end())
		{
			do_leave_debug_mark(name, file, line, found->second);
		}

		SDL_UnlockMutex(log_mutex);
	}

}

