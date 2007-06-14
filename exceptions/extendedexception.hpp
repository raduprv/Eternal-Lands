/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 * extended exception is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * extended exception is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with extended exception. If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

/**
 * @file
 * @ingroup error
 * @brief extended exception with logging support
 */

#ifndef	_EXTENDEDEXCEPTION_HPP_
#define	_EXTENDEDEXCEPTION_HPP_

#include <exception>
#include <string>
#include <sstream>
#include <SDL.h>
#include "../errors.h"

class extended_exception: public std::exception 
{
	private:
		/**
		 * @brief The source file name.
		 *
		 * The name of the source file where the error occured.
		 */
		const char *file;

		/**
		 * @brief The function name.
		 *
		 * The name of the function in the source file where the error occured.
		 */
		const char *func;

		/**
		 * @brief The line number.
		 *
		 * The line number in the source file where the error occured.
		 */
		const unsigned int line;

		/**
		 * @brief The error.
		 *
		 * The error string.
		 */
		const std::string error;

	public:

		/**
		 * @brief Constructor.
		 *
		 * Construtor with error, file, line and function.
		 * @param file The name of the source file where the error occured.
		 * @param line The line of the source file where the error occured.
		 * @param function The name of the function in the source file where the error
		 * occured.
		 */
		extended_exception(const char *file, const char *func, unsigned int line,
			const std::string &error) throw();

		/**
		 * @brief Destructor.
		 *
		 * Default exception destructor.
		 */
		virtual ~extended_exception() throw();

		/** Returns a C-style character string describing the general cause
		 *  of the current error.  */
		virtual const char* what() const throw();

		/**
		 * @brief Logs the error.
		 *
		 * Log the error using the log_error_detailed funtion.
		 */
		void log_error() const;
};

#define EXTENDED_EXCEPTION(error)	\
	throw extended_exception(__FILE__, __FUNCTION__, __LINE__, error)

#define EXTENDED_INVALID_STRING_VALUE_EXCEPTION(type, value)	\
{	\
	std::ostringstream error_str;	\
	\
	error_str << "String '" << value;	\
	error_str << "' is not a valid value for '" << type << "'";	\
	\
	throw extended_exception(__FILE__, __FUNCTION__, __LINE__, error_str.str());	\
} while (0)

#define EXTENDED_OUT_OF_RANGE_EXCEPTION(min, max, value)	\
{	\
	std::ostringstream error_str;	\
	\
	error_str << "Value '" << value << "' is out of range [";	\
	error_str << min << ", " << max << "] for value '" << #value << "'";	\
	\
	throw extended_exception(__FILE__, __FUNCTION__, __LINE__, error_str.str());	\
} while (0)

#define RANGE_CHECK(min, max, value)	\
if ((value < min) || (value > max))	\
{	\
	std::ostringstream error_str;	\
	\
	error_str << "Value '" << value << "' is out of range [";	\
	error_str << min << ", " << max << "] for value '" << #value << "'";	\
	\
	throw extended_exception(__FILE__, __FUNCTION__, __LINE__, error_str.str());	\
}

#endif	// _EXTENDEDEXCEPTION_HPP_
