#ifdef NEW_FILE_IO

#include "elfilewrapper.h"
#include "elfile.hpp"

extern "C" void add_zip_archive(const char* file_name, int replace)
{
	try
	{
		el_file::add_zip_archive(file_name, replace == 1);
	}
	CATCH_AND_LOG_EXCEPTIONS
}

extern "C" el_file* el_open(const char* file_name)
{
	el_file* file;

	try
	{
		file = new el_file(file_name, true);

		return file;
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
}

extern "C" el_file* el_open_no_decompress(const char* file_name)
{
	el_file* file;

	try
	{
		file = new el_file(file_name, false);

		return file;
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
}

extern "C" int el_read(el_file* file, int size, void* buffer)
{
	if (file == 0)
	{
		return -1;
	}
	try
	{
		return file->read(size, buffer);
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
}

extern "C" int el_seek(el_file* file, int offset, int seek_type)
{
	if (file == 0)
	{
		return -1;
	}
	try
	{
		return file->seek(offset, seek_type);
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
}

extern "C" int el_tell(el_file* file)
{
	if (file == 0)
	{
		return -1;
	}
	try
	{
		return file->tell();
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
}

extern "C" int el_get_size(el_file* file)
{
	if (file == 0)
	{
		return -1;
	}
	try
	{
		return file->get_size();
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(-1);
}

extern "C" void el_close(el_file* file)
{
	if (file == 0)
	{
		return;
	}
	delete file;
	file = 0;
}

extern "C" void* el_get_pointer(el_file* file)
{
	if (file == 0)
	{
		return 0;
	}
	try
	{
		return file->get_pointer();
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
}

extern "C" int el_file_exists(const char* file_name)
{
	try
	{
		return el_file::file_exists(file_name);
	}
	CATCH_AND_LOG_EXCEPTIONS_WITH_RETURN(0);
}

#endif //NEW_FILE_IO
