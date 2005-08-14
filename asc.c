#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>
#include "global.h"
#include "md5.h"

/* NOTE: This file contains implementations of the following, currently unused and commented functions:
 *          Look at the end of the file.
 *
 * Sint32 get_string_after_string(const Uint8*, const Uint8*, Sint32, Uint*, int);
 * void get_file_digest(const Uint8*, Uint8[16]);
 * void get_string_digest(const Uint8*, Uint8[16]);
 */

#ifndef LINUX
int my_UTF8Toisolat1(char **dest, size_t * lu, const char **src, size_t * len);
#else
int my_UTF8Toisolat1(char **dest, size_t * lu, char **src, size_t * len);
#endif

//find the first string occurance, and return the distance to that string
//if beggining is 1, it returns the offset to the beginning of the string
//otherwise it returns the offset to the end of the string
Sint32 get_string_occurance(const Uint8 * source_pointer, const Uint8 * dest_pointer, Sint32 max_len,Uint8 beginning)
{
	int i;
	int j;
	int k;
	Uint8 cur_src_char;
	Uint8 cur_dest_char;
	int source_length;

	source_length=strlen(source_pointer);
	i=j=0;
	for(i=0;i<max_len;i++)
		{
			k=0;
			j=0;//how if j different from k?
			while(k<source_length)
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
			if(k==source_length)//we found the string
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
//if the string is not found, after max_len, the function returns -1.
//the function is NOT case sensitive
Sint32 get_integer_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer,
							 int max_len)
{
	int i;
	int j;
	int k;
	Uint8 cur_src_char;
	Uint8 cur_dest_char;
	int source_length;

	source_length=strlen(source_pointer);
	i=j=0;
	for(i=0;i<max_len;i++)
		{
			k=0;
			j=0;
			while(k<source_length)
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
			if(k==source_length)//we found the string
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

//this function returns a float, after the source string in the destination string
//if the string is not found, after max_len, the function returns -1.0f.
//the function is NOT case sensitive
float get_float_after_string(const Uint8 * source_pointer, const Uint8 * dest_pointer,
							 int max_len)
{
	int i;
	int j;
	int k;
	Uint8 cur_src_char;
	Uint8 cur_dest_char;
	int source_length;

	source_length=strlen(source_pointer);
	i=j=0;
	for(i=0;i<max_len;i++)
		{
			k=0;
			j=0;
			while(k<source_length)
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
			if(k==source_length)//we found the string
				{
					//we have to find the first number now (there might be spaces, or other chars first
					while(1)
						{
							cur_dest_char=*(dest_pointer+i);
							if((cur_dest_char>=48 && cur_dest_char<=57) || cur_dest_char=='-' || cur_dest_char=='+' || cur_dest_char=='.')
								{
									break;//we found a number
								}
							if(cur_dest_char==0x0a) return -1;//we didn't find any number on this line
							i++;
						}
					return atof(dest_pointer+i);
				}
		}//end of the for
	return -1.0f;//if we are here, it means we didn't find the string...
}

void my_strcp(Uint8 *dest,const Uint8 * source)
{
	while(*source)
		{
			*dest++=*source++;
		}
	*dest='\0';
}

void my_strncp(Uint8 *dest,const Uint8 * source, Sint32 len)
{
	while(*source && --len > 0)
		{
			*dest++=*source++;
		}
	*dest='\0';
}

void my_strcat(Uint8 *dest,const Uint8 * source)
{
	int i,l,dl;
	l=strlen(source);
	dl=strlen(dest);
	for(i=0;i<l;i++)dest[dl+i]=source[i];
	dest[dl+i]='\0';
}

Sint32 my_strncompare(const Uint8 *dest, const Uint8 *src, Sint32 len)
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

Sint32 my_strcompare(const Uint8 *dest, const Uint8 *src)
{
	Uint32 len;

	len=strlen(dest);
	if(len!=strlen(src))return 0;
	return(my_strncompare(dest, src, len));
}

// is this string more then one character and all alpha in it are CAPS?
Sint32 my_isupper(const Uint8 *src, int len)
{
	int alpha=0;
	if (len < 0)	len=strlen(src);
	if(!src || !src[0] || !src[1] || !src[2] || len == 0) return 0;
	while(*src && len > 0)
		{
			if(isalpha(*src)) alpha++;
			if((isdigit(*src)&&alpha<len/2) || *src != toupper(*src)) return 0;	//at least one lower
			src++;
			len--;
		}
	return 1;	// is all upper or all num
}

Uint8 *my_tolower(Uint8 *src)
{
	Uint8 *dest=src;

	if(!dest || !dest[0]) return dest;
	while(*src)
		{
			*src=tolower(*src);
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
Uint32	clean_file_name(Uint8 *dest, const Uint8 *src, Uint32 max_len)
{
	Uint32	len;
	Uint32	i;

	len=strlen(src);
	if(len > max_len)len=max_len-1;
	for(i=0;i<len;i++)
		{
			if(src[i]=='\\')
				{
					dest[i]='/';
				}
			else
				{
					dest[i]=src[i];
				}
		}
	//always place a null that the end
	dest[len]='\0';
	return(len);
}

void http_get_file(char *server, char *path, FILE *fp)
{
	IPaddress http_ip;
	TCPsocket http_sock;
	char message[1024];
	int len;
	int got_header = 0;
	SDLNet_ResolveHost(&http_ip, server, 80);
	http_sock = SDLNet_TCP_Open(&http_ip);
	snprintf(message, sizeof(message), "GET %s HTTP/1.0\n\n", path);
	len = strlen(message);
	SDLNet_TCP_Send(http_sock,message,len);
	while(len > 0)
		{
			char buf[1024];
			memset(buf, 0, 1024);
			len=SDLNet_TCP_Recv(http_sock, buf, 1024);
			if(!got_header)
				{
					int i;
					for(i = 0; i < len; i++)
						{
							if(!got_header &&
							   buf[i] == 0x0D && buf[i+1] == 0x0A &&
							   buf[i+2] == 0x0D && buf[i+3] == 0x0A) {
								fwrite(buf + i + 4, 1, len - i - 4, fp);
								got_header = 1;
							}
						}
				}
			else
				fwrite(buf, 1, len, fp);
		}
	SDLNet_TCP_Close(http_sock);
}

/*XML*/

float xmlGetFloat(xmlNode * n, xmlChar * c)
{
	char * t=xmlGetProp(n,c);
	float f=t?atof(t):0;
	free(t);
	return f;
}

int xmlGetInt(xmlNode *n, xmlChar *c)
{
	char *t=xmlGetProp(n,c);
	int i=t?atoi(t):0;
	free(t);
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
		l2=xmlUTF8Strlen(in);
		
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

void get_file_digest(const Uint8 * filename, Uint8 digest[16])
{
	MD5 md5;
	FILE *fp = my_fopen(filename, "r");
	Uint8 buffer[64];
	Sint32 length;
	MD5Open(&md5);
	
	memset (digest, 0, sizeof (digest));	
	if (fp == NULL) return;
	
	while ((length = fread(buffer, 1, sizeof(buffer), fp)) > 0)
		{
			MD5Digest(&md5, buffer, length);
		}
	MD5Close(&md5, digest);
	fclose(fp);
}

void get_string_digest(const Uint8 * string, Uint8 digest[16])
{
	MD5 md5;
	MD5Open(&md5);
	MD5Digest(&md5, string, strlen(string));
	MD5Close(&md5, digest);
}
*/

#ifdef WINDOWS
#ifdef _MSC_VER
// the moronic _snprintf in MSVC doesn't necessarily terminate then string with
// a NULL byte. This function should at least terminate the string, but does
// not return the number of bytes that would have been written if the buffer
// was large enough, like gcc does. Instead, it returns a negative value, 
// indicating error. This probably won't be a problem, as the return value is 
// mostly ignored.
int sane_snprintf (char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int ret;
	
	va_start (ap, format);
	ret = _vsnprintf (str, size - 1, format, ap);
	if (ret < 0)
		str[size-1] = '\0';
	va_end (ap);
	return ret;
}
#endif 
#endif
