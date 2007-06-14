#include "elfile.hpp"

bool el_file::open_zip(const std::string& file_name, bool uncompress, zip_file_system& zfile_system)
{
	return zfile_system.open_file(file_name, memory, uncompress) > 0;
}

void el_file::open_gzip(const std::string& file_name)
{
	std::string str;
	gzFile file;
	int_fast32_t read, size;

	str = file_name;
	str.append(".gz");
	file = gzopen(str.c_str(), "rb");
	try
	{
		if (file == 0)
		{
			file = gzopen(file_name.c_str(), "rb");
		}
		if (file == 0)
		{
			EXTENDED_EXCEPTION("File not found!");
		}

		size = 0;
		do
		{
			memory.resize(size + max_mem_block_buffer_size);
			read = gzread(file, memory.get_memory(size), max_mem_block_buffer_size);
			size += read;
		}
		while (read == max_mem_block_buffer_size);
	
		memory.resize(size);

		gzclose(file);
	}
	catch (...)
	{
		gzclose(file);
		throw;
	}
}

void el_file::open(const std::string& file_name)
{
	std::string str;
	std::ifstream file;
	int_fast32_t read, size;

	file.open(file_name.c_str(), std::ios::binary);

	size = 0;
	do
	{
		memory.resize(size + max_mem_block_buffer_size);
		read = gzread(file, memory.get_memory(), max_mem_block_buffer_size);
		size += read;
	}
	while (read == max_mem_block_buffer_size);
	
	memory.resize(size);
}

el_file::el_file(const std::string& file_name, bool uncompress, zip_file_system& zfile_system)
{
	if (!open_zip(file_name, uncompress, zfile_system))
	{
		if (uncompress)
		{
			open_gzip(file_name);
		}
		else
		{
			open(file_name);
		}
	}
	position = 0;
}

bool el_file::file_exists(const std::string& file_name, const zip_file_system& zfile_system)
{
	std::string str;
	struct stat fstat;

	if (!zfile_system.file_exists(file_name))
	{
		if (stat(file_name.c_str(), &fstat) != 0)
		{
			str = file_name;

			str.append(".gz");

			return stat(str.c_str(), &fstat) == 0;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

