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
#include <cassert>
#include <sstream>
#include "../platform.h"
#include "../errors.h"

namespace eternal_lands
{

	class ExtendedException: public std::exception
	{
		private:
			Uint32 m_number;
			std::string m_description;
			std::string m_type;
			std::string m_file;
			std::string m_function;
			Uint32 m_line;
			mutable std::string m_full_description;

		public:
			enum ExceptionCodes
			{
				ec_duplicate_item,
				ec_file_not_found,
				ec_item_not_found,
				ec_io_error,
				ec_invalid_parameter,
				ec_opengl_error,
				ec_zip_error,
				ec_internal_error,
				ec_not_implemented
			};

			/**
			 * Default constructor.
			 */
			inline ExtendedException(): m_number(0), m_description(""), m_type(""), m_file(""),
				m_function(""), m_line(0), m_full_description() {}

			/**
			 * Default constructor.
			 */
			ExtendedException(const Uint32 number, const std::string &description, const char* type=nullptr);

			/**
			 * Advanced constructor.
			 */
			ExtendedException(const Uint32 number, const std::string &description, const char* type,
				const char* file, const char* function, const Uint32 line);

			/**
			 * Copy constructor.
			 */
			ExtendedException(const ExtendedException &ee);

			/**
			 * Assignment operator.
			 */
			ExtendedException& operator=(const ExtendedException &ee) = default;

			/*
			 * Destructor, needed for compatibility with std::exception.
			 */
			virtual inline ~ExtendedException() throw() {}

			/**
			 * Returns a string with the full description of this error.
			 */
			virtual const std::string& get_full_description() const;

			/**
			 * Gets the error code.
			 */
			virtual Uint32 get_number() const throw()
			{
				return m_number;
			}

			/**
			 * Gets the source file name.
			 */
			virtual inline const std::string &get_file() const throw()
			{
				return m_file;
			}

			/**
			 * Gets the source function.
			 */
			virtual inline const std::string &get_function() const throw()
			{
				return m_function;
			}

			/**
			 * Gets the line number.
			 */
			virtual inline Uint32 get_line() const throw()
			{
				return m_line;
			}

			/**
			 * Returns a string with only the 'description' field of this exception.
			 * Use get_full_descriptionto get a full description of the error including
			 * line number, error number and what function threw the exception.
			 */
			virtual inline const std::string &get_description() const throw()
			{
				return m_description;
			}

			/**
			 * Override std::exception::what
			 */
			inline const char* what() const throw()
			{
				return get_full_description().c_str();
			}

	};

#define EXTENDED_EXCEPTION(num, description)	\
	do	\
	{	\
		std::stringstream str;	\
	\
		str << description;	\
	\
		throw eternal_lands::ExtendedException(eternal_lands::num, str.str(), nullptr, \
			__FILE__, __FUNCTION__, __LINE__);	\
	}	\
	while (false)

#define CHECK_GL_EXCEPTION()	\
	do	\
	{	\
		GLint gl_error;	\
	\
		gl_error = glGetError();	\
	\
		if (gl_error != GL_NO_ERROR)	\
		{	\
			std::stringstream str;	\
	\
			str << gluErrorString(gl_error);	\
	\
			throw eternal_lands::ExceptionFactory::(eternal_lands::ExtendedException::ec_opengl_error, \
				str.str(), nullptr, __FILE__, __FUNCTION__, __LINE__);	\
		}	\
	}	\
	while (false)

} // namespace eternal_lands

#define	CATCH_AND_LOG_EXCEPTIONS	\
catch (std::exception &e)	\
{	\
	LOG_ERROR("%s(): %s", __func__, e.what());	\
}

#define	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(return_value)	\
catch (std::exception &e)	\
{	\
	LOG_ERROR("%s(): %s", __func__, e.what());	\
	return return_value;	\
}

#endif	// _EXTENDEDEXCEPTION_HPP_
