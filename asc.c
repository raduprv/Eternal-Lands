#include <math.h>
#include <stdlib.h>
#include <string.h>
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
	//char string_found; unused?

	source_lenght=strlen(source_pointer);
	i=j=0;
	for(i=0;i<max_len;i++)
		{
			k=0;
			j=0;//how if j different from k?
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
					// skip optional space or equal
					while(dest_pointer[i]==' ' || dest_pointer[i]=='=')
						{
							i++;
						}
					if(!beginning)return i;
					else return i-k+1;
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
	//char string_found; unused?

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
	//char string_found; unused?

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

void my_strcp(char *dest,char * source)
{
	/*int i,l;
	l=strlen(source);
	for(i=0;i<l;i++)dest[i]=source[i];
	dest[i]=0;*/
	while(*source)
		{
			*dest++=*source++;
		}
	*dest='\0';
}

void my_strcat(char *dest,char * source)
{
	int i,l,dl;
	l=strlen(source);
	dl=strlen(dest);
	for(i=0;i<l;i++)dest[dl+i]=source[i];
	dest[dl+i]=0;
}

int my_strncompare(Uint8 *dest, Uint8 *src, int len)
{
	int i;
	Uint8 ch1,ch2;

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

int my_strcompare(Uint8 *dest, Uint8 *src)
{
	int len;//i      unused?
	//Uint8 ch1,ch2;  unused?

	len=strlen(dest);
	if(len!=strlen(src))return 0;
	return(my_strncompare(dest, src, len));
}

