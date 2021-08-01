#ifdef PACKET_COMPRESSION
#include <zlib.h>
#endif
#include "proxy_ext.h"
#include "elloggingwrapper.h"
#include "multiplayer.h"
#include "text.h"

typedef enum
{
	PROXY_COMPRESSED = 0
} proxy_cmd;

void handle_proxy_command(const uint8_t* data, size_t data_len)
{
	if (data_len < 1)
	{
		LOG_WARNING("Received an empty proxy command from the server");
		return;
	}

	switch (data[0])
	{
#ifdef PACKET_COMPRESSION
		case OL_COMPRESSED_PACKET:
		{
			uint8_t buffer[256*1024];
			uLong buf_len;
			int result;

			if (data_len < 4)
			{
				LOG_WARNING("CAUTION: Possibly forged/invalid PROXY_COMPRESSED packet received.\n");
				break;
			}

			buf_len = sizeof(buffer);
			result = uncompress(buffer, &buf_len, data+4, data_len-4);
			if (result != Z_OK)
			{
				const char* msg = "Error uncompressing data from server!";
				LOG_ERROR("%s Result: %i", msg, result);
				LOG_TO_CONSOLE(c_red2, msg);
			}
			else
			{
				uint8_t *new_data = buffer;
				while (buf_len >= 3)
				{
					uint16_t size = (uint16_t)new_data[1] + (((uint16_t)new_data[2]) << 8) + 2;
					if (size > buf_len)
					{
						LOG_ERROR("Incomplete packet in compressed data, packet size = %u, buffer length = %lu", size, buf_len);
						break;
					}
					process_message_from_server(new_data, size);
					new_data += size;
					buf_len -= size;
				}
			}
			break;
		}
#endif // PACKET_COMPRESSION
		default:
			LOG_WARNING("Unknown proxy command %u", data[0]);
			break;
	}
}
