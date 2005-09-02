/*!
 * \file
 * \ingroup	misc
 * \brief	Miscellaneous functions used for file handling and string utilities.
 */
#ifndef __ASC_H__
#define __ASC_H__

/*!
 * Check if a character is a color character
 */
#define IS_COLOR(c) ((c) >= 127 + c_lbound && (c) <= 127 + c_ubound)

/*!
 * Check if a character is printable. In this context, that means
 * printable ascii, or non-ascii if it's not a color code
 */
#define IS_PRINT(c) ((c) >= 32 && !IS_COLOR (c))

/*!
 * A macro for the my_xmlstrncopy function that copies and converts an xml-string. Sets the length to 0, hence it will copy untill \\0 is reached.
 */
#define MY_XMLSTRCPY(d,s) my_xmlStrncopy(d,s,0)

/*!
 * \ingroup	misc_utils
 * \brief	Gets an integer after the given string
 *
 * 		The function finds \a needle in \a haystack and returns the integer value after the string given after it.
 *
 * \param	needle The string you wish to find
 * \param	haystack The pointer to the char array you wish to find the string from
 * \param	max_len The maximum length it should check
 * \retval Sint32	Returns the integer behind the string or -1 on failure.
 */
Sint32 get_integer_after_string (const char* needle, const char* haystack, Uint32 max_len);

/*!
 * \ingroup	misc_utils
 * \brief	Gets a float after the given string
 *
 * 		The function finds \a needle in \a haystack and returns the floating point value after it.
 *
 * \param	needle The string you wish to find
 * \param	haystack The pointer to the char array you want to search for the string in.
 * \param	max_len The maximum length it should check
 * \retval float	Returns the float after the string or -1.0f on failure.
 */
float get_float_after_string (const char* needle, const char* haystack, Uint32 max_len);

/*!
 * \ingroup	misc_utils
 * \brief	Gets the offset of a string in a char array
 *
 * 		The function gets the location of source_pointer in the dest_pointer char array, then returns the offset. The functio is not case-sensitive.
 *
 * \param	needle The string you wish to find
 * \param	haystack The char array you want to search for \a needle
 * \param	max_len The maximum length of \a haystack
 * \param	beginning Whether it should return the offset to the beginning of the string or the end of the string
 * \retval Sint32	Returns either the offset to the beginning of the string or to the end of the string - if the string was not found in the char array it returns -1 on failure.
 */
Sint32 get_string_occurance (const char *needle, const char *haystack, Uint32 max_len, Uint8 beginning);

/*!
 * \ingroup	misc_utils
 * \brief	The function copies the string from source to dest
 *
 * 		The function copies the string from source to destination, and put a terminating \\0
 *
 * \param	dest The destination char array
 * \param	source The source char array
 * \todo	We should just use strcpy instead...
 */
void my_strcp(Uint8 *dest,const Uint8 * source);

/*!
 * \ingroup	misc_utils
 * \brief	The function copies the string from source to dest, but no more than n characters
 *
 * 		The function copies the string from source to destination, but no more than n characters. It also puts an ending \\0
 *
 * \param	dest The destination char array
 * \param	source The source char array
 * \param	len The number of bytes you wish to copy
 */
void my_strncp (char *dest, const char *source, Uint32 len);

/*!
 * \ingroup	misc_utils
 * \brief	The function concencates the source string to the dest string
 *
 * 		The function concencates the source string to the dest string and sets a terminating \\0
 *
 * \param	dest The destination string
 * \param	source The source string
 * \todo	Err, use strcat instead...
 */
void my_strcat(Uint8 *dest,const Uint8 * source);

/*!
 * \ingroup	misc_utils
 * \brief	Compares n bytes of the 2 strings (case insensitive)
 *
 * 		The function compares n bytes of the 2 strings. It is not case sensitive
 *
 * \param	dest The first string
 * \param	src The second string
 * \param	len The number of bytes to compare
 * \retval Sint32	Returns 1 on match, 0 if the strings doesn't match.
 */
Sint32 my_strncompare(const Uint8 *dest, const Uint8 *src, Sint32 len);

/*!
 * \ingroup	misc_utils
 * \brief	Compares the 2 strings
 *
 * 		The function compares the 2 strings, calls my_strncompare. 
 *
 * \param	dest The first string
 * \param	src The second string
 * \retval Sint32 	Returns 1 on match, 0 if the strings doesn't match.
 * \sa my_strncompare
 */
Sint32 my_strcompare(const Uint8 *dest, const Uint8 *src);

/*!
 * \ingroup	misc_utils
 * \brief	Checks if len/2 characters of the string is uppercase
 *
 * 		Checks if len/2 characters of the string is uppercase
 *
 * \param	src The string to be checked
 * \param	len The length of characters you wish to check
 * \retval Sint32	Returns 1 if enough characters are uppercase, 0 if they are lowercase.
 */
