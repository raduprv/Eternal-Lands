#ifdef NEW_FILE_IO

#include "elfile.hpp"

zip_file_system el_file::default_zip_file_system;
std::vector<std::string> el_file::path_list;

bool el_file::file_exist(const std::string& file_name, file_type type)
{
	struct stat fstat;

	switch (type)
	{
		case ft_gzip:
			return stat((file_name + std::string(".gz")).c_str(), &fstat) == 0;
		case ft_zip:
			return default_zip_file_system.file_exist(file_name);
		case ft_uncompressed:
			return stat(file_name.c_str(), &fstat) == 0;
	}
	/**
	 * We should be never here. If so, it's a programming error, because we forgot to add all types to the switch!
	 */
	EXCEPTION("Internal error. This could only happen if we forgot to add all types to the switch!");
}

bool el_file::file_exist_in_dir(const std::string& file_name)
{
	int i;
	file_type type;

	for (i = ft_gzip; i <= ft_uncompressed; i++)
	{
		type = static_cast<file_type>(i);

		if (file_exist(file_name, type))
		{
			return true;
		}
	}
	return false;
}

void el_file::open(const std::string& file_name, bool uncompress, file_type type)
{
	switch (type)
	{
		case ft_gzip:
			open(file_name + std::string(".gz"), uncompress);
			return;
		case ft_zip:
			open_zip(file_name, uncompress, default_zip_file_system);
			return;
		case ft_uncompressed:
			open(file_name, uncompress);
			return;
	}
	/**
	 * We should be never here. If so, it's a programming error, because we forgot to add all types to the switch!
	 */
	EXCEPTION("Internal error. This could only happen if we forgot to add all types to the switch!");
}

bool el_file::open_if_exist(const std::string& file_name, bool uncompress)
{
	int i;
	file_type type;

	for (i = ft_gzip; i <= ft_uncompressed; i++)
	{
		type = static_cast<file_type>(i);

		if (file_exist(file_name, type))
		{
			open(file_name, uncompress, type);
			return true;
		}
	}
	return false;
}

void el_file::open_gzip(const std::string& file_name)
{
	gzFile file;
	int read, size;

	file = gzopen(file_name.c_str(), "rb");
	try
	{
		if (file == 0)
		{
			EXCEPTION("Can't open file '%s'.", file_name.c_str());
		}

#ifdef	EXTRA_DEBUG
		LOG_EXTRA_INFO("File '%s' opend.", file_name.c_str());
#endif	// EXTRA_DEBUG

		size = 0;
		do
		{
			memory->resize(size + max_mem_block_buffer_size);
			read = gzread(file, memory->get_memory(size), max_mem_block_buffer_size);
			size += read;
		}
		while (read == max_mem_block_buffer_size);
	
		memory->resize(size);

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
	std::ifstream file;
	int size;

	file.open(file_name.c_str(), std::ios::binary);

	if (!file.is_open())
	{
		EXCEPTION("Can't open file '%s'.", file_name.c_str());
	}
#ifdef	EXTRA_DEBUG
	LOG_EXTRA_INFO("File '%s' opend.", file_name.c_str());
#endif	// EXTRA_DEBUG

	size = file.tellg();
	memory->resize(size);
	file.read(reinterpret_cast<char*>(memory->get_memory()), size);
}

el_file::el_file(const std::string& file_name, bool uncompress, bool check_config): memory(new memory_buffer())
{
	std::string file;
	unsigned int i;

	position = 0;

	file = remove_path(file_name);

	if (check_config)
	{
		if (open_if_exist (get_path_config () + file, uncompress))
			return;
	}

	for (i = 0; i < path_list.size(); i++)
	{
		if (open_if_exist(get_file_name_with_path(file, i), uncompress))
		{
			return;
		}
	}

	FILE_NOT_FOUND_EXCEPTION(file_name.c_str());
}

bool el_file::file_exists(const std::string& file_name, bool check_config)
{
	std::string file;
	unsigned int i;

	file = remove_path(file_name);

	if (check_config)
	{
		if (file_exist_in_dir (get_path_config () + file))
			return true;
	}

	for (i = 0; i < path_list.size(); i++)
	{
		if (file_exist_in_dir(get_file_name_with_path(file, i)))
		{
			return true;
		}
	}

	return false;
}

#endif //NEW_FILE_IO
