#include <string.h>
#include <SDL_endian.h>
#ifdef OSX
 #include <sys/malloc.h>
#elif BSD
 #include <stdlib.h>
#else // OSX
 #include <malloc.h>
#endif // OSX
#include "elc_io.h"
#include "../errors.h"

#define	BLOCK_SIZE	1024*1024

int read_and_check_elc_header(el_file_ptr file, const magic_number magic, version_number *version, const char* filename)
{
	elc_file_header header;
	int size, header_offset;
	void* mem;
	MD5 md5;
	MD5_DIGEST md5_digest;
	
	size = el_read(file, sizeof(elc_file_header), &header);
	if (size != sizeof(elc_file_header)) 
	{
		LOG_ERROR("File '%s' too small! %d, %d", filename, size, sizeof(elc_file_header));
		return -1;
	}

	if (memcmp(header.magic, magic, sizeof(magic_number)) != 0)
	{
		char *m_str = (char *)malloc(sizeof(magic_number)+1);
		char *hm_str = (char *)malloc(sizeof(magic_number)+1);

		memcpy(m_str, magic, sizeof(magic_number));
		m_str[sizeof(magic_number)] = 0;

		memcpy(hm_str, header.magic, sizeof(header.magic));
		hm_str[sizeof(magic_number)] = 0;

		LOG_ERROR("Wrong file '%s'! Magic number not correct! Should be \"%s\", but is \"%s\"!", filename, m_str, hm_str);

		free(m_str);
		free(hm_str);
		return -1;
	}

	memcpy(version, header.version, sizeof(version_number));

	header_offset = SDL_SwapLE32(header.header_offset);

	MD5Open(&md5);
	size = el_get_size(file) - header_offset;
	mem = &((Uint8*)el_get_pointer(file))[header_offset];
	MD5Digest(&md5, mem, size);
	MD5Close(&md5, md5_digest);

	if (memcmp(header.md5, md5_digest, sizeof(MD5_DIGEST)) != 0)
	{
		LOG_ERROR("Wrong MD5! File '%s' is corrupt! MD5 [%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x] MD5 [%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x]", filename, header.md5[0], header.md5[1], header.md5[2], header.md5[3], header.md5[4], header.md5[5], header.md5[6], header.md5[7], header.md5[8], header.md5[9], header.md5[10], header.md5[11], header.md5[12], header.md5[13], header.md5[14], header.md5[15], md5_digest[0], md5_digest[1], md5_digest[2], md5_digest[3], md5_digest[4], md5_digest[5], md5_digest[6], md5_digest[7], md5_digest[8], md5_digest[9], md5_digest[10], md5_digest[11], md5_digest[12], md5_digest[13], md5_digest[14], md5_digest[15]);
		return -1;
	}

	el_seek(file, SDL_SwapLE32(header.header_offset), SEEK_SET);

	return 0;
}
