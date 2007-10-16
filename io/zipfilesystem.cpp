#include "zipfilesystem.hpp"

namespace eternal_lands
{

	void zip_file_system::add(const std::string& path, zip_file_entry_list &files,
		bool replace)
	{
		Uint8* pos;
		int index, L, number_disk, number_disk_with_CD;
		int number_entry, number_entry_CD, size_central_dir, offset_central_dir;
		int size_comment, central_pos, bytes_before_zipfile;
		long int size;

		zip_files.back().file->open(zip_files.back().name.c_str(), std::ios::binary);
		if (!zip_files.back().file->is_open())
		{
			EXTENDED_EXCEPTION(extended_exception::ec_file_not_found, "Can't find file "
				<< zip_files.back().name);
		}

		zip_files.back().file->seekg(0, std::ios::end);
		size = zip_files.back().file->tellg();

		central_pos = size;
		size = std::min(size, 0x10020L);
		central_pos -= size;

		memory_ptr memory(new memory_buffer(size));

		zip_files.back().file->seekg(-size, std::ios::end);
		zip_files.back().file->read(memory->get_memory<char*>(), size);

		index = read_files_entry(memory->get_memory<Uint8*>(), size);
		central_pos += index;
		pos = memory->get_memory<Uint8*>(index);

		L = get_uint32_from_pos(pos);
		number_disk = get_uint16_from_pos(pos);
		number_disk_with_CD = get_uint16_from_pos(pos);
		number_entry = get_uint16_from_pos(pos);
		number_entry_CD = get_uint16_from_pos(pos);
		size_central_dir = get_uint32_from_pos(pos);
		offset_central_dir = get_uint32_from_pos(pos);
		size_comment = get_uint16_from_pos(pos);

		if ((number_entry_CD != number_entry) || (number_disk_with_CD != 0) ||
			(number_disk != 0))
		{
			EXTENDED_EXCEPTION(extended_exception::ec_zip_error, "Invalid zip file!");
		}

		bytes_before_zipfile = central_pos - (offset_central_dir + size_central_dir);

		zip_files.back().bytes_before_zipfile = bytes_before_zipfile;

		memory_ptr extra_memory(new memory_buffer(size_central_dir));;

		zip_files.back().file->seekg(central_pos - size_central_dir, std::ios::beg);
		zip_files.back().file->read(extra_memory->get_memory<char*>(), size_central_dir);
		read_files_infos(extra_memory->get_memory<Uint8*>(), number_entry, path, files, replace);
	}

	void zip_file_system::add_zip_archive(const std::string &file_name, bool replace)
	{
		zip_file zfile;
		zip_file_entry_list files;

		zfile.name = file_name;
		zfile.file = new std::ifstream();
		try
		{
			zip_files.push_back(zfile);
			try
			{
				add(get_path(file_name), files, replace);
				file_entrys.insert(files.begin(), files.end());
			}
			catch (...)
			{
				zip_files.pop_back();
				throw;
			}
		}
		catch (...)
		{
			delete zfile.file;
			throw;
		}
	}

	void zip_file_system::add_zip_archive(const std::string &file_name, const std::string &path,
		bool replace)
	{
		zip_file zfile;
		zip_file_entry_list files;

		zfile.name = file_name;
		zfile.file = new std::ifstream();
		try
		{
			zip_files.push_back(zfile);
			try
			{
				add(path, files, replace);
				file_entrys.insert(files.begin(), files.end());
			}
			catch (...)
			{
				zip_files.pop_back();
				throw;
			}
		}
		catch (...)
		{
			delete zfile.file;
			throw;
		}
	}

	int zip_file_system::read_files_entry(const Uint8* pos, int size) const
	{
		int i;

		for (i = size - 4; i >= 0; i--)
		{
			if ((pos[i + 0] == 0x50) && (pos[i + 1] == 0x4b) &&
				(pos[i + 2] == 0x05) && (pos[i + 3] == 0x06))
			{
				return i;
			}
		}
		EXTENDED_EXCEPTION(extended_exception::ec_zip_error, "Can't find magic number "
			<< "(0x50 0x4B 0x05 0x06) of files entry in file "
			<< zip_files.back().name);
	}

