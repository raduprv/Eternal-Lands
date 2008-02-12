#include <algorithm>
#include <cassert>
#include "asc.h"
#include "xml/xmlhelper.hpp"
#include "platform.h"
#include "multiplayer.h"
#include "client_serv.h"
#include "init.h"
#include "errors.h"
#include "sendvideoinfo.h"
#include "io/elfilewrapper.h"

namespace eternal_lands
{

	const int vendor_names_count = 5;

	const char* vendor_names[vendor_names_count] =
	{
		"ATI", "NVIDIA", "INTEL", "SIS", "TRIDENT"
	};

	const int opengl_versions_count = 8;

	const char* opengl_versions[opengl_versions_count] =
	{
		"1.0", "1.1", "1.2", "1.3", "1.4", "1.5", "2.0", "2.1"
	};

	typedef	char bit_set_96[12];

	static inline void set_bit(bit_set_96 bits, int bit)
	{
		int n, i;

		assert((bit >= 0) && (bit < 96));

		i = bit % 8;
		n = bit / 8;
		bits[n] |= 1 << i;
	}

	static inline int get_first_set_bit(int value)
	{
		int i;

		i = 0;

		while ((1 << i) < value)
		{
			i++;
		}

		return i;
	}

	static inline void parse_extention(const xmlNodePtr extension_element, std::string extensions, bit_set_96 bits)
	{
		xmlNodePtr cur_node;
		std::string name;
		int value;

		NODE_NAME_CHECK(extension_element, "extention");

		cur_node = get_node_element_children(extension_element);

		NODE_NAME_CHECK(cur_node, "name");

		name = get_value_from_node<std::string>(cur_node);

		cur_node = get_next_element_node(cur_node);

		NODE_NAME_CHECK(cur_node, "value");

		value = get_value_from_node<unsigned int>(cur_node);

		if (extensions.find(name) != extensions.npos)
		{
			set_bit(bits, value);
		}
	}

	static inline void parse_extentions(const xmlNodePtr extensions_element,
		const char* extensions, bit_set_96 bits)
	{
		xmlNodePtr cur_node;

		NODE_NAME_CHECK(extensions_element, "extentions");

		for (cur_node = get_node_element_children(extensions_element); cur_node != 0;
			cur_node = get_next_element_node(cur_node))
		{
			parse_extention(cur_node, extensions, bits);
		}
	}

	static inline int get_version_idx()
	{
		std::string str;
		int i;

		str = reinterpret_cast<const char *>(glGetString(GL_VERSION));

		for (i = 0; i < opengl_versions_count; i++)
		{
			if (str.find(opengl_versions[i]) == 0)
			{
				return i;
			}
		}

		return 0xFF;
	}

	static inline int get_vendor_idx()
	{
		std::string str;
		int i;

		str = reinterpret_cast<const char *>(glGetString(GL_VENDOR));

		std::transform(str.begin(), str.end(), str.begin(), toupper);

		for (i = 0; i < vendor_names_count; i++)
		{
			if (str.find(vendor_names[i]) == 0)
			{
				return i;
			}
		}

		return 0xFF;
	}

	extern "C" void send_video_info()
	{
		Uint8 data[33];
		bit_set_96 caps;
		xmlNodePtr root_element;
		xmlDoc *document;
		GLint i;

		if (video_info_sent == 0)
		{
			memset(caps, 0, sizeof(caps));

			if ((document = xmlReadFile("extentions.xml", 0, 0)) != NULL)
			{
				/*Get the root element node */
				root_element = xmlDocGetRootElement(document);

				if (root_element != 0)
				{
					try
					{
						parse_extentions(root_element,
							reinterpret_cast<const char *>
							(glGetString(GL_EXTENSIONS)), caps);
					}
					CATCH_AND_LOG_EXCEPTIONS

					data[0] = SEND_VIDEO_INFO;
					data[1] = get_vendor_idx();
					data[2] = get_version_idx();

					glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
					data[3] = get_first_set_bit(i);

					glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i);
					data[4] = i;

					glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &i);
					data[5] = i % 256;
					data[6] = i / 256;
			
					glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &i);
					data[7] = i % 256;
					data[8] = i / 256;

					memcpy(&data[9], caps, sizeof(caps));
					if (my_tcp_send(my_socket, data, sizeof(data)) <
						static_cast<int>(sizeof(data)))
					{
						LOG_ERROR("Error sending video info");
					}
					else
					{
						video_info_sent = 1;
					}
					my_tcp_flush(my_socket);
				}
			}
			xmlFree(document);
		}
	}

}

