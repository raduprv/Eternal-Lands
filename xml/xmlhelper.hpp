/*
 * xml helper
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 */

#ifndef	_XMLHELPER_HPP_
#define	_XMLHELPER_HPP_

#include <string>
#include <vector>
#include "../exceptions/extendedexception.hpp"
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include "../platform.h"
#include <cassert>

namespace eternal_lands
{

	typedef	std::vector<float> float_vector;
	typedef	std::vector<int> int_vector;
	typedef	std::vector<bool> bool_vector;

	/**
	 * @brief Checks if the node has the given name.
	 *
	 * Checks if the node has the given name and returns true if so, else false.
	 *
	 * @param cur_node The xml node to check.
	 * @param name The expected name.
	 * @return True, if the node has the given name, else false.
	 */
	bool is_node(const xmlNodePtr cur_node, const std::string &name);

	/**
	 * @brief Checks if the node has the given name.
	 *
	 * Checks if the node has the given name and returns true if so, else false.
	 *
	 * @param reader The xml reader that points to the node to check.
	 * @param name The expected name.
	 * @return True, if the node has the given name, else false.
	 */
	bool is_node(xmlTextReaderPtr reader, const std::string &name);

	/**
	 * @brief Gets the name of the node.
	 * 
	 * Gets the name of the node. Does also checking if the node is zero and throws exception if so.
	 * The string encoding is isolat1. If the name can't get converted, an exception is thrown.
	 * @param cur_node The node from where to get the name.
	 * @return Retruns node name as isolat1 string.
	 */
	std::string get_node_name(const xmlNodePtr cur_node);

	/**
	 * @brief Generic template to extrac a value from a string.
	 * 
	 */
	template <typename T>
	inline T get_value_from_str(const char* str)
	{
		T value;

		std::istringstream sstr(str);

		sstr >> value;

		return value;
	}

	/**
	 * @brief Specialized template to extrac a boolean from a string.
	 * 
	 */
	template <>
	inline bool get_value_from_str<bool>(const char* str)
	{
		std::string s;

		s = str;
		if ((s == "true") || (s == "1"))
		{
			return true;
		}
		else
		{
			if ((s == "false") || (s == "0"))
			{
				return false;
			}
			else
			{
				EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found,
					"Expected boolean value 'true', '1', 'false' or '0', but"
					<< " found value '" << s << "'");
			}
		}
	}

	/**
	 * @brief Specialized template to extrac a string from a string
	 * 
	 */
	template <>
	inline std::string get_value_from_str<std::string>(const char* str)
	{
		return std::string(str);
	}

	/**
	 * @brief Generic template to extrac a value from a node.
	 * 
	 */
	template <typename T>
	inline T get_value_from_node(xmlNodePtr cur_node)
	{
		if (cur_node == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Node is zero");
		}
		else
		{
			return get_value_from_str<T>(reinterpret_cast<const char*>(
				xmlNodeGetContent(cur_node)));
		}
	}

	/**
	 * @brief Generic template to extrac values from a node.
	 * 
	 */
	template <typename T, typename V>
	inline void get_values_from_node(xmlNodePtr cur_node, V &values)
	{
		if (cur_node == 0)
		{
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Node is zero");
		}
		else
		{
			std::string tmp;

			std::istringstream strin(reinterpret_cast<const char*>(
				xmlNodeGetContent(cur_node)));

			while (getline(strin, tmp, ' '))
			{
				values.push_back(get_value_from_str<T>(tmp.c_str()));
			}
		}
	}

	xmlNodePtr get_next_element_node(const xmlNodePtr cur_node);

	xmlNodePtr get_node_element_children(const xmlNodePtr cur_node);

	void reader_read(xmlTextReaderPtr reader, bool ignore_whitespace = true);

	bool reader_read_next(xmlTextReaderPtr reader);

#define	NODE_NAME_CHECK(node, name)	\
	do	\
	{	\
		if (node == 0)	\
		{	\
			EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, "Expected node "	\
				<< name << ", but node is zero");	\
		}	\
		else	\
		{	\
			if (!is_node(node, name))	\
			{	\
				EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found,	\
					"Expected node " << name << ", but node name is "	\
					<< get_node_name(node));	\
			}	\
		}	\
	}	\
	while (false)

}

#endif	// _XMLHELPER_HPP_
