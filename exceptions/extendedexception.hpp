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

		protected:

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
			inline ExtendedException(): m_number(0), m_description(""), m_type(""),
				m_file(""), m_function(""), m_line(0)
			{
			}

			/**
			 * Default constructor.
			 */
			inline ExtendedException(const Uint32 number, const std::string &description,
				const char* type): m_number(number), m_description(description),
				m_type(type), m_file(""), m_function(""), m_line(0)
			{
			}

			/**
			 * Advanced constructor.
			 */
			ExtendedException(const Uint32 number, const std::string &description,
				const char* type, const char* file, const char* function,
				const Uint32 line);

			/**
			 * Copy constructor.
			 */
			ExtendedException(const ExtendedException &ee);

			/**
			 * Assignment operator.
			 */
			void operator= (const ExtendedException &ee);

			/*
			 * Destrucor, needed for compatibility with std::exception.
			 */
			virtual inline ~ExtendedException() throw()
			{
			}

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

	/**
	 * Template struct which creates a distinct type for each exception code.
	 */
	template <Uint32 num>
	struct ExceptionCodeType
	{
		enum
		{
			number = num
		};
	};

	class ExceptionFactory
	{
		private:

			/**
			 * Private constructor, no construction.
			 */
			inline ExceptionFactory()
			{
			}

		public:

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_duplicate_item> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"duplicate_item", file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_file_not_found> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"file_not_found", file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_item_not_found> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"item_not_found", file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_io_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description, "io_error",
					file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_invalid_parameter> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"invalid_parameter", file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_opengl_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description, "opengl_error",
					file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_zip_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description, "zip_error",
					file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_internal_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"internal_error", file, function, line);
			}

			static inline ExtendedException create(
				ExceptionCodeType<ExtendedException::ec_not_implemented> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return ExtendedException(code.number, description,
					"not_implemented", file, function, line);
			}
	};

#define EXTENDED_EXCEPTION(num, description)	\
	do	\
	{	\
		std::stringstream str;	\
	\
		str << description;	\
	\
		throw eternal_lands::ExceptionFactory::create(	\
			eternal_lands::ExceptionCodeType<eternal_lands::num>(), str.str(),	\
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
			throw eternal_lands::ExceptionFactory::create(	\
				eternal_lands::ExceptionCodeType<eternal_lands::ExtendedException::ec_opengl_error>(),	\
				str.str(), __FILE__, __FUNCTION__, __LINE__);	\
		}	\
	}	\
	while (false)

}

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
