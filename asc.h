#ifndef __ASC_H__
#define __ASC_H__

#define my_xmlStrcpy(d,s) my_xmlstrncopy(d,s,0)
#define my_xmlStrncpy(d,s,l) my_xmlstrncopy(d,s,l)

//proto
Sint32 get_integer_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer, 
							 Sint32 max_len);
float get_float_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer, 
							 Sint32 max_len);
Sint32 get_string_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer, 
						 Sint32 max_len, Uint8 * value, int value_len);
Sint32 get_string_occurance(const Uint8 * source_pointer, const Uint8 * dest_pointer, 
						 Sint32 max_len,Uint8 begining);
void my_strcp(Uint8 *dest,const Uint8 * source);
void my_strncp(Uint8 *dest,const Uint8 * source,Sint32 len);
void my_strcat(Uint8 *dest,const Uint8 * source);
Sint32 my_strncompare(const Uint8 *dest, const Uint8 *src, Sint32 len);
Sint32 my_strcompare(const Uint8 *dest, const Uint8 *src);
Sint32 my_isupper(const Uint8 *src, int len);
Uint8 *my_tolower(Uint8 *src);

char ** get_lines(char * str, int chars_per_line);

Uint32	clean_file_name(Uint8 *dest, const Uint8 *src, Uint32 max_len);
void get_file_digest(const Uint8 * filename, Uint8 digest[16]);
void get_string_digest(const Uint8 * string, Uint8 digest[16]);

void http_get_file(char *server, char *path, FILE *fp);

float xmlGetFloat(xmlNode * n, xmlChar * p);
int xmlGetInt(xmlNode *n, xmlChar *p);
int my_xmlstrncopy(char ** dest, char * src, int len);

#endif

