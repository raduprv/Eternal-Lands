#ifndef __ASC_H__
#define __ASC_H__

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
Sint32 my_strncompare(Uint8 *dest, const Uint8 *src, Sint32 len);
Sint32 my_strcompare(Uint8 *dest, const Uint8 *src);

#endif

