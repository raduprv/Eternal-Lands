#ifndef __ASC_H__
#define __ASC_H__

//proto
int get_integer_after_string(char * source_pointer, char * dest_pointer, 
							 int max_len);
float get_float_after_string(char * source_pointer, char * dest_pointer, 
							 int max_len);
int get_string_occurance(char * source_pointer, char * dest_pointer, 
						 int max_len,char beggining);
void my_strcp(char *dest,char * source);
void my_strncp(char *dest,char * source,int len);
void my_strcat(char *dest,char * source);
int my_strncompare(Uint8 *dest, Uint8 *src, int len);
int my_strcompare(Uint8 *dest, Uint8 *src);

#endif

