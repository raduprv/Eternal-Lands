#ifdef NEW_FILE_IO

#include "elfile.hpp"

namespace eternal_lands
{

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
		 * We should be never here. If so, it's a programming error,
		 * because we forgot to add all types to the switch or an invalid int
		 * was used (with a type cast)!
		 */
		EXTENDED_EXCEPTION(extended_exception::ec_internal_error, "Can't find value " <<
			static_cast<Sint32>(type) << " in the switch");
	}

	bool el_file::file_exist_in_dir(const std::string& file_name)
	{
		Sint32 i;
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
		 * We should be never here. If so, it's a programming error,
		 * because we forgot to add all types to the switch or an invalid int
		 * was used (with a type cast)!
		 */
		EXTENDED_EXCEPTION(extended_exception::ec_internal_error, "Can't find value " <<
			static_cast<Sint32>(type) << " in the switch");
	}

	bool el_file::open_if_exist(const std::string& file_name, bool uncompress)
	{
		Sint32 i;
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
		Sint32 read, size;

		file = gzopen(file_name.c_str(), "rb");
		try
		{
			if (file == 0)
			{
				EXTENDED_EXCEPTION(extended_exception::ec_io_error, "Can't open file "
					<< file_name);
			}

	#ifdef	EXTRA_DEBUG
			LOG_EXTRA_INFO("File '%s' opened.", file_name.c_str());
	#endif	// EXTRA_DEBUG

			size = 0;
			do
			{
				memory->resize(size + max_mem_block_buffer_size);
				read = gzread(file, memory->get_memory<char*>(size),
					max_mem_block_buffer_size);
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
		Sint32 size;

		file.open(file_name.c_str(), std::ios::binary);

		if (!file.is_open())
		{
			EXTENDED_EXCEPTION(extended_exception::ec_io_error, "Can't open file "
				<< file_name);
		}
#ifdef	EXTRA_DEBUG
		LOG_EXTRA_INFO("File '%s' opened.", file_name.c_str());
#endif	// EXTRA_DEBUG

		size = file.tellg();
		memory->resize(size);
		file.read(memory->get_memory<char*>(), size);
	}

	el_file::el_file(const std::string& file_name, bool uncompress,
		const std::string &extra_path): memory(new memory_buffer())
	{
		std::string file;
		Uint32 i;

		position = 0;

		file = remove_path(file_name);

		if (!extra_path.empty())
		{
			if (open_if_exist(extra_path + file, uncompress))
			{
				file_name_str = extra_path + file;
				return;
			}
		}

		for (i = 0; i < path_list.size(); i++)
		{
			if (open_if_exist(get_file_name_with_path(file, i), uncompress))
			{
				file_name_str = get_file_name_with_path(file, i);
				return;
			}
		}
		EXTENDED_EXCEPTION(extended_exception::ec_file_not_found, "Can't find file "
			<< file_name);
	}

	bool el_file::file_exists(const std::string& file_name, const std::string &extra_path)
	{
		std::string file;
		Uint32 i;

		file = remove_path(file_name);

		if (!extra_path.empty())
		{
			if (file_exist_in_dir(extra_path + file))
			{
				return true;
			}
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

}

#endif //NEW_FILE_IO
