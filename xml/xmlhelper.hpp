/*
 * xml helper
 * Copyright (C) Daniel Jungmann 2007 <dsj@gmx.net>
 * 
 */

#ifndef	_XMLHELPER_HPP_
#define	_XMLHELPER_HPP_

#include <string>
#include "../global.h"
#include "../platform.h"
#include "../exceptions/extendedexception.hpp"
#include <libxml/parser.h>

/**
 * @brief Gets boolean value from node.
 * 
 * Gets the value from the node as boolean("true", "yes" and "1" are true, "false", "no" and "0"
 * are false). Does also checking if the node is zero or value is not a valid boolean and throws
 * an exception if so.
 * @param cur_node The node from where to get the value.
 * @return Retruns node value as boolean.
 */
bool get_bool_value_from_node(xmlNode *cur_node);

/**
 * @brief Gets float value from node.
 * 
 * Gets the value from the node as float. Does also checking if the node is zero or value is not a
 * valid float and throws an exception if so.
 * @param cur_node The node from where to get the value.
 * @return Retruns node value as float.
 */
float get_float_value_from_node(xmlNode *cur_node);

/**
 * @brief Gets int value from node.
 * 
 * Gets the value from the node as int. Does also checking if the node is zero or value is not a
 * valid int and throws an exception if so.
 * @param cur_node The node from where to get the value.
 * @return Retruns node value as int.
 */
int get_int_value_from_node(xmlNode *cur_node);

/**
 * @brief Gets string value from node.
 * 
 * Gets the string value from the node. Does also checking if the node is zero and throws exception
 * if so. The string encoding is isolat1. If the value can't get converted, an exception is thrown.
 * @param cur_node The node from where to get the value.
 * @return Retruns node values as isolat1 string.
 */
std::string get_string_value_from_node(xmlNode *cur_node);

/**
 * @brief Gets boolean values from node.
 * 
 * Gets the values from the node as booleans("true", "yes" and "1" are true, "false", "no" and "0"
 * are false). Does also checking if the node is zero or values are not valid booleans and throws
 * an exception if so. The values must be separeted with ','. If not enought or too much values are
 * found, an exception is thrown.
 * @param cur_node The node from where to get the value.
 * @param data Memory where to store the values.
 * @param size Number of values to expect.
 */
void get_bool_values_from_node(xmlNode *cur_node, GLint* data, int size);

/**
 * @brief Gets float values from node.
 * 
 * Gets the value from the node as floats. Does also checking if the node is zero or values are not
 * valid floats and throws an exception if so. The values must be separeted with ','. If not
 * enought or too much values are found, an exception is thrown.
 * @param cur_node The node from where to get the value.
 * @param data Memory where to store the values.
 * @param size Number of values to expect.
 */
void get_float_values_from_node(xmlNode *cur_node, GLfloat* data, int size);

/**
 * @brief Gets int values from node.
 * 
 * Gets the values from the node as ints. Does also checking if the node is zero or values are not
 * valid ints and throws an exception if so. The values must be separeted with ','. If not enought
 * or too much values are found, an exception is thrown.
 * @param cur_node The node from where to get the value.
 * @param data Memory where to store the values.
 * @param size Number of values to expect.
 */
void get_int_values_from_node(xmlNode *cur_node, GLint* data, int size);

xmlNode *get_next_element_node(const xmlNode *cur_node);

xmlNode *get_node_element_children(const xmlNode *cur_node);

/**
 * @brief Checks if the node has the given name.
 *
 * Checks if the node has the given name and returns true if so, else false.
 *
 * @param cur_node The xml node to check use.
 * @param name The expected name.
 * @return True, if the node has the given name, else false.
 */
bool is_node(const xmlNode *cur_node, const std::string &name);

/**
 * @brief Gets the name of the node.
 * 
 * Gets the name of the node. Does also checking if the node is zero and throws exception if so.
 * The string encoding is isolat1. If the name can't get converted, an exception is thrown.
 * @param cur_node The node from where to get the name.
 * @return Retruns node name as isolat1 string.
 */
std::string get_node_name(const xmlNode *cur_node);

#define NODE_ZERO_CHECK(node)	\
do	\
{	\
	if ((node) == 0)	\
	{	\
		EXCEPTION("Node is zero!");	\
	}	\
}	\
while (false)

#define NODE_NAME_CHECK(node, node_name)	\
do	\
{	\
	std::string str;	\
	\
	NODE_ZERO_CHECK((node));	\
	\
	str = get_node_name((node));	\
	\
	if (str != (node_name))	\
	{	\
		EXCEPTION("Node name is '%s', but should be '%s'.", str.c_str(), node_name);	\
	}	\
} while (false)

#define NODE_VALUE_ZERO_CHECK(node)	\
do	\
{	\
	NODE_ZERO_CHECK((node));	\
	\
	if (xmlNodeGetContent((node)) == 0)	\
	{	\
		EXCEPTION("Value of node '%s' is zero.", get_node_name(node).c_str());	\
	}	\
} while (false)

#define WRONG_BOOL_VALUE_EXCEPTION(value) INVALID_STRING_VALUE_EXCEPTION(value, "bool")

#endif	// _XMLHELPER_HPP_
