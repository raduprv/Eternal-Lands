#include <math.h>
#include <stdlib.h>
#include "global.h"

//find the first string occurance, and return the distance to that string
//if beggining is 1, it returns the offset to the beginning of the string
//otherwise it returns the offset to the end of the string
int get_string_occurance(char * source_pointer, char * dest_pointer, int max_len,char beginning)
{
	int i;
	int j;
	int k;
	char cur_src_char;
	char cur_dest_char;
	int source_lenght;
	char string_found;

	source_lenght=strlen(source_pointer);
	i=j=0;
for(i=0;i<max_len;i++)
  {
	k=0;
	j=0;
	while(k<source_lenght)
		{
			cur_src_char=*(source_pointer+j);
			cur_dest_char=*(dest_pointer+i);

			if(cur_src_char>=65 && cur_src_char<=90)cur_src_char+=32;
			if(cur_dest_char>=65 && cur_dest_char<=90)cur_dest_char+=32;

			if(cur_src_char!=cur_dest_char)break;//not found, sorry
			i++;
			j++;
			k++;
		}
	if(k==source_lenght)//we found the string
		{
			if(!beginning)return i;
			else return i-k;
		}
  }//end of the for
return -1;//if we are here, it means we didn't find the string...
}

//this function returns an integer, after the source string in the destination string
//if the string is not found, after max_len, the function returns null.
//the function is NOT case sensitive
int get_integer_after_string(char * source_pointer, char * dest_pointer, int max_len)
{
	int i;
	int j;
	int k;
	char cur_src_char;
	char cur_dest_char;
	int source_lenght;
	char string_found;

	source_lenght=strlen(source_pointer);
	i=j=0;
for(i=0;i<max_len;i++)
  {
	k=0;
	j=0;
	while(k<source_lenght)
		{
			cur_src_char=*(source_pointer+j);
			cur_dest_char=*(dest_pointer+i);

			if(cur_src_char>=65 && cur_src_char<=90)cur_src_char+=32;
			if(cur_dest_char>=65 && cur_dest_char<=90)cur_dest_char+=32;

			if(cur_src_char!=cur_dest_char)break;//not found, sorry
			i++;
			j++;
			k++;
		}
	if(k==source_lenght)//we found the string
		{
			//we have to find the first number now (there might be spaces, or other chars first
			while(1)
				{
					cur_dest_char=*(dest_pointer+i);
					if((cur_dest_char>=48 && cur_dest_char<=57) || cur_dest_char=='-' || cur_dest_char=='+')break;//we found a number
					if(cur_dest_char==0x0a) return -1;//we didn't find any number on this line
					i++;
				}
			return atoi(dest_pointer+i);
		}
  }//end of the for
return -1;//if we are here, it means we didn't find the string...
}

//this function returns an integer, after the source string in the destination string
//if the string is not found, after max_len, the function returns null.
//the function is NOT case sensitive
float get_float_after_string(char * source_pointer, char * dest_pointer, int max_len)
{
	int i;
	int j;
	int k;
	char cur_src_char;
	char cur_dest_char;
	int source_lenght;
	char string_found;

	source_lenght=strlen(source_pointer);
	i=j=0;
for(i=0;i<max_len;i++)
  {
	k=0;
	j=0;
	while(k<source_lenght)
		{
			cur_src_char=*(source_pointer+j);
			cur_dest_char=*(dest_pointer+i);

			if(cur_src_char>=65 && cur_src_char<=90)cur_src_char+=32;
			if(cur_dest_char>=65 && cur_dest_char<=90)cur_dest_char+=32;

			if(cur_src_char!=cur_dest_char)break;//not found, sorry
			i++;
			j++;
			k++;
		}
	if(k==source_lenght)//we found the string
		{
			//we have to find the first number now (there might be spaces, or other chars first
			while(1)
				{
					cur_dest_char=*(dest_pointer+i);
					if((cur_dest_char>=48 && cur_dest_char<=57) || cur_dest_char=='-' || cur_dest_char=='+')break;//we found a number
					if(cur_dest_char==0x0a) return -1;//we didn't find any number on this line
					i++;
				}
			return atof(dest_pointer+i);
		}
  }//end of the for
return -1;//if we are here, it means we didn't find the string...
}
