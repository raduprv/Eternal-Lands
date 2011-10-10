#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>
#include <errno.h>
#include "asc.h"
#include "errors.h"
#include "md5.h"
#include "io/elfilewrapper.h"
#ifdef MAP_EDITOR
# include "map_editor/misc.h"
#else
# include "misc.h"
#endif //MAP_EDITOR

/* NOTE: This file contains implementations of the following, currently unused and commented functions:
 *          Look at the end of the file.
 *
 * Sint32 get_string_after_string(const Uint8*, const Uint8*, Sint32, Uint*, int);
 */

#ifndef LINUX
int my_UTF8Toisolat1(char **dest, size_t * lu, const char **src, size_t * len);
#else
int my_UTF8Toisolat1(char **dest, size_t * lu, char **src, size_t * len);
#endif

// find the first occurance of needle in haystack, and return the distance to 
// that string. If beggining is 1, it returns the offset to the beginning of 
// the string otherwise it returns the offset to the end of the string. Needle
// must be null-terminated. hyastack need not be, but must be at least max_len
// bytes long
Sint32 get_string_occurance (const char* needle, const char* haystack, const Uint32 max_len, const char beginning)
{
	const Uint32 n_len = strlen(needle);
	Uint32 istart, i;
	Uint32 search_len;

	if (max_len < n_len) {
		return -1;
	}

	for (istart = 0, search_len = max_len - n_len; istart <= search_len; istart++)
	{
		for (i = 0; i < n_len; i++)
		{
			if (tolower(haystack[istart+i]) != tolower(needle[i])) {
				break;
			}
		}
		if (i >= n_len)
		{
			// We found the string. return the beginning if asked
			if (beginning) {
				return istart;
			}
			// return the end of the string occurence, but skip
			// space and equal signs
			while ((istart+i < max_len) && (haystack[istart+i] == ' ' || haystack[istart+i] == '=')) {
				i++;
			}
			return istart+i;
		}
	}
	return -1;
}

#ifndef FASTER_MAP_LOAD
// This function returns an integer, after the needle in the haystack 
// string. If the string is not found, after max_len, the function returns -1.
// The function is NOT case sensitive
Sint32 get_integer_after_string (const char *needle, const char *haystack, Uint32 max_len)
{
	Sint32 n_end = get_string_occurance (needle, haystack, max_len, 0);
	Uint32 istart;
	
	if (n_end < 0)
	{
		// needle not found
		return -1;
	}
	
	istart = n_end;
	while (istart < max_len)
	{
		if (haystack[istart] == '\n')
			// no integer on this line
			return -1;
		if (isdigit (haystack[istart]) || haystack[istart] == '+' || haystack[istart] == '-'){
			// we've probably found a number
			//return atoi (&haystack[istart]);
			char temp[1<<sizeof(int)];	//Wasteful, but it will reserve enough space for MAX_INT as a string. If we change to atol or similar, use sizeof(long) instead
			int len = min2i(max_len-istart, (1<<sizeof(int))-1);
			memcpy(temp, &haystack[istart], len-1);
			temp[len] = '\0';
			return atoi (temp);
		}
		istart++;
	}
	
	// no integer after needle
	return -1;
}

// This function returns a float, after the source string in the destination 
// string. If the string is not found, after max_len, the function returns 
// -1.0f. The function is NOT case sensitive
float get_float_after_string (const char *needle, const char *haystack, Uint32 max_len)
{
	Sint32 n_end = get_string_occurance (needle, haystack, max_len, 0);
	Uint32 istart;
	
	if (n_end < 0)
	{
		// needle not found
		return -1.0f;
	}
	
	istart = n_end;
	while (istart < max_len)
	{
		if (haystack[istart] == '\n')
			// no number on this line
			return -1.0f;
		if (isdigit (haystack[istart]) || haystack[istart] == '+' || haystack[istart] == '-' || haystack[istart] == '.'){
			// we've probably found a number
			//return atof (&haystack[istart]);
			//char temp[max_len-istart+1];	//Wasteful, if the float doesn't go to the end of the line, but it will reserve enough space
			char temp[200];	//It'd be better not to use an arbitrary constant, but we can't use run-time size on compilers like MSVC
			memcpy(temp, &haystack[istart], min2i(max_len-istart, sizeof(temp)));
			temp[min2i(max_len-istart, sizeof(temp))] = '\0';
			return atof (temp);
		}
		istart++;
	}
	
	// no number after needle
	return -1.0f;
}
#endif // FASTER_MAP_LOAD

