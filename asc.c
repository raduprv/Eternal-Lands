#include <math.h>
#include <stdlib.h>
#include "global.h"

void my_strncp(Uint8 *dest,const Uint8 * source, Sint32 len)
{
	while(*source && --len > 0)
		{
			*dest++=*source++;
		}
	*dest='\0';
}
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
		l2=xmlUTF8Strlen(in);
		
		if(l2<0) lout=l1;
		else if (len>0 && len<l2) lout=len;
		else lout=l2;
		
		inbuf=inbuf2=(char *)malloc((lin+1)*sizeof(char));
		outbuf=outbuf2=(char *)malloc((lout+1)*sizeof(char));

		memcpy(inbuf,in,lin);

		l1=lin;
		l2=lout;

#ifdef WINDOWS
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

#ifdef WINDOWS
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
