/*
 * xml helper
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 */

#include "xmlhelper.hpp"

namespace eternal_lands
{

	xmlNodePtr get_next_element_node(const xmlNodePtr cur_node)
	{
		if (cur_node == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Node is zero");
		}
		else
		{
			xmlNodePtr next_node;

			next_node = cur_node->next;

			while (next_node)
			{
				if (next_node->type == XML_ELEMENT_NODE)
				{
					return next_node;
				}
				else
				{
					next_node = next_node->next;
				}
			}
			return 0;
		}
	}

	xmlNodePtr get_node_element_children(const xmlNodePtr cur_node)
	{
		if (cur_node == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Node is zero");
		}
		else
		{
			xmlNodePtr next_node;

			next_node = cur_node->children;

			while (next_node)
			{
				if (next_node->type == XML_ELEMENT_NODE)
				{
					return next_node;
				}
				else
				{
					next_node = next_node->next;
				}
			}
			return 0;
		}
	}

	bool is_node(const xmlNodePtr cur_node, const std::string &name)
	{
		if (cur_node != 0)
		{
			return get_node_name(cur_node) == name;
		}
		else
		{
			return false;
		}
	}

	bool is_node(xmlTextReaderPtr reader, const std::string &name)
	{
		if (reader != 0)
		{
			return xmlStrcmp(reinterpret_cast<const xmlChar*>(name.c_str()), xmlTextReaderConstName(reader)) == 0;
		}
		else
		{
			return false;
		}
	}

	std::string get_node_name(const xmlNodePtr cur_node)
	{
		if (cur_node == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Node is zero");
		}
		else
		{
			return reinterpret_cast<const char*>(cur_node->name);
		}
	}

	static inline void reader_read_helper(xmlTextReaderPtr reader)
	{
		if (reader == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_internal_error, "Reader is zero");
		}
		else
		{
			switch (xmlTextReaderRead(reader))
			{
				case 0:
					EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found,
						"Nothing to parse any more");
				case 1:
					break;
				default:
					EXTENDED_EXCEPTION(ExtendedException::ec_io_error,
						"XML parser error");
			}
		}
	}

	void reader_read(xmlTextReaderPtr reader, bool ignore_whitespace)
	{
		reader_read_helper(reader);

		if (ignore_whitespace)
		{
			while (xmlTextReaderNodeType(reader) ==
				XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
			{
				reader_read_helper(reader);
			}
		}
	}	

	bool reader_read_next(xmlTextReaderPtr reader)
	{
		int ret;

		ret = xmlTextReaderNext(reader);

		if (ret < 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_io_error, "XML parser error");
		}

		return ret == 1;
	}

}

