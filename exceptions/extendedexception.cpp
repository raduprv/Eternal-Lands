/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 */

#include "extendedexception.hpp"
#include <sstream>

namespace
{

const char* type_string(Uint32 number)
{
	switch (number)
	{
		case eternal_lands::ExtendedException::ec_duplicate_item:    return "duplicate_item";
		case eternal_lands::ExtendedException::ec_file_not_found:    return "file_not_found";
		case eternal_lands::ExtendedException::ec_item_not_found:    return "item_not_found";
		case eternal_lands::ExtendedException::ec_io_error:          return "io_error";
		case eternal_lands::ExtendedException::ec_invalid_parameter: return "invalid_parameter";
		case eternal_lands::ExtendedException::ec_opengl_error:      return "opengl_error";
		case eternal_lands::ExtendedException::ec_zip_error:         return "zip_error";
		case eternal_lands::ExtendedException::ec_internal_error:    return "internal_error";
		case eternal_lands::ExtendedException::ec_not_implemented:   return "not_implemented";
		default:                                                     return "";
	}
}

} // namespace

namespace eternal_lands
{

	ExtendedException::ExtendedException(const ExtendedException &ee): m_number(ee.m_number),
		m_description(ee.m_description), m_type(ee.m_type), m_file(ee.m_file),
		m_function(ee.m_function), m_line(ee.m_line), m_full_description() {}

	ExtendedException::ExtendedException(const Uint32 number, const std::string &description,
		const char* type):
		m_number(number), m_description(description), m_type(type ? type : type_string(number)),
		m_file(""), m_function(""), m_line(0), m_full_description() {}

	ExtendedException::ExtendedException(const Uint32 number, const std::string &description,
		const char* type, const char* file, const char* function, const Uint32 line):
		m_number(number), m_description(description), m_type(type ? type : type_string(number)),
		m_file(file), m_function(function), m_line(line), m_full_description() {}

	const std::string &ExtendedException::get_full_description() const
	{
		if (m_full_description.empty())
		{
			std::stringstream desc;

			desc << "EXTENDED EXCEPTION(" << m_number << ":" << m_type << "): " << m_description;
			if (m_line > 0)
			{
				desc << " in " << m_function << " at " << m_file << " (line "
					<< m_line << ")";
			}

			m_full_description = desc.str();
		}

		return m_full_description;
	}
}

