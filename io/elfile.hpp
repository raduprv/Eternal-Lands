/**
 * @file
 * @ingroup io
 * @brief file i/o object with support for zip and gzip files
 */

#ifndef	_ELFILE_HPP_
#define	_ELFILE_HPP_

#ifdef NEW_FILE_IO

#include "allio.hpp"
#include "zipfilesystem.hpp"
#include "elpathwrapper.h"
#include "../init.h"
#ifdef	EXTRA_DEBUG
#include "../errors.h"
#endif	// EXTRA_DEBUG

namespace eternal_lands
{

	const Uint32 max_mem_block_buffer_size = 0x80000; // 512kb

	class el_file
	{
		private:

			enum file_type
			{
				ft_gzip,		/**< File name ends with .gzip */
				ft_zip,			/**< File is in a zip archive */
				ft_uncompressed		/**< All other files */
			};

			static std::vector<std::string> path_list;

			/**
			 * @brief File position.
			 *
			 * The position in the file.
			 */
			Sint32 position;

			/**
			 * @brief Memory buffer.
			 *
			 * Memory buffer of the file data.
			 */
			memory_ptr memory;

			/**
			 * @brief File name.
			 *
			 * Name of the file.
			 */
			std::string file_name_str;

			/**
			 * @brief Opens the file in a zip file.
			 *
			 * Opens the file in a zip file.
			 * @param file_name The name of the file to open.
			 * @param uncompress Flag indicating if the file should get uncompressed.
			 * @param zfile_system The zip file system where to search for the file.
			 */
			inline void open_zip(const std::string& file_name, bool uncompress,
				zip_file_system& zfile_system)
			{
				zfile_system.open_file(file_name, memory, uncompress);
			}

			/**
			 * @brief Opens the file and uncompress it.
			 *
			 * Opens the file and uncompress it ignoring all files in the zip files.
			 * @param file_name The name of the file to open.
			 */
			void open_gzip(const std::string& file_name);

			/**
			 * @brief Tries to open the file and don't uncompress it.
			 *
			 * Tries to open the file and don't uncompress it ignoring all files in the
			 * zip files.
			 * @param file_name The name of the file to open.
			 */
			void open(const std::string& file_name);

			/**
			 * @brief Check if a file exists.
			 *
			 * Check if the given file exists using the given file type.
			 * @param file_name The name of the file.
			 * @param type The type of the file.
			 * @return Returns true if the file exists, else false.
			 * @see file_type
			 */
			static bool file_exist(const std::string& file_name, file_type type);

			/**
			 * @brief Check if a file exists.
			 *
			 * Check if the given file exists in the given dir, trying all possible file
			 * types.
			 * @param file_name The name of the file.
			 * @return Returns true if the file exists, else false.
			 * @see file_type
			 */
			static bool file_exist_in_dir(const std::string& file_name);

			/**
			 * @brief Opens a file.
			 *
			 * Opens a file ignoing all files in the zip files.
			 * @param file_name The name of the file to open.
			 * @param uncompress Flag indicating if the file should get uncompressed.
			 */
			inline void open(const std::string& file_name, bool uncompress)
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

			/**
			 * @brief Opens a file.
			 *
			 * Opens a file using the given file type.
			 * @param file_name The name of the file to open.
			 * @param uncompress Flag indicating if the file should get uncompressed.
			 * @param type The type of the file.
			 */
			void open(const std::string& file_name, bool uncompress, file_type type);
	
			/**
			 * @bief Opens a file if it exist.
			 *
			 * Opens a file if it exist and returns true or returns false if the file
			 * don't exist.
			 * @param file_name The name of the file to open.
			 * @param uncompress Flag indicating if the file should get uncompressed.
			 * @return Returns true if the files exist or false else.
			 */
			bool open_if_exist(const std::string& file_name, bool uncompress);

			/**
			 * @brief Returns the file name with the given dir.
			 *
			 * Returns the file name with the given path. Used for iteratin over all possible
			 * directories and doing that in the correct order.
			 * @param file_name The file name to use.
			 * @param path_index The index into path_list of the path to use.
			 * @return Returns the file name with the given path.
			 */
			static inline std::string get_file_name_with_path(const std::string
				&file_name, Uint32 path_index)
			{
				assert(path_index < path_list.size());

				return path_list[path_index] + file_name;
			}

			/**
			 * @brief The zip file system to use.
			 * The zip file system to use for opening a file.
			 */
			static zip_file_system default_zip_file_system;

			/**
			 * @brief Removes the leading "./" from the file name.
			 *
			 * Removes the leading "./" from the file name.
			 * @param file_name The file name to use.
			 * @returns The file name without "./".
			 */
			static inline std::string remove_path(const std::string &file_name)
			{
				std::string::size_type pos;
				std::string str;

				str = file_name;

				pos = str.find("./");
				while (pos != std::string::npos)
				{
					str = str.erase(pos, 2);
					pos = str.find("./");
				}

				pos = str.find("//");
				while (pos != std::string::npos)
				{
					str = str.erase(pos, 1);
					pos = str.find("//");
				}

				return str;
			}

		public:

			/**
			 * @brief Adds a path where to search for files.
			 *
			 * Adds a path where to search for files.
			 * @param path The path to add.
			 */
			static inline void add_path(const std::string &path)
			{
#ifdef	EXTRA_DEBUG
				LOG_EXTRA_INFO("Added path '%s'", path.c_str());
#endif	// EXTRA_DEBUG
				path_list.push_back(path);
			}

			/**
			 * @brief Opens a file.
			 *
			 * Opens a file read only in binary mode.
			 * @param file_name The name of the file to open.
			 * @param uncompress Flag indicating if the file should get uncompressed.
			 * @param extra_path Extra path where to look for the file. This path is
			 * used befor any of the added paths are used.
			 */
			el_file(const std::string &file_name, bool uncompress,
				const std::string &extra_path = "");
	
			/**
			 * @brief Reads data from the file.
			 *
			 * Reads data from the file.
			 * @param size The number of bytes to read.
			 * @param buffer The buffer for the read data.
			 * @return Returns the number of read bytes.
			 */
			inline Sint32 read(Sint32 count, void* buffer)
			{
				count = std::max(std::min(count, get_size() - position), 0);
				if (count <= 0)
				{
					return 0;
				}
				memcpy(buffer, memory->get_memory<void*>(position), count);
				position += count;

				return count;
			}

			/**
			 * @brief Sets the position in the file.
			 *
			 * Sets the position in the file. If seek_type is SEEK_SET, the new
			 * position is offset. If seek_type is SEEK_CUR, the new position is the
			 * old position plus the offset. If seek_type is SEEK_END, the new position
			 * is the file size minus the offset.
			 * @param offset The value used for the calculation for the new position.
			 * @param seek_type The type of seek. Can only be SEEK_SET, SEEK_END or
			 * SEEK_CUR.
			 * @return Returns the new position in the file.
			 */
			inline Sint32 seek(Sint32 offset, Sint32 seek_type)
			{
				Sint32 pos;

				switch (seek_type)
				{
					case SEEK_SET:
						pos = offset;
						break;
					case SEEK_END:
						pos = get_size() - offset;
						break;
					case SEEK_CUR:
						pos = position + offset;
						break;
					default:
						return -1;
				}
				if ((pos < 0) || (pos > get_size()))
				{
					return -1;
				}
				else
				{
					position = pos;
					return position;
				}
			}

			/**
			 * @brief Gets the position in the file.
			 *
			 * Gets the position in the file.
			 * @return Returns the position in the file.
			 */
			inline Sint32 tell() const
			{
				return position;
			}

			/**
			 * @brief Gets the size of the file.
			 *
			 * Gets the size of the file.
			 * @return Returns the size of the file.
			 */
			inline Sint32 get_size() const
			{
				return memory->get_size();
			}

			/**
			 * @brief Gets a pointer to the file data.
			 *
			 * Gets the memory pointer of the file data.
			 * @return Returns a memory pointer to the file data.
			 */
			inline void* get_pointer() const
			{
				return memory->get_memory<void*>();
			}

			/**
			 * @brief Gets a pointer to the file data at the current position.
			 *
			 * Gets the memory pointer of the file data at the current position.
			 * @return Returns a memory pointer to the file data at the current position.
			 */
			inline void* get_current_pointer() const
			{
				if (position >= get_size())
				{
					return 0;
				}
				else
				{
					return memory->get_memory<void*>(position);
				}
			}

			/**
			 * @brief Check if a file exists.
			 *
			 * Check if the given file exists.
			 * @param file_name The name of the file.
			 * @param extra_path Extra path where to look for the file. This path is
			 * used befor any of the added paths are used.
			 * @return Returns true if the file exists, else false.
			 */
			static bool file_exists(const std::string& file_name,
				const std::string &extra_path = "");

			/**
			 * @brief Adds a zip file to the search list.
			 *
			 * Adds a zip file to the list where to search for a file that is opend with
			 * open_file.
			 * @param file_name The file name of the zip file.
			 * @param replace True if we should replace files.
			 */
			static inline void add_zip_archive(const std::string &file_name,
				bool replace)
			{
				default_zip_file_system.add_zip_archive(file_name, replace);
			}

			/**
			 * @brief Adds a zip file to the search list.
			 *
			 * Adds a zip file to the list where to search for a file that is opend with
			 * open_file.
			 * @param file_name The file name of the zip file.
			 * @param path The path of the files in the zip file.
			 * @param replace True if we should replace files.
			 */
			static inline void add_zip_archive(const std::string &file_name,
				const std::string &path, bool replace)
			{
				default_zip_file_system.add_zip_archive(file_name, path, replace);
			}

			/**
			 * @brief Returns the file name.
			 *
			 * Returns the file name of the file. The name is with full path.
			 * @returns The file name.
			 * @see file_name_str
			 */
			inline const std::string &get_file_name() const
			{
				return file_name_str;
			}
	};

#ifdef	USE_TR1
	typedef std::tr1::shared_ptr<el_file> el_file_ptr;
#else
	typedef shared_ptr<el_file> el_file_ptr;
#endif
}

#endif //NEW_FILE_IO

#endif	// _ELFILE_HPP_