	void zip_file_system::read_files_infos(Uint8* pos, int count, const std::string& path,
		zip_file_entry_list &files, bool replace)
	{
		int magic, version, version_needed, flag, compression_method, dosDate;
		int size_filename, size_file_extra, size_file_comment;
		int disk_num_start, internal_fa, external_fa, i;
		zip_file_entry zfile;
		std::string str;

		zfile.zip_file_index = zip_files.size() - 1;

		for (i = 0; i < count; i++)
		{
			magic = get_uint32_from_pos(pos);
			if (magic != 0x02014b50)
			{
				EXTENDED_EXCEPTION(extended_exception::ec_zip_error, "Wrong magic"
					<< " number! Found: 0x" << std::hex << magic
					<< ", expected: 0x" << std::hex << 0x02014b50);
			}
			version = get_uint16_from_pos(pos);
			version_needed = get_uint16_from_pos(pos);
			flag = get_uint16_from_pos(pos);
			compression_method = get_uint16_from_pos(pos);
			if ((compression_method != Z_DEFLATED) && (compression_method != 0))
			{
				EXTENDED_EXCEPTION(extended_exception::ec_zip_error,
					"Unsupported compression method " << compression_method
					<< " in zip file " << zip_files.back().name);
			}
			dosDate = get_uint32_from_pos(pos);
			zfile.crc32 = get_uint32_from_pos(pos);
			zfile.compressed_size = get_uint32_from_pos(pos);
			zfile.uncompressed_size = get_uint32_from_pos(pos);
			size_filename = get_uint16_from_pos(pos);
			size_file_extra = get_uint16_from_pos(pos);
			size_file_comment = get_uint16_from_pos(pos);
			disk_num_start = get_uint16_from_pos(pos);
			internal_fa = get_uint16_from_pos(pos);
			external_fa = get_uint32_from_pos(pos);
			zfile.offset_curfile = get_uint32_from_pos(pos);

			read_file_header(zfile);
			str = path;
			str.append(reinterpret_cast<char*>(pos), size_filename);

			zip_file_entry_list::iterator found;

			found = files.find(str);
			if (found != files.end())
			{
				if (replace)
				{
					files.erase(found);
					files[str] = zfile;
	#ifdef	EXTRA_DEBUG
					LOG_EXTRA_INFO("Replaced file '%s' in zip file '%s' with file '%s' from zip file '%s'.",
						str.c_str(), get_zip_file_name(found->second).c_str(),
						str.c_str(), get_zip_file_name(zfile).c_str());
	#endif	// EXTRA_DEBUG
				}
				else
				{
					EXTENDED_EXCEPTION(extended_exception::ec_duplicate_item, 
						"Duplicate file " << str << " from zip file "
						<< get_zip_file_name(zfile) << ", not added again."
						<< " The file was first added from zip file "
						<< get_zip_file_name(found->second));
				}
			}
			else
			{
				files[str] = zfile;
	#ifdef	EXTRA_DEBUG
				LOG_EXTRA_INFO("Added file '%s' from zip file '%s'.",
					str.c_str(), get_zip_file_name(zfile).c_str());
	#endif	// EXTRA_DEBUG
			}
			pos = &pos[size_filename];
			pos = &pos[size_file_extra];
			pos = &pos[size_file_comment];
		}
	}

