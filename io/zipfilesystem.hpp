/**
 * @file
 * @ingroup io
 * @brief zip file system
 */
#ifndef	_ZIPFILESYSTEM_HPP_
#define	_ZIPFILESYSTEM_HPP_

#ifdef NEW_FILE_IO

#include <string.h>
#include "allio.hpp"
#include "memorybuffer.hpp"

namespace eternal_lands
{

	class zip_file_system
	{
		private:

			/**
			 * @brief Zip file data struct.
			 *
			 * Zip file data struct.
			 * @{
			 */
			typedef struct
			{
				std::string name;		/**< Name of the zip file. */
				std::ifstream* file;		/**< File stream of the zip file. */
				int bytes_before_zipfile;	/**< Bytes before the zip file. */
			} zip_file;
			/* @} */

			typedef std::vector<zip_file> zip_files_vector;

			/**
			 * @brief Zip file file entry struct.
			 *
			 * Zip file file entry struct.
			 * @{
			 */
			typedef struct
			{
				int compressed_size; 				/**< Compressed file size. */
				int uncompressed_size;				/**< Uncompressed file size. */
				int offset_curfile; 				/**< Offset to the file data. */
				zip_files_vector::size_type zip_file_index;	/**< Index of the zip. */
				int crc32; 					/**< Crc32. */
				bool is_compressed;			 	/**< Flag that indicates if the file is compressed. */
			} zip_file_entry;
			/* @} */

#ifdef	USE_TR1
			/**
			 * @brief Type of the files list.
			 *
			 * An unordered map, so the searching of the files is faster. Verry fast, because
			 * it use hashs. Needs tr1.
			 * @see file_entrys
			 */
			typedef std::tr1::unordered_map<std::string, zip_file_entry> zip_file_entry_list;
#else
			/**
			 * @brief Type of the files list.
			 *
			 * An orderd map, so the searching of the files is faster. Not so fast as an
			 * unordered map with hashs, but the default STL don't have hashs or unordered maps.
			 * @see file_entrys
			 */
			typedef std::map<std::string, zip_file_entry> zip_file_entry_list;
#endif

			/**
			 * @brief List of the files in the zip archives.
			 *
			 * The list of the files that are in the zip archives.
			 * @see zip_files
			 */
			zip_file_entry_list file_entrys;

			/**
			 * @brief Vector of zip files.
			 *
			 * Vector of zip files that are used for file opening. Contains file name of the
			 * zip file and either a file pointer or a memory that points to the file data.
			 */
			zip_files_vector zip_files;

			/**
			 * @brief Reads the files entry.
			 *
			 * Reads the files entry form a pointer and returns the position.
			 * @param pos The pointer from where to read the data.
			 * @param size The size of the memory block to that pos points.
			 * @return Returns the position of the files entry block relativ to the pointer.
			 */
			int read_files_entry(const Uint8* pos, int size) const;

			/**
			 * @brief Gets an Uint32 from pos and increase pos.
			 *
			 * Gets an Uint32 from pointer pos and increase the pointer pos at
			 * sizeof(Uint32).
			 * @param pos The pointer from where to get the Uint32.
			 * @return Returns the Uint32.
			 */
			Uint32 get_uint32_from_pos(Uint8* &pos) const
			{
				Uint32 value;

				memcpy(&value, pos, sizeof(Uint32));
				pos = &pos[sizeof(Uint32)];

				return SDL_SwapLE32(value);
			}

			/**
			 * @brief Gets an Uint16 from pos and increase pos.
			 *
			 * Gets an Uint16 from pointer pos and increase the pointer pos at
			 * sizeof(Uint16).
			 * @param pos The pointer from where to get the Uint16.
			 * @return Returns the Uint16.
			 */
			Uint16 get_uint16_from_pos(Uint8* &pos) const
			{
				Uint16 value;

				memcpy(&value, pos, sizeof(Uint16));
				pos = &pos[sizeof(Uint16)];

				return SDL_SwapLE16(value);
			}

