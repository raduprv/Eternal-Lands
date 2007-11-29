/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 */

#include "extendedexception.hpp"
#include <sstream>

namespace eternal_lands
{

	ExtendedException::ExtendedException(const ExtendedException &ee): m_number(ee.m_number),
		m_description(ee.m_description), m_type(ee.m_type), m_file(ee.m_file),
		m_function(ee.m_function), m_line(ee.m_line)
	{
	}

	ExtendedException::ExtendedException(const Uint32 number, const std::string &description,
		const char* type, const char* file, const char* function, const Uint32 line):
		m_number(number), m_description(description), m_type(type), m_file(file),
		m_function(function), m_line(line)
	{
	}

	void ExtendedException::operator= (const ExtendedException &ee)
	{
		m_number = ee.m_number;
		m_description = ee.m_description;
		m_type = ee.m_type;
		m_file = ee.m_file;
		m_function = ee.m_function;
		m_line = ee.m_line;
	}

	const std::string &ExtendedException::get_full_description() const
	{
		if (m_full_description.empty())
		{
			std::stringstream desc;

			desc <<  "EXTENDED EXCEPTION(" << m_number << ":" << m_type << "): "
				<< m_description;

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

