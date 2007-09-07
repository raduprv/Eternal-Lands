/*
 * extended exception
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 */

#include "extendedexception.hpp"

#if defined DEBUG && !defined _MSC_VER
extended_exception::extended_exception(const char *file, const char *function, unsigned int line,
	const char *error, ...) throw(): file(file), function(function), line(line)
#else	// DEBUG && !__MSC_VER
extended_exception::extended_exception(const char *error, ...) throw()
#endif	// DEBUG && !__MSC_VER
{
	char buffer[4096];
	int len;

	va_list format;

	va_start(format, error);
	len = vsnprintf(buffer, sizeof(buffer), error, format);
	va_end(format);

	error_str = std::string(buffer, std::min(len, static_cast<int>(sizeof(buffer))));
}

extended_exception::~extended_exception() throw()
{
}

const char* extended_exception::what() const throw()
{
	return error_str.c_str();
}

void extended_exception::log() const
{
#if defined DEBUG && !defined _MSC_VER
	log_error_detailed(error_str.c_str(), file, func, line);
#else	// DEBUG && !__MSC_VER
	log_error(error_str.c_str());
#endif	// DEBUG && !__MSC_VER
}

