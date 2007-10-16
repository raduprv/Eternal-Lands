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

namespace eternal_lands
{

	class extended_exception: public std::exception
	{
		private:
			int number;
			std::string description;
			std::string type;
			std::string file;
			std::string function;
			unsigned int line;
			mutable std::string full_description;

		protected:

		public:
			enum exception_codes
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
			inline extended_exception(unsigned int number, const std::string &description,
				const char* type): number(number), description(description),
				type(type), file(""), function(""), line(0)
			{
			}

			/**
			 * Advanced constructor.
			 */
			extended_exception(unsigned int number, const std::string &description,
				const char* type, const char* file, const char* function,
				unsigned int line);

			/**
			 * Copy constructor.
			 */
			extended_exception(const extended_exception &ee);

			/**
			 * Assignment operator.
			 */
			void operator = (const extended_exception &ee);

			/*
			 * Destrucor, needed for compatibility with std::exception.
			 */
			virtual inline ~extended_exception() throw()
			{
			}

			/**
			 * Returns a string with the full description of this error.
			 */
			virtual const std::string& get_full_description() const;

			/**
			 * Gets the error code.
			 */
			virtual int get_number() const throw()
			{
				return number;
			}

			/**
			 * Gets the source file name.
			 */
			virtual inline const std::string &get_file() const throw()
			{
				return file;
			}

			/**
			 * Gets the source function.
			 */
			virtual inline const std::string &get_function() const throw()
			{
				return function;
			}

			/**
			 * Gets the line number.
			 */
			virtual inline unsigned int get_line() const throw()
			{
				return line;
			}

			/**
			 * Returns a string with only the 'description' field of this exception.
			 * Use get_full_descriptionto get a full description of the error including
			 * line number, error number and what function threw the exception.
			 */
			virtual inline const std::string &get_description() const throw()
			{
				return description;
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
	template <int num>
	struct exception_code_type
	{
		enum {
			number = num
		};
	};

	class exception_factory
	{
		private:

			/**
			 * Private constructor, no construction.
			 */
			inline exception_factory()
			{
			}

		public:

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_duplicate_item> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "duplicate_item",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_file_not_found> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "file_not_found",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_item_not_found> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "item_not_found",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_io_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "io_error",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_invalid_parameter> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "invalid_parameter",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_opengl_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "opengl_error",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_zip_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "zip_error",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_internal_error> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "internal_error",
					file, function, line);
			}

			static inline extended_exception create(
				exception_code_type<extended_exception::ec_not_implemented> code,
				const std::string &description, const char* file,
				const char* function, unsigned int line)
			{
				return extended_exception(code.number, description, "not_implemented",
					file, function, line);
			}
	};

#define EXTENDED_EXCEPTION(num, description)	\
	do	\
	{	\
		std::stringstream str;	\
	\
		str << description;	\
	\
		throw eternal_lands::exception_factory::create(	\
			eternal_lands::exception_code_type<num>(), str.str(),	\
			__FILE__, __FUNCTION__, __LINE__);	\
	}	\
	while (false)

}

#define	CATCH_AND_LOG_EXCEPTIONS	\
catch (std::exception &e)	\
{	\
	log_error(e.what());	\
}

#define	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(return_value)	\
catch (std::exception &e)	\
{	\
	log_error(e.what());	\
	return return_value;	\
}

#endif	// _EXTENDEDEXCEPTION_HPP_
