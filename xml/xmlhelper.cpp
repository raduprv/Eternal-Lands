/*
 * xml helper
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 */

#include "xmlhelper.hpp"

void trim(std::string &str)
{
	std::string::size_type pos;

	pos = str.find_last_not_of(' ');

	if (pos != std::string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if (pos != std::string::npos)
		{
			str.erase(0, pos);
		}
	}
	else str.erase(str.begin(), str.end());
}

/* Convertes an utf8 string to an isolat1 string if possible or throws an exception otherwise */
std::string utf8_to_isolat1_str(const xmlChar* str)
{
	unsigned char* buffer;
	std::string conv_str;
	int buffer_size, ret, size;

	buffer_size = xmlUTF8Strlen(str) + 1;

	buffer = new unsigned char[buffer_size];

	memset(buffer, 0, buffer_size);

	size = strlen(reinterpret_cast<const char*>(str));

	ret = UTF8Toisolat1(buffer, &buffer_size, reinterpret_cast<const unsigned char*>(str), &size);

	conv_str = std::string(reinterpret_cast<char*>(buffer), buffer_size);

	delete[] buffer;

	if (ret == -2)
	{
		EXCEPTION("Can't convert utf8 string to isolat1");
	}
	else
	{
		if (ret < 0)
		{
			EXCEPTION("Unknow utf8 string conversion error");
		}
		else
		{
			conv_str.resize(buffer_size, '\0');
		}
	}

	return conv_str;
}

bool get_bool_from_str(const std::string &str)
{
	if ((str == "yes") || (str == "true") || (str == "1"))
	{
		return true;
	}
	else
	{
		if ((str == "no") || (str == "false") || (str == "0"))
		{
			return false;
		}
		else
		{
			WRONG_BOOL_VALUE_EXCEPTION(str.c_str());
		}
	}
}

bool get_bool_value_from_node(xmlNode *cur_node)
{
	std::string str;

	NODE_VALUE_ZERO_CHECK(cur_node);

	str = get_string_value_from_node(cur_node);

	return get_bool_from_str(str);
}

float get_float_value_from_node(xmlNode *cur_node)
{
	float f;

	NODE_VALUE_ZERO_CHECK(cur_node);

	std::istringstream str(get_string_value_from_node(cur_node));

	str >> f;

	return f;
}	

int get_int_value_from_node(xmlNode *cur_node)
{
	int i;

	NODE_VALUE_ZERO_CHECK(cur_node);

	std::istringstream str(get_string_value_from_node(cur_node));

	str >> i;

	return i;
}	

std::string get_string_value_from_node(xmlNode *cur_node)
{
	NODE_VALUE_ZERO_CHECK(cur_node);

	return utf8_to_isolat1_str(xmlNodeGetContent(cur_node));
}

void get_bool_values_from_node(xmlNode *cur_node, GLint* data, int size)
{
	std::string tmp;
	int i;

	NODE_VALUE_ZERO_CHECK(cur_node);

	std::istringstream strin(get_string_value_from_node(cur_node));

	i = 0;

	while (getline(strin, tmp, ','))
	{
		INT_RANGE_CHECK(0, size - 1, i);
		trim(tmp);
		if (get_bool_from_str(tmp))
		{
			data[i++] = 1;
		}
		else
		{
			data[i++] = 0;
		}
	}
	if (i < size)
	{
		EXCEPTION("Not enought boolean values! Fount: %d, expected: %d.", i, size);
	}
}

void get_float_values_from_node(xmlNode *cur_node, GLfloat* data, int size)
{
	std::string tmp;
	int i;

	NODE_VALUE_ZERO_CHECK(cur_node);

	std::istringstream strin(get_string_value_from_node(cur_node));

	i = 0;

	while (getline(strin, tmp, ','))
	{
		INT_RANGE_CHECK(0, size - 1, i);
		std::istringstream temp(tmp);
		temp >> data[i++];
	}
	if (i < size)
	{
		EXCEPTION("Not enought float values! Fount: %d, expected: %d.", i, size);
	}
}

void get_int_values_from_node(xmlNode *cur_node, GLint* data, int size)
{
	std::string tmp;
	int i;

	NODE_VALUE_ZERO_CHECK(cur_node);

	std::istringstream strin(get_string_value_from_node(cur_node));

	i = 0;

	while (getline(strin, tmp, ','))
	{
		INT_RANGE_CHECK(0, size - 1, i);
		std::istringstream temp(tmp);
		temp >> data[i++];
	}
	if (i < size)
	{
		EXCEPTION("Not enought integer values! Fount: %d, expected: %d.", i, size);
	}
}

xmlNode *get_next_element_node(const xmlNode *cur_node)
{
	xmlNode *next_node;

	NODE_ZERO_CHECK(cur_node);

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

xmlNode *get_node_element_children(const xmlNode *cur_node)
{
	xmlNode *next_node;

	NODE_ZERO_CHECK(cur_node);

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

bool is_node(const xmlNode *cur_node, const std::string &name)
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

std::string get_node_name(const xmlNode *cur_node)
{
	NODE_ZERO_CHECK(cur_node);

	return utf8_to_isolat1_str(cur_node->name);
}