char* safe_strncpy(char *dest, const char * source, const size_t len)
{
	if (len > 0)
	{
		strncpy(dest, source, len - 1);
		dest[len - 1] = '\0';
	}
	return dest;
}

char* safe_strncpy2(char *dest, const char * source, const size_t dest_len, const size_t src_len)
{
	if (dest_len > 0)
	{
		if (src_len >= dest_len)
		{
			strncpy(dest, source, dest_len - 1);
			dest[dest_len - 1] = '\0';
		}
		else
		{
			strncpy(dest, source, src_len);
			dest[src_len] = '\0';
		}
	}
	return dest;
}

int safe_snprintf(char *dest, const size_t len, const char* format, ...)
{
	int ret;
	if (len > 0)
	{
		va_list ap;
		va_start(ap, format);
#ifdef __MINGW32__
		ret = vsnprintf(dest, len, format, ap);
#else
 #if defined(WINDOWS) && (defined(__MINGW32__) || defined(_MSC_VER))
		ret = _vsnprintf(dest, len, format, ap);
 #else
		ret = vsnprintf(dest, len, format, ap);
 #endif
#endif
		va_end(ap);
		dest[len - 1] = '\0';
		if ((ret < 0) || (ret >= len))
			return len;
		return ret;
	}
	return 0;
}

char* safe_strcat (char* dest, const char* src, size_t len)
{
	size_t start_pos = strlen (dest);
	if (start_pos < len)
		safe_strncpy (dest+start_pos, src, len-start_pos);
	return dest;
}

char* safe_strcasestr (const char* haystack, size_t haystack_len, const char* needle, size_t needle_len)
{
	if (haystack_len >= needle_len)
	{
		const char* res;
		size_t istart;
		size_t imax = haystack_len - needle_len;

		for (istart = 0, res = haystack; istart <= imax && *res; istart++, res++)
		{
			if (strncasecmp (res, needle, needle_len) == 0)
				return (char*) res;
		}
	}

	return NULL;	
}

void my_strcp(char *dest,const char * source)
{
	while(*source)
	{
		*dest++=*source++;
	}
	*dest='\0';
}

void my_strncp (char *dest, const char *source, size_t len)
{
	while (*source != '\0' && --len > 0)
	{
		*dest++ = *source++;
	}
	*dest = '\0';
}

void my_strcat(char *dest,const char * source)
{
	int i,l,dl;
	
	l=strlen(source);
	dl=strlen(dest);
	for(i=0;i<l;i++)dest[dl+i]=source[i];
	dest[dl+i]='\0';
}

Sint32 my_strncompare(const char *dest, const char *src, Sint32 len)
{
	int i;
	char ch1,ch2;

	for(i=0;i<len;i++)
		{
			ch1=src[i];
			ch2=dest[i];
			if(ch1>=65 && ch1<=90)ch1+=32;//make lowercase
			if(ch2>=65 && ch2<=90)ch2+=32;//make lowercase
			if(ch1!=ch2)break;
		}
	if(i!=len)return 0;
	else return 1;
}

Sint32 my_strcompare(const char *dest, const char *src)
{
	Uint32 len;

	len=strlen(dest);
	if(len!=strlen(src))return 0;
	return(my_strncompare(dest, src, len));
}

// is this string more then one character and all alpha in it are CAPS?
Sint32 my_isupper(const char *src, int len)
{
	int alpha=0;
	if (len < 0)	len=strlen(src);
	if(!src || !src[0] || !src[1] || !src[2] || len == 0) return 0;
	while(*src && len > 0)
		{
            if(isalpha((unsigned char)*src)) alpha++;
            if((isdigit((unsigned char)*src)&&alpha<len/2) || *src != toupper(*src)) return 0;    //at least one lower			
            src++;
			len--;
		}
	return 1;	// is all upper or all num
}

