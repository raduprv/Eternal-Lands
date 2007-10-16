/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 */

#include "extendedexception.hpp"
#include <sstream>

namespace eternal_lands
{

	extended_exception::extended_exception(const extended_exception &ee): number(ee.number),
		description(ee.description), type(ee.type), file(ee.file), function(ee.function),
		line(ee.line)
	{
	}

	extended_exception::extended_exception(unsigned int number, const std::string &description,
		const char* type, const char* file, const char* function, unsigned int line):
		number(number), description(description), type(type), file(file),
		function(function), line(line)
	{
	}

	void extended_exception::operator = (const extended_exception &ee)
	{
		number = ee.number;
		description = ee.description;
		type = ee.type;
		file = ee.file;
		function = ee.function;
		line = ee.line;
	}

	const std::string &extended_exception::get_full_description() const
	{
		if (full_description.empty())
		{
			std::stringstream desc;

			desc <<  "EXTENDED EXCEPTION(" << number << ":" << type << "): "
				<< description;

			if (line > 0)
			{
				desc << " in " << function << " at " << file << " (line "
					<< line << ")";
			}

			full_description = desc.str();
		}

		return full_description;
	}
}

