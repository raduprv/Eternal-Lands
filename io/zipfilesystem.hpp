/**
 * @file
 * @ingroup io
 * @brief zip file system
 */
#ifndef	_ZIPFILESYSTEM_HPP_
#define	_ZIPFILESYSTEM_HPP_

#ifdef NEW_FILE_IO

#include "allio.hpp"
#include "memorybuffer.hpp"

#define ZLIB_EXCEPTION(strm) EXTENDED_EXCEPTION(strm.msg)

/*
	We always use the file that is *first* added. I will change it later :)
*/
class zip_file_system
{
	private:

		typedef struct
		{
			int compressed_size; /**< Compressed file size. */
			int uncompressed_size; /**< Uncompressed file size. */
			int offset_curfile; /**< Offset to the file data. */
			int zip_file_index; /**< Index of the zip. */
			int crc32; /**< Crc32. */
			bool is_compressed; /**< Flag that indicates if the file is compressed. */
		} zip_file_entry;

		typedef struct
		{
			std::string name; /**< Name of the zip file. */
			std::ifstream* file; /**< File stream of the zip file. */
			int bytes_before_zipfile; /**< Bytes before the zip file. */
		} zip_file;
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
		std::vector<zip_file> zip_files;

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
		 * @brief Gets an uint32_t from pos and increase pos.
		 *
		 * Gets an uint32_t from pointer pos and increase the pointer pos at
		 * sizeof(uint32_t).
		 * @param pos The pointer from where to get the uint32_t.
		 * @return Returns the uint32_t.
		 */
		int get_uint32_from_pos(Uint8* &pos) const;

		/**
		 * @brief Gets an uint16_t from pos and increase pos.
		 *
		 * Gets an uint16_t from pointer pos and increase the pointer pos at
		 * sizeof(uint16_t).
		 * @param pos The pointer from where to get the uint16_t.
		 * @return Returns the uint16_t.
		 */
		int get_uint16_from_pos(Uint8* &pos) const;

		/**
		 * @brief Reads the files informations.
		 *
		 * Reads the informations from the pointer and adds the files to to the files
		 * entrys map.
		 * @param pos The pointer to memory where to get the data from.
		 * @param count The number of files that are in the zip file.
		 * @param index The index of the zip file where the files are in.
		 * @param path The path of the zip file where the files are in.
		 * @see file_entrys
		 */
		void read_files_infos(Uint8* pos, int count, int index,
			const std::string& path, zip_file_entry_list& files);

		void read_file_header(zip_file_entry &zfile, int index);

		void add(int idx, const std::string& path, zip_file_entry_list &files);
	public:
		/**
		 * @brief Adds a zip file to the search list.
		 *
		 * Adds a zip file to the list where to search for a file that is opend with
		 * open_file.
		 * @param file_name The file name of the zip file.
		 * @see open_file
		 */		
		void add_zip_archive(const std::string &file_name);

		/**
		 * @brief Opens a file from a zip archive.
		 *
		 * Opens a file that is in one of the zip archives previously added and sets the
		 * buffer to the (compressed or uncompressed) file data.
		 * @param file_name The name of the file to be opend.
		 * @param buffer A pointer that the function sets to the uncompressed data. Should
		 * not be a valid pointer, because the address gets overwritten.
		 * @param uncompr Flag that indicates if the file data gets uncompressed.
		 * @return Returns the size of the file (compressed or uncompressed) or zero if the
		 * file is not found.
		 * @see add_zip_archive
		 */
		int open_file(const std::string &file_name, memory_ptr &buffer,
			bool uncompr);

		inline bool file_exists(const std::string &file_name) const
		{
			zip_file_entry_list::const_iterator found;

			found = file_entrys.find(file_name);

			return found != file_entrys.end();
		}
};

#endif //NEW_FILE_IO

#endif	//_ZIPFILESYSTEM_HPP_