char *my_tolower (char *src)
{
	char *dest = src;

	if (dest == NULL || dest[0] == '\0')
		return dest;
	while (*src)
	{
		*src = tolower (*src);
		src++;
	}
	
	return dest;
}

/*Wraps the lines*/

char ** get_lines(char * str, int chars_per_line)
{
	char ** my_str=NULL;
	char * cur=NULL;
	int lines=0;
	int i=0;
	if(str){
		for(lines = 0; *str; lines++) {
			my_str=(char **)realloc(my_str,(lines+2)*sizeof(char *));
			cur=my_str[lines]=(char*)calloc(chars_per_line+3,sizeof(char));
		
			for(i = 0; i < chars_per_line && str[i]; i++){
				if(str[i] == '\r') i++;
				if (str[i] == '\n'){
					i++;
					break;
				}
				cur[i]=str[i];
			}
			if(i >= chars_per_line){//Wrap it
				//go back to the last space
				while(i){
					if(str[i]=='/' || str[i]=='-' || str[i]=='?' || str[i]=='!' || str[i]==' ' || str[i]=='\n' || str[i]=='\r') break;
					i--;
				}
				if(i){
					i++;
					if(str[i]==' ')str++;
				} else {
					//Force a break then...
					i=chars_per_line;
				}
			}
			str+=i;
			cur[i]=0;
		}
		if(my_str)my_str[lines]=NULL;//Used to get the bounds for displaying each line
	}
	return my_str;
}

// File utilities
Uint32 clean_file_name (char *dest, const char *src, Uint32 max_len)
{
	char *dptr, *dend = dest + (max_len-1);
	const char *sptr;

	for (dptr = dest, sptr = src; dptr < dend && *sptr; dptr++, sptr++)
		*dptr = *sptr == '\\' ? '/' : tolower(*sptr);
	// always place a null at the end
	*dptr = '\0';

	return dptr-dest;
}

/*XML*/

float xmlGetFloat(xmlNode * n, xmlChar * c)
{
	char * t=(char*)xmlGetProp(n,c);
	float f=t?atof(t):0.0f;
	xmlFree(t);
	return f;
}

int xmlGetInt(xmlNode *n, xmlChar *c)
{
	char *t=(char*)xmlGetProp(n,c);
	int i=t?atoi(t):0;
	xmlFree(t);
	return i;
}

