#ifdef NEW_FILE_IO

#include "elfile.hpp"

std::string el_file::el_find_file(const std::string& file_name) {
	char * cfgpath = get_path_config();
	std::string str = cfgpath + file_name;
	free(cfgpath);
	struct stat fstat;

	if (stat(str.c_str(), &fstat) != 0){
		str.append(".gz");
		if(stat(str.c_str(), &fstat) != 0){
			str = file_name;
			if(stat(str.c_str(), &fstat) != 0){
				str.append(".gz");
				if(stat(str.c_str(), &fstat) != 0){
					return "";	//No luck anywhere!
				}
			}
		}
	}
	return str;
}



bool el_file::open_zip(const std::string& file_name, bool uncompress, zip_file_system& zfile_system)
{
	return zfile_system.open_file(file_name, memory, uncompress) > 0;
}

void el_file::open_gzip(const std::string& file_name)
{
	std::string str;
	gzFile file;
	int_fast32_t read, size;
	char * cfgpath = get_path_config();
	str = std::string(cfgpath) + "updates/" + file_name + ".gz";
	free(cfgpath);

	file = gzopen(str.c_str(), "rb");
	try
	{
		if (file == 0)
		{
			str = str.substr(0, str.size() - 3);	//trim the extension we added earlier
			file = gzopen(str.c_str(), "rb");
		}
		if (file == 0)
		{
			str = file_name + ".gz";
			file = gzopen(file_name.c_str(), "rb");
		}
		if (file == 0)
		{
			file = gzopen(file_name.c_str(), "rb");
		}
		if (file == 0)
		{
			EXTENDED_FILE_NOT_FOUND_EXCEPTION(file_name);
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
	char * cfgpath = get_path_config();
	str = std::string(cfgpath) + "updates/" + file_name;
	free(cfgpath);

	file.open(str.c_str(), std::ios::binary);

	if(!file.is_open()){
		file.open(file_name.c_str(), std::ios::binary);
	}

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

	if (zfile_system.file_exists(file_name)){
		return true;
	}
	if(!el_find_file(file_name).empty()){
		return true;
	}
	return false;
}

#endif //NEW_FILE_IO
