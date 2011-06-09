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
	const char *buf = el_get_pointer(file);
	Sint64 file_size = el_get_size(file);
	const elc_file_header *header;
	int header_offset;
	MD5 md5;
	MD5_DIGEST md5_digest;

	if (file_size < sizeof(elc_file_header)) 
	{
		LOG_ERROR("File '%s' too small! %d, %d", filename, file_size, sizeof(elc_file_header));
		return -1;
	}

	header = (const elc_file_header *)buf;
	if (memcmp(header->magic, magic, sizeof(magic_number)) != 0)
	{
		LOG_ERROR("Wrong file '%s'! Magic number not correct! Should be \"%.*s\", but is \"%.*s\"!",
			filename, sizeof(magic_number), magic,
			sizeof(magic_number), header->magic);
		return -1;
	}

	memcpy(version, header->version, sizeof(version_number));
	header_offset = SDL_SwapLE32(header->header_offset);

	MD5Open(&md5);
	MD5Digest(&md5, buf+header_offset, file_size-header_offset);
	MD5Close(&md5, md5_digest);

	if (memcmp(header->md5, md5_digest, sizeof(MD5_DIGEST)) != 0)
	{
		LOG_ERROR("Wrong MD5! File '%s' is corrupt! MD5 [%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x] MD5 [%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x]", filename, header->md5[0], header->md5[1], header->md5[2], header->md5[3], header->md5[4], header->md5[5], header->md5[6], header->md5[7], header->md5[8], header->md5[9], header->md5[10], header->md5[11], header->md5[12], header->md5[13], header->md5[14], header->md5[15], md5_digest[0], md5_digest[1], md5_digest[2], md5_digest[3], md5_digest[4], md5_digest[5], md5_digest[6], md5_digest[7], md5_digest[8], md5_digest[9], md5_digest[10], md5_digest[11], md5_digest[12], md5_digest[13], md5_digest[14], md5_digest[15]);
		return -1;
	}

	el_seek(file, header_offset, SEEK_SET);

	return 0;
}