int my_xmlStrncopy(char ** out, const char * in, int len)
{
	if(in) {
		size_t lin=0;
		size_t lout=0;
		int l1=0;
		int l2=0;
		int retval=1;
		char *inbuf;
		char *inbuf2;
		char *outbuf;
		char *outbuf2;
		
		lin=strlen(in);
		l2=xmlUTF8Strlen((xmlChar*)in);
		
		if(l2<0) lout=l1;
		else if (len>0 && len<l2) lout=len;
		else lout=l2;
		
		inbuf=inbuf2=(char *)malloc((lin+1)*sizeof(char));
		outbuf=outbuf2=(char *)malloc((lout+1)*sizeof(char));

		memcpy(inbuf,in,lin);

		l1=lin;
		l2=lout;

#ifndef LINUX
		if(my_UTF8Toisolat1(&outbuf2,&lout,(const char **)&inbuf2,&lin)<0) {
#else
		if(my_UTF8Toisolat1(&outbuf2,&lout,&inbuf2,&lin)<0) {
#endif
			retval=-1;
		}

		free(inbuf);

		outbuf[l2]=0;

		if(*out) {
			memcpy(*out,outbuf,l2+1);
			free(outbuf);
		} else {
			*out=outbuf;
		}

		return retval<0?-1:l2;
	} else return -1;
}

#ifndef LINUX
int my_UTF8Toisolat1(char **dest, size_t * lu, const char **src, size_t * l)
#else
int my_UTF8Toisolat1(char **dest, size_t * lu, char **src, size_t * l)
#endif
{
	iconv_t t=iconv_open("ISO_8859-1","UTF-8");

	iconv(t, src, l, dest, lu);

	iconv_close(t);
	return 1;
}

/* return true if digest calculated */
int get_file_digest(const char * filename, Uint8 digest[16])
{
	MD5 md5;
	el_file_ptr file = NULL;

	file = el_open(filename);

	memset (digest, 0, 16);

	if (file == NULL)
	{
		LOG_ERROR("MD5Digest: Unable to open %s (%d)", filename, errno);
		return 0;
	}
	
	if (el_get_pointer(file) == NULL)
	{
		el_close(file);
		return 0;
	}

	MD5Open(&md5);
	MD5Digest(&md5, el_get_pointer(file), el_get_size(file));
	MD5Close(&md5, digest);

	el_close(file);
	
	return 1;
}

/* currently UNUSED
//find & copy a string into memory
//return the length or -1 if not found
Sint32 get_string_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer, Sint32 max_len, Uint8 *value, Sint32 value_len)
{
	int i;
	int loc=get_string_occurance(source_pointer, dest_pointer, max_len, 0);

	if (loc < 0)
		{
			return -1;
		}
	// now copy the string
	for(i=0;i<value_len-1;i++)
		{
			Uint8 ch;
			ch=dest_pointer[loc+i];
			if(ch==0x0a || ch==0x0d)break;
  			value[i]=ch;
		}
	value[i]=0;	// always place a NULL

	return(i);
}

void get_string_digest(const Uint8 * string, Uint8 digest[16])
{
	MD5 md5;
	MD5Open(&md5);
	MD5Digest(&md5, string, strlen(string));
	MD5Close(&md5, digest);
}
*/

int find_description_index (const dict_elem dict[], const char *elem, const char *desc) {
	int idx = 0;
	const char *key;

	while ((key = dict[idx].desc) != NULL) {
		if (strcasecmp (key, elem) == 0)
			return dict[idx].index;
		idx++;
	}

	LOG_ERROR("Unknown %s \"%s\"\n", desc, elem);
	return -1;
}

void get_string_value(char *buf, size_t maxlen, const xmlNode *node)
{
	if (!node)
	{
		LOG_ERROR("Node is null!");
		buf[0] = '\0';
		return;
	}

	if (!node->children)
		buf[0] = '\0';
	else
		my_strncp(buf, (const char*)node->children->content, maxlen);
}

void get_item_string_value(char *buf, size_t maxlen, const xmlNode *item,
	const unsigned char *name)
{
	const xmlNode *node;

	if (!item)
	{
		LOG_ERROR("Item is null!");
		buf[0] = '\0';
		return;
	}

	// look for this entry in the children
	for (node = item->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE
			&& xmlStrcasecmp(node->name, name) == 0)
		{
			get_string_value(buf, maxlen, node);
			return;
		}
	}
}

int get_bool_value(const xmlNode *node)
{
	const xmlChar *tval;

	if (!node)
	{
		LOG_ERROR("Node is null!");
		return 0;
	}

	if (!node->children)
		return 0;

	tval = node->children->content;
	return (xmlStrcasecmp(tval, (xmlChar*)"yes") == 0) ||
		(xmlStrcasecmp(tval, (xmlChar*)"true") == 0) ||
		(xmlStrcasecmp(tval, (xmlChar*)"1") == 0);
}

int get_int_value(const xmlNode *node)
{
	if (!node)
	{
		LOG_ERROR("Node is null!");
		return 0;
	}

	if (!node->children)
		return 0;

	return atoi((const char*)node->children->content);
}

double get_float_value(const xmlNode *node)
{
	if (!node)
	{
		LOG_ERROR("Node is null!");
		return 0.0;
	}

	if (!node->children)
		return 0.0;

	return atof((const char*)node->children->content);
}

int get_int_property(const xmlNode *node, const char *prop)
{
	const xmlAttr *attr;

	if (!node)
	{
		LOG_ERROR("Node is null!");
		return 0;
	}

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE &&
			xmlStrcasecmp(attr->name, (const xmlChar*)prop) == 0)
		{
			return atoi((const char*)attr->children->content);
		}
	}

	return -1;
}

int get_property(const xmlNode *node, const char *prop, const char *desc,
	const dict_elem dict[])
{
	const xmlAttr *attr;

	if (!node)
	{
		LOG_ERROR("Node is null!");
		return 0;
	}

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE &&
			xmlStrcasecmp (attr->name, (const xmlChar*)prop) == 0)
		{
			return find_description_index(dict,
				(const char*)attr->children->content, desc);
		}
	}

	LOG_ERROR("Unable to find property %s in node %s\n", prop, node->name);
	return -1;
}