Sint32 my_isupper(const Uint8 *src, int len);

/*!
 * \ingroup	misc_utils
 * \brief	Converts all characters in the string to lowercase
 *
 * 		Converts all characters in the string to lowercase
 *
 * \param	src The string to convert
 * \retval char*	Returns the src-pointer.
 */
char *my_tolower (char *src);

/*!
 * \ingroup	misc_utils
 * \brief	Splits up the char array into multiple character arrays
 *
 * 		Splits up the char array into multiple character arrays. The new arrays will have chars_per_line+3 bytes allocated. The char ** array will have a NULL pointer as the end pointer.
 *
 * \param	str The string to split
 * \param	chars_per_line The number of characters per line
 * \retval char**	Returns a char ** to the new array. You must free the memory yourself.
 */
char ** get_lines(char * str, int chars_per_line);

/*!
 * \ingroup	misc_utils
 * \brief	Goes through the file-name and replaces \\ with /
 *
 * 		Goes through the file-name and replaces \\ with /. Leaves the source intact, and copies the string to the destination.
 *
 * \param	dest The destination string
 * \param	src The source string
 * \param	max_len The maximum length
 * \retval Uint32	Returns the length of the string
 */
Uint32 clean_file_name (char *dest, const char *src, Uint32 max_len);

/*!
 * \ingroup	misc_utils
 * \brief	A simple implementation of the http-GET
 *
 * 		The function gets the given file from the server and writes it to the open file pointed to by fp.
 *
 * \param	server The server you're getting the file from
 * \param	path The path on the server
 * \param	fp The file you're saving the file to...
 */
void http_get_file(char *server, char *path, FILE *fp);

/*!
 * \ingroup	xml_utils
 * \brief	Finds the xml-attribute with the identifier p in the xmlNode and returns it as a floating point value
 *
 * 		Finds the xml-attribute with the identifier p in the xmlNode and returns it as a floating point value
 *
 * \param	n The xml-node you wish to search
 * \param	p The attribute name you wish to search for
 * \retval float	The floating point value of the string. Returns 0 on failure.
 */
float xmlGetFloat(xmlNode * n, xmlChar * p);

/*!
 * \ingroup	xml_utils
 * \brief	Finds the xml-attribute with the identifier p in the xmlNode and returns it as an integer value
 *
 * 		Finds the xml-attribute with the identifier p in the xmlNode and returns it as an integer value
 *
 * \param	n The node you wish to search
 * \param	p The attribute name you wish to search for
 * \retval int	The integer value of the string. Returns 0 on failure.
 */
int xmlGetInt(xmlNode *n, xmlChar *p);

/*!
 * \ingroup	xml_utils
 * \brief	Copies and converts the UTF8-string pointed to by src into the destination.
 *
 * 		Copies and converts the UTF8-string pointed to by src into the destination. It will max copy n characters, but if n is 0 it will copy the entire string. 
 * 		The function allocates appropriate buffer sizes using strlen and xmlUTF8Strlen. The src is copied to the in-buffer, then the in-buffer is converted using iconv() to iso-8859-1 and the converted string is put in the outbuffer.
 * 		The main case is where the pointer pointed to by dest is non-NULL. In that case it will copy the content of the out-buffer to the *dest. Next it will free() the allocated buffers.
 * 		A second case is where the pointer pointed to by dest is NULL - here it will set the pointer to the out-buffer and only free() the in-buffer.
 *
 * \param	dest A pointer to the destination character array pointer
 * \param	src The source string
 * \param	len The maximum length of chars that will be copied
 * \retval int	Returns the number of characters that have been copied, or -1 on failure.
 * \sa my_UTF8Toisolat1
 */
int my_xmlStrncopy(char ** dest, const char * src, int len);

#ifdef WINDOWS
#ifdef _MSC_VER
/*!
 * \ingroup	misc_utils
 * \brief	An snprintf replacement for MSVC
 *
 *	 The moronic _snprintf in MSVC doesn't necessarily terminate the
 *	string with a NULL byte. This function should at least terminate the 
 *	string, but does not return the number of bytes that would have been 
 *	written if the buffer was large enough, like gcc does. Instead, it 
 *	returns a negative value, indicating error. This probably won't be a
 *	problem, as the return value is mostly ignored.
 *
 * \param	buffer A pointer to the destination character array pointer
 * \param	size The size of \a buffer
 * \param	format The format string
 * \retval int	Returns the number of characters that have been written, or 
 *		a negative value on failure or overflow.
 */
int sane_snprintf (char *buffer, size_t size, const char *format, ...);
#endif 
#endif

#endif
