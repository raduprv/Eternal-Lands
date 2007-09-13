/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
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
	protected:

#if defined DEBUG && !defined _MSC_VER
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
		const char *function;

		/**
		 * @brief The line number.
		 *
		 * The line number in the source file where the error occured.
		 */
		const unsigned int line;
#endif	// DEBUG && !__MSC_VER

		/**
		 * @brief The error.
		 *
		 * The error string.
		 */
		std::string error_str;

	public:

#if defined DEBUG && !defined _MSC_VER
		/**
		 * @brief Constructor.
		 *
		 * Construtor with error, file, line, function and format.
		 * @param file The name of the source file where the error occured.
		 * @param function The name of the function in the source file where the error
		 * @param line The line of the source file where the error occured.
		 * occured.
		 * @param error The error string.
		 */
		extended_exception(const char *file, const char *function, unsigned int line,
			const char *error, ...) throw();
#else	// DEBUG && !__MSC_VER
		/**
		 * @brief Constructor.
		 *
		 * Construtor with error and format.
		 * @param error The error string.
		 */
		extended_exception(const char *error, ...) throw();
#endif	// DEBUG && !__MSC_VER

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
		 * Logs the error.
		 */
		void log() const;
};

#if defined DEBUG && !defined _MSC_VER
#define EXCEPTION(error, args ...)	\
	throw extended_exception(__FILE__, __FUNCTION__, __LINE__, error, ## args)
#else	// DEBUG && !__MSC_VER
#define EXCEPTION	\
	throw extended_exception
#endif	// DEBUG && !__MSC_VER

#define INVALID_STRING_VALUE_EXCEPTION(type, value) EXCEPTION("String '%s' is not a valid value for '%s'", value, type)
#define INT_OUT_OF_RANGE_EXCEPTION(min, max, value) EXCEPTION("Value '%s' is out of range [%d, %d] for value '%s'", value, min, max, #value)
#define UINT_OUT_OF_RANGE_EXCEPTION(min, max, value) EXCEPTION("Value '%s' is out of range [%u, %u] for value '%s'", value, min, max, #value)
#define FLOAT_OUT_OF_RANGE_EXCEPTION(min, max, value) EXCEPTION("Value '%s' is out of range [%f, %f] for value '%s'", value, min, max, #value)

#define INT_RANGE_CHECK(min, max, value)	\
{	\
	if ((value < min) || (value > max))	\
	{	\
		INT_OUT_OF_RANGE_EXCEPTION(min, max, value);	\
	}	\
} while (0)

#define UINT_RANGE_CHECK(min, max, value)	\
{	\
	if ((value < min) || (value > max))	\
	{	\
		UINT_OUT_OF_RANGE_EXCEPTION(min, max, value);	\
	}	\
} while (0)

#define FLOAT_RANGE_CHECK(min, max, value)	\
{	\
	if ((value < min) || (value > max))	\
	{	\
		FLOAT_OUT_OF_RANGE_EXCEPTION(min, max, value);	\
	}	\
} while (0)

#define FILE_NOT_FOUND_EXCEPTION(file_name) EXCEPTION("File '%s' not found", file_name)

#define	CATCH_AND_LOG_EXCEPTIONS	\
catch (extended_exception &e)	\
{	\
	e.log();	\
}	\
catch (std::exception &e)	\
{	\
	log_error(e.what());	\
}

#define	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(return_value)	\
catch (extended_exception &e)	\
{	\
	e.log();	\
	return return_value;	\
}	\
catch (std::exception &e)	\
{	\
	log_error(e.what());	\
	return return_value;	\
}

#endif	// _EXTENDEDEXCEPTION_HPP_