	Uint32 zip_file_system::open_file(const std::string &file_name, memory_ptr &buffer,
		bool uncompress)
	{
		z_stream strm;
		zip_file_entry_list::const_iterator found;
		uLongf buffer_size;
		zip_files_vector::size_type index;
		int size, offset;
		int crc;
		int error;

		found = file_entrys.find(file_name);
		if (found != file_entrys.end())
		{
			size = found->second.compressed_size;
			buffer_size = found->second.uncompressed_size;
			index = found->second.zip_file_index;
			assert(index < zip_files.size());
			offset = found->second.offset_curfile + zip_files[index].bytes_before_zipfile;

#ifdef	EXTRA_DEBUG
			LOG_EXTRA_INFO("File '%s' opend in zip file '%s'.", file_name.c_str(),
				zip_files[index].name.c_str());
#endif	// EXTRA_DEBUG

			if (uncompress && found->second.is_compressed)
			{
				memory_ptr memory(new memory_buffer(size));

				buffer->resize(buffer_size);

				zip_files[index].file->seekg(offset, std::ios::beg);
				zip_files[index].file->read(memory->get_memory<char*>(), size);

				strm.next_in = memory->get_memory<Bytef*>();
				strm.avail_in = size;
				strm.total_in = 0;
				strm.next_out = buffer->get_memory<Bytef*>();
				strm.avail_out = buffer_size;
				strm.total_out = 0;
				strm.zalloc = Z_NULL;
				strm.zfree = Z_NULL;
				strm.opaque = Z_NULL;

				error = inflateInit2(&strm, -MAX_WBITS);
				if (error != Z_OK)
				{
					EXTENDED_EXCEPTION(extended_exception::ec_zip_error, strm.msg);
				}

				error = inflate(&strm, Z_FINISH);
				if (error != Z_STREAM_END)
				{
					EXTENDED_EXCEPTION(extended_exception::ec_zip_error, strm.msg);
				}

				error = inflateEnd(&strm);
				if (error != Z_OK)
				{
					EXTENDED_EXCEPTION(extended_exception::ec_zip_error, strm.msg);
				}

				crc = crc32(0L, Z_NULL, 0);
				crc = crc32(crc, buffer->get_memory<Bytef*>(), buffer_size);
				if (crc != found->second.crc32)
				{
					EXTENDED_EXCEPTION(extended_exception::ec_zip_error,
						"CRC error in zip file " << zip_files[index].name
						<< "! Found: 0x" << std::hex << crc
						<< ", expected: 0x" << std::hex
						<< found->second.crc32);
				}

				return buffer_size;
			}
			else
			{
				buffer->resize(size);

				zip_files[index].file->seekg(offset, std::ios::beg);
				zip_files[index].file->read(buffer->get_memory<char*>(), size);

				return size;
			}
		}
		EXTENDED_EXCEPTION(extended_exception::ec_file_not_found, "Can't find file "
			<< file_name << " in zip files");
	}

	void zip_file_system::read_file_header(zip_file_entry &zfile)
	{
		/* Size must be 30! */
		Uint8 buffer[30];
		Uint8* pos;
		int offset, magic, version, flag, compression_method, dosDate;
		int crc, compressed_size, uncompressed_size, size_filename, size_file_extra;

		offset = zfile.offset_curfile + zip_files.back().bytes_before_zipfile;

		pos = buffer;
		zip_files.back().file->seekg(offset, std::ios::beg);
		zip_files.back().file->read(reinterpret_cast<char*>(buffer), sizeof(buffer));

		magic = get_uint32_from_pos(pos);
		if (magic != 0x04034b50)
		{
			EXTENDED_EXCEPTION(extended_exception::ec_zip_error, "Wrong magic number "
				<< "for file header of zip file " << zip_files.back().name
				<< "! Found: 0x" << std::hex << magic << ", expected: 0x"
				<< std::hex << 0x04034b50);
		}
		version = get_uint16_from_pos(pos);
		flag = get_uint16_from_pos(pos);
		compression_method = get_uint16_from_pos(pos);
		if ((compression_method != Z_DEFLATED) && (compression_method != 0))
		{
			EXTENDED_EXCEPTION(extended_exception::ec_zip_error,
				"Unsupported compression method " << compression_method
				<< " in zip file " << zip_files.back().name);
		}
		dosDate = get_uint32_from_pos(pos);
		crc = get_uint32_from_pos(pos);
		compressed_size = get_uint32_from_pos(pos);
		uncompressed_size = get_uint32_from_pos(pos);
		size_filename = get_uint16_from_pos(pos);
		size_file_extra = get_uint16_from_pos(pos);

		zfile.is_compressed = compression_method == Z_DEFLATED;
		zfile.offset_curfile += sizeof(buffer);
		zfile.offset_curfile += size_filename;
		zfile.offset_curfile += size_file_extra;
	}

}