const char *get_string_property(const xmlNode *node, const char *prop)
{
	const xmlAttr *attr;

	if (node == NULL)
	{
		LOG_ERROR("Node is null!");
		return "";
	}

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE &&
			xmlStrcasecmp (attr->name, (xmlChar *)prop) == 0)
		{
			return (const char*)attr->children->content;
		}
	}

#ifdef	DEBUG_XML
	// don't normally report this, or optional properties will report errors
	LOG_ERROR("Unable to find property %s in node %s\n", prop, node->name);
#endif	//DEBUG_XML
	return "";
}

void append_char(char** s, char c, int* len, int* max_len)
{
	if (*len >= *max_len)
	{
		*s = (char*) realloc(*s, *max_len + APPEND_CHAR_BLOCK);
		*max_len += APPEND_CHAR_BLOCK;
	}
	(*s)[(*len)++] = c;
}

xmlChar* toUTF8 (const char* str, int len)
{
	int out_size = 2*len;
	int out_len;
	xmlChar* out = calloc (out_size, sizeof (xmlChar));

	while (1)
	{
		int in_len = len;
		out_len = out_size;

		if (isolat1ToUTF8 (out, &out_len, BAD_CAST str, &in_len) < 0)
		{
			// Conversion error
			free (out);
			return NULL;
		}
		if (in_len >= len)
			break;

		out_size *= 2;
		out = realloc (out, out_size * sizeof (xmlChar));
	}

	if (out_len >= out_size)
		// drats, no space to store a terminator
		out = realloc (out, (out_size + 1) * sizeof (xmlChar));
	out[out_len] = '\0';

	return out;
}

char* fromUTF8 (const xmlChar* str, int len)
{
	int out_size = len+1;
	int out_len = out_size;
	int in_len = len;
	char* out = calloc (out_size, 1);

	if (UTF8Toisolat1 (BAD_CAST out, &out_len, str, &in_len) < 0)
	{
		// Conversion error
		free (out);
		return NULL;
	}

	out[out_len] = '\0';

	return out;
}


/* whether you pass in a NULL pointer or your own allocated memory for
 * out_str, you need to free the memory yourself */
char *substitute_char_with_string(const char *str, char **out_str, char to_sub, const char* with_sub)
{
	int amp_count = 0;
	const char *start_ptr;
	char *end_ptr;
	int out_len = 0;
	size_t alloc_len = 0;

	for (start_ptr = str; (start_ptr = strchr(start_ptr, to_sub)) != NULL; start_ptr++)
		amp_count++;

	alloc_len = strlen(str) + amp_count*(strlen(with_sub)-1) + 1;
	*out_str = (char *)realloc(*out_str, alloc_len);
	**out_str = '\0';
	
	for (start_ptr = str; (end_ptr = strchr(start_ptr, to_sub)) != NULL; )
	{
		while (start_ptr < end_ptr)
			(*out_str)[out_len++] = *start_ptr++;
		(*out_str)[out_len] = '\0';

		safe_strcat(*out_str, with_sub, alloc_len);
		out_len = strlen(*out_str);
		start_ptr++;
	}
	safe_strcat(*out_str, start_ptr, alloc_len);
	return *out_str;
}


/* Return a copy of source truncated to be no longer than max_len_x including the append_str on the end. */
char *truncated_string(char *dest, const char *source, size_t dest_max_len, const char *append_str, float max_len_x, float font_ratio)
{
	float string_width = 0;
	size_t dest_len = 0;
	float append_len_x = get_string_width((unsigned char*)append_str) * font_ratio;
	char *dest_p = dest;
	
	while ((*source != '\0') && (dest_len < dest_max_len-1))
	{
		float char_width = get_char_width(*source) * font_ratio;
		if ((string_width + char_width) > (max_len_x - append_len_x))
			break;
		*dest_p++ = *source++;
		dest_len++;
		string_width += char_width;
	}
	
	while ((*append_str != '\0') && (dest_len < dest_max_len-1))
	{
		*dest_p++ = *append_str++;
		dest_len++;
	}

	*dest_p = '\0';
	return dest;
}
