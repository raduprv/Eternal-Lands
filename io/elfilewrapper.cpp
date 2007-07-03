#ifdef NEW_FILE_IO

#include "elfilewrapper.h"
#include "elfile.hpp"

extern "C" void add_zip_archive(const char* file_name)
{
	try
	{
		el_file::add_zip_archive(file_name);
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
		file = new el_file(file_name, true);

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
		file = new el_file(file_name, false);

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

extern "C" int el_read(el_file* file, int size, void* buffer)
{
	if(file == NULL){
		return -1;
	}
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

extern "C" int el_seek(el_file* file, int offset, int seek_type)
{
	if(file == NULL){
		return -1;
	}
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

extern "C" int el_tell(el_file* file)
{
	if(file == NULL){
		return -1;
	}
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

extern "C" int el_get_size(el_file* file)
{
	if(file == NULL){
		return -1;
	}
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
	if(file == NULL){
		return;
	}
	delete file;
	file = 0;
}

extern "C" void* el_get_pointer(el_file* file)
{
	if(file == NULL){
		return NULL;
	}
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

extern "C" int el_file_exists(const char* file_name)
{
	try
	{
		return el_file::file_exists(file_name);
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
