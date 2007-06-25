#ifdef NEW_FILE_IO

#include "zipfilesystem.hpp"
#include "elfilewrapper.h"
#include "elfile.hpp"

zip_file_system zfile_system;

extern "C" void add_zip_archive(const char* file_name)
{
	try
	{
		zfile_system.add_zip_archive(file_name);
	}
	catch (extended_exception &e)
	{
		e.log_error();
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
	}
}

extern "C" el_file* el_open(const char* file_name)
{
	el_file* file;

	try
	{
		file = new el_file(file_name, true, zfile_system);

		return file;
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return NULL;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return NULL;
	}
}

extern "C" el_file* el_open_no_decompress(const char* file_name)
{
	el_file* file;

	try
	{
		file = new el_file(file_name, false, zfile_system);

		return file;
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return NULL;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return NULL;
	}
}

extern "C" int_fast32_t el_read(el_file* file, int_fast32_t size, void* buffer)
{
	try
	{
		return file->read(size, buffer);
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return -1;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return -1;
	}
}

extern "C" int_fast32_t el_seek(el_file* file, int_fast32_t offset, int_fast32_t seek_type)
{
	try
	{
		return file->seek(offset, seek_type);
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return -1;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return -1;
	}
}

extern "C" int_fast32_t el_tell(el_file* file)
{
	try
	{
		return file->tell();
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return -1;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return -1;
	}
}

extern "C" int_fast32_t el_get_size(el_file* file)
{
	try
	{
		return file->get_size();
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return -1;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return -1;
	}
}

extern "C" void el_close(el_file* file)
{
	delete file;
	file = 0;
}

extern "C" void* el_get_pointer(el_file* file)
{
	try
	{
		return file->get_pointer();
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return 0;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return 0;
	}
}

extern "C" int_fast32_t el_file_exists(const char* file_name)
{
	try
	{
		return el_file::file_exists(file_name, zfile_system);
	}
	catch (extended_exception &e)
	{
		e.log_error();
		return 0;
	}
	catch (std::exception &e)
	{
		LOG_ERROR(e.what());
		return 0;
	}
}

#endif //NEW_FILE_IO
