#ifndef __ASC_H__
#define __ASC_H__

#define my_xmlStrcpy(d,s) my_xmlStrncopy(d,s,0)

//proto
int get_integer_after_string(char * source_pointer, char * dest_pointer, int max_len);
float get_float_after_string(char * source_pointer, char * dest_pointer, int max_len);
int get_string_occurance(char * source_pointer, char * dest_pointer, int max_len,char beggining);
Sint32 my_strncompare(const Uint8 *dest, const Uint8 *src, Sint32 len);
int my_xmlStrncopy(char ** in, const char * xmlstr, int len);
#ifdef WINDOWS
int my_UTF8Toisolat1(char **dest, size_t * lu, const char **src, size_t * l);
#else
int my_UTF8Toisolat1(char **dest, size_t * lu, char **src, size_t * l);
#endif
#endif
