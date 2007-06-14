#include <SDL_net.h>
#include "zipfilesystem.hpp"

void zip_file_system::add(uint_fast32_t idx, const std::string& path, zip_file_entry_list &files)
{
	uint8_t* pos;
	uint_fast32_t index, L, number_disk, number_disk_with_CD;
	uint_fast32_t number_entry, number_entry_CD, size_central_dir, offset_central_dir;
	uint_fast32_t size_comment, central_pos, bytes_before_zipfile;
	long int size;

	zip_files[idx].file->open(zip_files[idx].name.c_str(), std::ios::binary);
	if (!zip_files[idx].file->is_open())
	{
		EXTENDED_FILE_NOT_FOUND_EXCEPTION(zip_files[idx].name);
	}

	zip_files[idx].file->seekg(0, std::ios::end);
	size = zip_files[idx].file->tellg();

	central_pos = size;
	size = std::min(size, 0x10020L);
	central_pos -= size;

	memory_buffer memory(size);

	zip_files[idx].file->seekg(-size, std::ios::end);
	zip_files[idx].file->read(reinterpret_cast<char*>(memory.get_memory()), size);

	index = read_files_entry(memory.get_memory(), size);
	central_pos += index;
	pos = memory.get_memory(index);

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
		EXTENDED_EXCEPTION("Invalid zip file!");
	}

	bytes_before_zipfile = central_pos - (offset_central_dir + size_central_dir);

	zip_files[idx].bytes_before_zipfile = bytes_before_zipfile;

	memory_buffer extra_memory(size_central_dir);

	zip_files[idx].file->seekg(central_pos - size_central_dir, std::ios::beg);
	zip_files[idx].file->read(reinterpret_cast<char*>(extra_memory.get_memory()), size_central_dir);
	read_files_infos(extra_memory.get_memory(), number_entry, idx, path, files);
}

