#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "md5.h"

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
	int source_lenght;

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
	int source_lenght;

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
	int source_lenght;

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
	if (len < 0)	len=strlen(src);
	if(!src || !src[0] || !src[1] || !src[2] || len == 0) return 0;
	while(*src && len > 0)
		{
			if(*src != toupper(*src)) return 0;	//at least one lower
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

void get_file_digest(const Uint8 * filename, Uint8 digest[16])
{
	MD5 md5;
	FILE *fp = fopen(filename, "r");
	Uint8 buffer[64];
	Sint32 length;
	MD5Open(&md5);
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

void http_get_file(char *server, char *path, FILE *fp)
{
	IPaddress http_ip;
	TCPsocket http_sock;
	char message[1024];
	int len;
	int got_header = 0;
	SDLNet_ResolveHost(&http_ip, server, 80);
	http_sock = SDLNet_TCP_Open(&http_ip);
	strcpy(message, "GET ");
	strcat(message, path);
	strcat(message, " HTTP/1.0\n\n");
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