			/**
			 * @brief Reads the files informations.
			 *
			 * Reads the informations from the pointer and adds the files to to the files
			 * entrys map.
			 * @param pos The pointer to memory where to get the data from.
			 * @param count The number of files that are in the zip file.
			 * @param path The path of the zip file where the files are in.
			 * @param files The map where to add the files.
			 * @param replace True if we should replace files.
			 * @see file_entrys
			 */
			void read_files_infos(Uint8* pos, int count, const std::string& path,
				zip_file_entry_list& files, bool replace);

			/**
			 * @brief Reads the file header.
			 *
			 * Reads the informations from the file header into @see zfile.
			 * @param zfile The zip file entry.
			 * @see zip_files
			 * @see zip_file_entry
			 */
			void read_file_header(zip_file_entry &zfile);

			/**
			 * @brief
			 *
			 * @param path The path of the zip file where the files are in.
			 * @param files The map where to add the files.
			 * @param replace True if we should replace files.
			 */
			void add(const std::string& path, zip_file_entry_list &files,
				bool replace);

			/**
			 * @brief Gets the file path.
			 *
			 * Gets the path of the given file.
			 * @returns file_name The file name where to extract the path from.
			 * @returns Returns the path of the given file.
			 */		
			static inline std::string get_path(const std::string &file_name)
			{
				std::string::size_type pos_1, pos_2;

				pos_1 = file_name.rfind("/");
				pos_2 = file_name.rfind("\\");

				if (pos_1 != std::string::npos)
				{
					if (pos_2 != std::string::npos)
					{
						return file_name.substr(0, std::max(pos_1, pos_2) + 1);
					}
					else
					{
						return file_name.substr(0, pos_1 + 1);
					}
				}
				else
				{
					if (pos_2 != std::string::npos)
					{
						return file_name.substr(0, pos_2 + 1);
					}
					else
					{
						return "";
					}
				}
			}

			/**
			 * @brief Gets the zip file name from a zip file entry.
			 *
			 * Gets the zip file name from a zip file entry.
			 * @param zfile One zip file entry from the requested zip file.
			 * @return Returns the name of the zip file.
			 */
			inline const std::string &get_zip_file_name(const zip_file_entry &zfile) const
			{
				zip_files_vector::size_type index;

				index = zfile.zip_file_index;

				assert(index < zip_files.size());

				return zip_files[index].name;
			}

		public:
			/**
			 * @brief Adds a zip file to the search list.
			 *
			 * Adds a zip file to the list where to search for a file that is opend with
			 * open_file.
			 * @param file_name The file name of the zip file.
			 * @param replace True if we should replace files.
			 * @see open_file
			 */		
			void add_zip_archive(const std::string &file_name, bool replace);

			/**
			 * @brief Adds a zip file to the search list.
			 *
			 * Adds a zip file to the list where to search for a file that is opend with
			 * open_file.
			 * @param file_name The file name of the zip file.
			 * @param path The path of the files @b in the zip file.
			 * @param replace True if we should replace files.
			 * @see open_file
			 */
			void add_zip_archive(const std::string &file_name, const std::string &path,
				bool replace);

			/**
			 * @brief Opens a file from a zip archive.
			 *
			 * Opens a file that is in one of the zip archives previously added and sets the
			 * buffer to the (compressed or uncompressed) file data.
			 * @param file_name The name of the file to be opend.
			 * @param buffer A pointer that the function sets to the uncompressed data. Should
			 * not be a valid pointer, because the address gets overwritten.
			 * @param uncompress Flag that indicates if the file data gets uncompressed.
			 * @return Returns the size of the file (compressed or uncompressed).
			 * @see add_zip_archive
			 */
			Uint32 open_file(const std::string &file_name, memory_ptr &buffer, bool uncompress);

			/**
			 * @brief Checks if the file exist in any of the zip files.
			 *
			 * Checks if the file exist in any of the zip files.
			 * @param file_name The name of the file to be checked.
			 * @return Returns true if the file is in the zip files or false else.
			 */
			inline bool file_exist(const std::string &file_name) const
			{
				zip_file_entry_list::const_iterator found;

				found = file_entrys.find(file_name);

				return found != file_entrys.end();
			}
	};

}

#endif //NEW_FILE_IO

#endif	//_ZIPFILESYSTEM_HPP_
