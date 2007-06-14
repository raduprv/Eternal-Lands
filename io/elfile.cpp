/*
 * el file
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 * el file is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * el file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with el file. If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

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

