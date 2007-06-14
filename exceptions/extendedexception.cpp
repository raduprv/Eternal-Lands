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

#include "extendedexception.hpp"

extended_exception::extended_exception(const char *file, const char *func, unsigned int line,
	const std::string &error) throw(): file(file), func(func), line(line), error(error)
{
}

extended_exception::~extended_exception() throw()
{
}

const char* extended_exception::what() const throw()
{
	return error.c_str();
}

void extended_exception::log_error() const
{
	log_error_detailed(error.c_str(), file, func, line);
}