void zip_file_system::add_zip_archive(const std::string &file_name)
{
	zip_file zfile;
	zip_file_entry_list files;
	std::string path;
	int_fast32_t pos;

	zfile.name = file_name;
	zfile.file = new std::ifstream();
	try
	{
		zip_files.push_back(zfile);
		try
		{
			pos = file_name.rfind("/");
			if (pos < 0)
			{
				pos = file_name.rfind("\\");
			}
			path = file_name.substr(0, pos);
			add(zip_files.size() - 1, path, files);
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

uint_fast32_t zip_file_system::read_files_entry(uint8_t* pos, uint_fast32_t size) const
{
	uint_fast32_t i;

	for (i = size - 4; i >= 0; i--)
	{
		if ((pos[i + 0] == 0x50) && (pos[i + 1] == 0x4b) &&
			(pos[i + 2] == 0x05) && (pos[i + 3] == 0x06))
		{
			return i;
		}
	}
	EXTENDED_EXCEPTION("No valid zip file.");
}

uint_fast32_t zip_file_system::get_uint32_from_pos(uint8_t* &pos) const
{
	uint32_t value;

	memcpy(&value, pos, sizeof(uint32_t));
	pos = &pos[sizeof(uint32_t)];

	return SDL_SwapLE32(value);
}

uint_fast32_t zip_file_system::get_uint16_from_pos(uint8_t* &pos) const
{
	uint16_t value;

	memcpy(&value, pos, sizeof(uint16_t));
	pos = &pos[sizeof(uint16_t)];

	return SDL_SwapLE16(value);
}

void zip_file_system::read_files_infos(uint8_t* pos, uint_fast32_t count, uint_fast32_t index,
	const std::string& path, zip_file_entry_list &files)
{
	uint_fast32_t magic, version, version_needed, flag, compression_method, dosDate;
	uint_fast32_t size_filename, size_file_extra, size_file_comment;
	uint_fast32_t disk_num_start, internal_fa, external_fa, i;
	zip_file_entry zfile;
	std::string str;

	zfile.zip_file_index = index;

	for (i = 0; i < count; i++)
	{
		magic = get_uint32_from_pos(pos);
		if (magic != 0x02014b50)
		{
			EXTENDED_EXCEPTION("Wrong magic number!");
		}
		version = get_uint16_from_pos(pos);
		version_needed = get_uint16_from_pos(pos);
		flag = get_uint16_from_pos(pos);
		compression_method = get_uint16_from_pos(pos);
		if ((compression_method != Z_DEFLATED) && (compression_method != 0))
		{
			EXTENDED_EXCEPTION("Unsupported compression method!");
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

		read_file_header(zfile, index);
		str = path;
		str.append((char*)pos, size_filename);
		files[str] = zfile;
		pos = &pos[size_filename];
		pos = &pos[size_file_extra];
		pos = &pos[size_file_comment];
	}
}

uint_fast32_t zip_file_system::open_file(const std::string &file_name, memory_buffer &buffer,
	bool uncompr)
{
	z_stream strm;
	zip_file_entry_list::const_iterator found;
	uLongf buffer_size;
	uint_fast32_t size, offset, index;
	uint_fast32_t crc;
	int_fast32_t error;

	found = file_entrys.find(file_name);
	if (found != file_entrys.end())
	{
		size = found->second.compressed_size;
		buffer_size = found->second.uncompressed_size;
		index = found->second.zip_file_index;
		offset = found->second.offset_curfile + zip_files[index].bytes_before_zipfile;
		if (uncompr && found->second.is_compressed)
		{
			memory_buffer memory(size);

			buffer = memory_buffer(buffer_size);

			zip_files[index].file->seekg(offset, std::ios::beg);
			zip_files[index].file->read(reinterpret_cast<char*>(memory.get_memory()),
				size);

			strm.next_in = reinterpret_cast<Bytef*>(memory.get_memory());
			strm.avail_in = size;
			strm.total_in = 0;
			strm.next_out = static_cast<Bytef*>(buffer.get_memory());
			strm.avail_out = buffer_size;
			strm.total_out = 0;
			strm.zalloc = Z_NULL;
			strm.zfree = Z_NULL;
			strm.opaque = Z_NULL;

			error = inflateInit2(&strm, -MAX_WBITS);
			if (error != Z_OK)
			{
				ZLIB_EXCEPTION(strm);
			}

			error = inflate(&strm, Z_FINISH);
			if (error != Z_STREAM_END)
			{
				ZLIB_EXCEPTION(strm);
			}

			error = inflateEnd(&strm);
			if (error != Z_OK)
			{
				ZLIB_EXCEPTION(strm);
			}

			crc = crc32(0L, Z_NULL, 0);
			crc = crc32(crc, static_cast<Bytef*>(buffer.get_memory()), buffer_size);
			if (crc != found->second.crc32)
			{
				EXTENDED_EXCEPTION("CRC error!");
			}

			return buffer_size;
		}
		else
		{
			buffer = memory_buffer(size);

			zip_files[index].file->seekg(offset, std::ios::beg);
			zip_files[index].file->read(reinterpret_cast<char*>(buffer.get_memory()),
				size);

			return size;
		}
	}
	return 0;
}

void zip_file_system::read_file_header(zip_file_entry &zfile, uint_fast32_t index)
{
	/* Size must be 30! */
	uint8_t buffer[30];
	uint8_t* pos;
	uint_fast32_t offset, magic, version, flag, compression_method, dosDate;
	uint_fast32_t crc, compressed_size, uncompressed_size, size_filename, size_file_extra;

	offset = zfile.offset_curfile + zip_files[index].bytes_before_zipfile;

	pos = buffer;
	zip_files[index].file->seekg(offset, std::ios::beg);
	zip_files[index].file->read(reinterpret_cast<char*>(buffer), sizeof(buffer));

	magic = get_uint32_from_pos(pos);
	if (magic != 0x04034b50)
	{
		EXTENDED_EXCEPTION("Wrong magic number!");
	}
	version = get_uint16_from_pos(pos);
	flag = get_uint16_from_pos(pos);
	compression_method = get_uint16_from_pos(pos);
	if ((compression_method != Z_DEFLATED) && (compression_method != 0))
	{
		EXTENDED_EXCEPTION("Unsupported compression method!");
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
