#include <stdlib.h>
#include <string.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#ifdef	ZLIB
#include	<zlib.h>
#endif

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * char* load_bmp8_color_key_no_texture(char*);
 * char* load_bmp8_alpha_map(char*);
 */

//get a texture id out of the texture cache
//if null, then reload it (means it was previously freed)

int get_texture_id(int i)
{
	int new_texture_id;
	unsigned char alpha;

	if(!texture_cache[i].texture_id)
	{
		alpha=texture_cache[i].alpha;
		//our texture was freed, we have to reload it
		if(alpha == 0) {
			new_texture_id=load_bmp8_color_key(texture_cache[i].file_name);
		} else {
			new_texture_id=load_bmp8_fixed_alpha(texture_cache[i].file_name, alpha);
		}
		texture_cache[i].texture_id=new_texture_id;
	}
	return texture_cache[i].texture_id;
}

#ifndef	USE_INLINE
void	bind_texture_id(int texture_id)
{
	if(last_texture!=texture_id)
	{
		last_texture=texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}
}

int get_and_set_texture_id(int i)
{
	int	texture_id;

#ifdef	DEBUG
	if(i<0||i>TEXTURE_CACHE_MAX) {
		log_error("We tried binding a texture ID of %d\n", i);
		return 0;
	}
#endif	//DEBUG

	// do we need to make a hard load or do we already have it?
	if(!texture_cache[i].texture_id)
	{
		texture_id= get_texture_id(i);
	} else {
		texture_id= texture_cache[i].texture_id;
	}
	bind_texture_id(texture_id);

	return(texture_id);
}
#endif	//USE_INLINE

int load_alphamap(char * FileName, char * texture_mem, int orig_x_size, int orig_y_size)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	char filename[512];//Create a buffer...
	char * name;
#ifdef	ZLIB
	gzFile *f = NULL;
#else	//ZLIB
	FILE *f = NULL;
#endif	//ZLIB

	/* copy (maybe truncating) FileName into a buffer */
	snprintf(filename, sizeof(filename), "%s", FileName);
	/* find last dot */
	name = strrchr(filename, '.');
	if (name == NULL)
	{
		name = filename;
	}
	/* terminate filename before last dot */
	*name = '\0';

	/* safely add '_alpha.bmp' to the string */
	strncat(filename, "_alpha.bmp", sizeof(filename) - strlen(filename) - 1);

#ifdef	ZLIB
	f= my_gzopen(filename);
#else	//ZLIB
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!= SDL_SwapLE16(19778))//BM (the identifier)
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=18;
	x_size= SDL_SwapLE32(*((int *) file_mem));
	file_mem+=4;
	y_size= SDL_SwapLE32(*((int *) file_mem));
	if(x_size != orig_x_size || y_size != orig_y_size){
		log_error("The alphamap for %s was not the same size as the original - we didn't load the alphamap...", FileName);
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB

		return 0;
	}
	file_mem+=6;
	if(*((short *)file_mem)!=SDL_SwapLE16(8))//8 bit/pixel?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=16;

	colors_no=SDL_SwapLE32(*((int *)file_mem));
	if(!colors_no)
		colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
#ifdef	ZLIB
	gzread(f, file_mem, colors_no*4+4);//header only
#else	//ZLIB
	fread(file_mem, 1, colors_no*4+4, f);//header only
#endif	//ZLIB
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;

	//now, allocate the memory for the file
	read_buffer = (Uint8 *) calloc ( x_size-x_padding+1, sizeof(Uint8));

	for(y=0;y<y_size;y++)
	{
#ifdef	ZLIB
		gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB
		for(x=0;x<x_size;x++)
		{
			current_pallete_entry=*(read_buffer+x);
			b=*(color_pallete+current_pallete_entry*4);
			g=*(color_pallete+current_pallete_entry*4+1);
			r=*(color_pallete+current_pallete_entry*4+2);
			a=(r+b+g)/3;
			*(texture_mem+(y*x_size+x)*4+3)=a;
		}
	}

#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
	free(file_mem_start);
	free(read_buffer);

	return 1;
}

//load a bmp texture, in respect to the color key
GLuint load_bmp8_color_key(char * filename)
{
	int y,x_padding,x_size,y_size,colors_no;
	Uint8 * file_mem;
	//Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 read_buffer[2048];
	Uint8 color_pallete[256*4];
	GLuint texture;
#ifdef	ZLIB
	gzFile *f = NULL;
#else	//ZLIB
	FILE *f = NULL;
#endif	//ZLIB

	CHECK_GL_ERRORS();
#ifdef	ZLIB
	{
		char	gzfilename[1024];
		strcpy(gzfilename, filename);
		strcat(gzfilename, ".gz");
		f= gzopen(gzfilename, "rb");
		if(!f){
			// didn't work, try the name that was specified
			f= gzopen(filename, "rb");
		}
	}
#else	//ZLIB
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return 0;
  	file_mem= read_buffer;
  	//file_mem= (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	//file_mem_start= file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50+4);	//header only, plus first 4 bytes of the color pallete
#else	//ZLIB
  	fread(file_mem, 1, 50+4, f);	//header only, plus first 4 bytes of the color pallete
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!= SDL_SwapLE16(19778))//BM (the identifier)
	{
		//free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=18;
	x_size= SDL_SwapLE32(*((int *) file_mem));
	file_mem+=4;
	y_size= SDL_SwapLE32(*((int *) file_mem));
	file_mem+=6;
	if(*((short *)file_mem)!=SDL_SwapLE16(8))//8 bit/pixel?
	{
		//free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
	{
		//free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=16;

	colors_no=SDL_SwapLE32(*((int *)file_mem));
	if(!colors_no)
		colors_no=256;
	//file_mem+=8;//here comes the pallete

#ifdef	ZLIB
	gzread(f, color_pallete, colors_no*4);	// just the color pallete
#else	//ZLIB
	//color_pallete=file_mem+4;
	//fread (file_mem, 1, colors_no*4+4, f);//header only
	//file_mem+=colors_no*4;	// not needed, value isn't used
	fread(color_pallete, 1, colors_no*4, f);	// just the color pallete
#endif	//ZLIB

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;
	if(x_size<=x_padding)
		x_padding=0;

	//now, allocate the memory for the file
	texture_mem= (Uint8 *) calloc(x_size*y_size*4, sizeof(Uint8));
	//read_buffer= (Uint8 *) calloc(2000, sizeof(Uint8));

	for(y=0; y<y_size; y++)
	{
		int	x;
		int	y_offset= y*x_size;

#ifdef	ZLIB
		gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB
		for(x=0; x<x_size; x++)
		{
			int r,g,b, current_pallete_entry, texture_offset;

			// find what color we are really using
			current_pallete_entry= (*(read_buffer+x))*4;
			b= color_pallete[current_pallete_entry];
			g= color_pallete[current_pallete_entry+1];
			r= color_pallete[current_pallete_entry+2];

			// and store the converted version in the proper spot
			texture_offset= (y_offset+x)*4;
			texture_mem[texture_offset]= r;
			texture_mem[texture_offset+1]= g;
			texture_mem[texture_offset+2]= b;
			texture_mem[texture_offset+3]= (r+b+g)/3;	// a
		}
	}

	//free(read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB

	load_alphamap(filename, texture_mem, x_size, y_size);
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}


	if(have_arb_compression)
	{
		if(have_s3_compression)
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		else
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();

	//free(file_mem_start);
	free(texture_mem);
	return texture;
}

//load a bmp texture, with the specified global alpha
GLuint load_bmp8_fixed_alpha(char * filename, Uint8 a)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	GLuint texture;
	short format;
#ifdef	ZLIB
	gzFile *f = NULL;
	{
		char	gzfilename[1024];
		strcpy(gzfilename, filename);
		strcat(gzfilename, ".gz");
		f= gzopen(gzfilename, "rb");
		if(!f){
			// didn't work, try the name that was specified
			f= gzopen(filename, "rb");
		}
	}
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=SDL_SwapLE16(19778))//BM (the identifier)
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=18;
	x_size=SDL_SwapLE32(*((int *) file_mem));
	file_mem+=4;
	y_size=SDL_SwapLE32(*((int *) file_mem));
	file_mem+=6;
	
	format = *((short *)file_mem);
	if(format != SDL_SwapLE16(8) && format != SDL_SwapLE16(24)) //8 or 24 bit/pixel?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	
	x_padding=x_size%4;
	if(x_padding)
		x_padding = 4-x_padding;
	if(x_size<=x_padding)
		x_padding = 0;

	if(format == SDL_SwapLE16(8))
	{
		file_mem+=16;

		colors_no=SDL_SwapLE32(*((int *)file_mem));
		if(!colors_no)
			colors_no=256;
		file_mem+=8;//here comes the pallete
		
		color_pallete=file_mem+4;
#ifdef	ZLIB
		gzread(f, file_mem, colors_no*4+4);//header only
#else	//ZLIB
		fread(file_mem, 1, colors_no*4+4, f);//header only
#endif	//ZLIB
		file_mem+=colors_no*4;
		
		//now, allocate the memory for the file
		texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
		read_buffer = (Uint8 *) calloc ( x_size-x_padding+1, sizeof(Uint8));

		for(y=0;y<y_size;y++)
		{
#ifdef	ZLIB
			gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
			//fread(texture_mem+y*x_size, 1, x_size-x_padding, f);
			fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB
			for(x=0; x<x_size; x++)
			{
				current_pallete_entry=*(read_buffer+x);
				b=*(color_pallete+current_pallete_entry*4);
				g=*(color_pallete+current_pallete_entry*4+1);
				r=*(color_pallete+current_pallete_entry*4+2);
				*(texture_mem+(y*x_size+x)*4)=r;
				*(texture_mem+(y*x_size+x)*4+1)=g;
				*(texture_mem+(y*x_size+x)*4+2)=b;
				*(texture_mem+(y*x_size+x)*4+3)=a;
			}
		}
	}
	
	else // decode 24 bpp bitmap
	{
#ifdef	ZLIB
		gzread(f, file_mem, 4); // Skip 4 bytes
#else	//ZLIB
		fread(file_mem, 1, 4, f); // Skip 4 bytes
#endif	//ZLIB
		
		//now, allocate the memory for the file
		texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
		read_buffer = (Uint8 *) calloc ( 3* (x_size-x_padding+1), sizeof(Uint8));
		
		for(y = 0; y < y_size; y++)
		{
#ifdef	ZLIB
			gzread(f, read_buffer, (x_size-x_padding)* 3);
#else	//ZLIB
			fread(read_buffer, 1, (x_size-x_padding)* 3, f);
#endif	//ZLIB
			for(x = 0; x < x_size; x++)
			{
				*(texture_mem+(y*x_size+x)*4)   = *(read_buffer+ x*3 + 2);
				*(texture_mem+(y*x_size+x)*4+1) = *(read_buffer+ x*3 + 1);
				*(texture_mem+(y*x_size+x)*4+2) = *(read_buffer+ x*3 + 0);
				*(texture_mem+(y*x_size+x)*4+3) = a;
			}
		}
	}

	free(file_mem_start);
	free(read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(have_arb_compression)
	{
		if(have_s3_compression)
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		else
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}
	CHECK_GL_ERRORS();
	free(texture_mem);
	return texture;
}

//Tests to see if a texture is already loaded. If it is, return the handle.
//If not, load it, and return the handle
int load_texture_cache (const char * file_name, unsigned char alpha)
{
	int slot = load_texture_cache_deferred(file_name, alpha);
	get_and_set_texture_id(slot);
	return slot;
}

int load_texture_cache_deferred (const char * file_name, unsigned char alpha)
{
	int i;
	int file_name_length;
	int texture_slot= -1;

	file_name_length=strlen(file_name);

	for(i=0;i<TEXTURE_CACHE_MAX;i++)
	{
		if(texture_cache[i].file_name[0])
		{
			if(!strcasecmp(texture_cache[i].file_name, file_name))
			{
				// already loaded, use existing texture
				return i;
			}
		}
		else
		{
			// remember the first open slot we have
			if(texture_slot < 0)
			{
				texture_slot= i;
			}
		}
	}

	if(texture_slot >= 0 && !texture_cache[texture_slot].file_name[0]) {//we found a place to store it
		snprintf(texture_cache[texture_slot].file_name, sizeof(texture_cache[texture_slot].file_name), "%s", file_name);
		texture_cache[texture_slot].texture_id = 0;
		texture_cache[texture_slot].alpha = alpha;
		return texture_slot;
	} else {	
		log_error("Error: out of texture space\n");
		return 0;	// ERROR!
	}
}


#ifndef MAP_EDITOR2
//load a bmp texture, and remaps it
GLuint load_bmp8_remapped_skin(char * filename, Uint8 a, short skin, short hair, short shirt,
							   short pants, short boots)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,current_pallete_entry;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	GLuint texture;
#ifdef	ZLIB
	gzFile *f = NULL;
	{
		char	gzfilename[1024];
		strcpy(gzfilename, filename);
		strcat(gzfilename, ".gz");
		f= gzopen(gzfilename, "rb");
		if(!f){
			// didn't work, try the name that was specified
			f= gzopen(filename, "rb");
		}
	}
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=SDL_SwapLE16(19778))//BM (the identifier)
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=18;
	x_size=SDL_SwapLE32(*((int *) file_mem));
	file_mem+=4;
	y_size=SDL_SwapLE32(*((int *) file_mem));
	file_mem+=6;
	if(*((short *)file_mem)!=SDL_SwapLE16(8))//8 bit/pixel?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return 0;
	}
	file_mem+=16;

	colors_no=SDL_SwapLE32(*((int *)file_mem));
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
#ifdef	ZLIB
	gzread(f, file_mem, colors_no*4+4);//header only
#else	//ZLIB
	fread(file_mem, 1, colors_no*4+4, f);//header only
#endif	//ZLIB
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));

	r=g=b=0;	//keep the compiler happy
	for(y=0;y<y_size;y++)
	{
#ifdef	ZLIB
		gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB

		for(x=0;x<x_size;x++)
		{
			current_pallete_entry=*(read_buffer+x);
			if(current_pallete_entry>1 && current_pallete_entry<22)
			{
				if(current_pallete_entry>1 && current_pallete_entry<6)//skin
				{
					int intensity;
					intensity=current_pallete_entry-1;

					switch(intensity)
					{
						case 1:
							r=colors_list[skin].r1;
							g=colors_list[skin].g1;
							b=colors_list[skin].b1;
						break;
						case 2:
							r=colors_list[skin].r2;
							g=colors_list[skin].g2;
							b=colors_list[skin].b2;
						break;
						case 3:
							r=colors_list[skin].r3;
							g=colors_list[skin].g3;
							b=colors_list[skin].b3;
						break;
						case 4:
							r=colors_list[skin].r4;
							g=colors_list[skin].g4;
							b=colors_list[skin].b4;
						break;
					}

				}
				else if(current_pallete_entry>5 && current_pallete_entry<10)//hair
				{
					int intensity;
					intensity=current_pallete_entry-5;

					switch(intensity)
					{
						case 1:
							r=colors_list[hair].r1;
							g=colors_list[hair].g1;
							b=colors_list[hair].b1;
						break;
						case 2:
							r=colors_list[hair].r2;
							g=colors_list[hair].g2;
							b=colors_list[hair].b2;
						break;
						case 3:
							r=colors_list[hair].r3;
							g=colors_list[hair].g3;
							b=colors_list[hair].b3;
						break;
						case 4:
							r=colors_list[hair].r4;
							g=colors_list[hair].g4;
							b=colors_list[hair].b4;
						break;
					}
				}
				else if(current_pallete_entry>9 && current_pallete_entry<14)//shirt
				{
					int intensity;
					intensity=current_pallete_entry-9;

					switch(intensity)
					{
						case 1:
							r=colors_list[shirt].r1;
							g=colors_list[shirt].g1;
							b=colors_list[shirt].b1;
						break;
						case 2:
							r=colors_list[shirt].r2;
							g=colors_list[shirt].g2;
							b=colors_list[shirt].b2;
						break;
						case 3:
							r=colors_list[shirt].r3;
							g=colors_list[shirt].g3;
							b=colors_list[shirt].b3;
						break;
						case 4:
							r=colors_list[shirt].r4;
							g=colors_list[shirt].g4;
							b=colors_list[shirt].b4;
						break;
					}
				}
				else if(current_pallete_entry>13 && current_pallete_entry<18)//pants
				{
					int intensity;
					intensity=current_pallete_entry-13;

					switch(intensity)
					{
						case 1:
							r=colors_list[pants].r1;
							g=colors_list[pants].g1;
							b=colors_list[pants].b1;
						break;
						case 2:
							r=colors_list[pants].r2;
							g=colors_list[pants].g2;
							b=colors_list[pants].b2;
						break;
						case 3:
							r=colors_list[pants].r3;
							g=colors_list[pants].g3;
							b=colors_list[pants].b3;
						break;
						case 4:
							r=colors_list[pants].r4;
							g=colors_list[pants].g4;
							b=colors_list[pants].b4;
						break;
					}
				}

				else if(current_pallete_entry>17 && current_pallete_entry<22)//boots
				{
					int intensity;
					intensity=current_pallete_entry-17;

					switch(intensity)
					{
						case 1:
							r=colors_list[boots].r1;
							g=colors_list[boots].g1;
							b=colors_list[boots].b1;
						break;
						case 2:
							r=colors_list[boots].r2;
							g=colors_list[boots].g2;
							b=colors_list[boots].b2;
						break;
						case 3:
							r=colors_list[boots].r3;
							g=colors_list[boots].g3;
							b=colors_list[boots].b3;
						break;
						case 4:
							r=colors_list[boots].r4;
							g=colors_list[boots].g4;
							b=colors_list[boots].b4;
						break;
					}
				}
			}
			else//not a special color, leave it alone
			{
				b=*(color_pallete+current_pallete_entry*4);
				g=*(color_pallete+current_pallete_entry*4+1);
				r=*(color_pallete+current_pallete_entry*4+2);
			}

			*(texture_mem+(y*x_size+x)*4)=r;
			*(texture_mem+(y*x_size+x)*4+1)=g;
			*(texture_mem+(y*x_size+x)*4+2)=b;
			*(texture_mem+(y*x_size+x)*4+3)=a;
		}

	}

	free(file_mem_start);
	free(read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(have_arb_compression)
	{
		if(have_s3_compression)
		glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		else
		glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}


	CHECK_GL_ERRORS();
	free(texture_mem);
	return texture;
}


void load_bmp8_to_coordinates (const char *filename, Uint8 *texture_space, int x_pos, int y_pos, Uint8 alpha)
{
	int x, y, x_padding, x_size, y_size, colors_no, r, g, b, current_pallete_entry;
	Uint8 *file_mem;
	Uint8 *file_mem_start;
	Uint8 *read_buffer;
	Uint8 *color_pallete;
#ifdef	ZLIB
	gzFile *f = NULL;
	{
		char	gzfilename[1024];
		strcpy(gzfilename, filename);
		strcat(gzfilename, ".gz");
		f= gzopen(gzfilename, "rb");
		if(!f){
			// didn't work, try the name that was specified
			f= gzopen(filename, "rb");
		}
	}
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (f == NULL)
		return;
  	file_mem = calloc (20000, sizeof(Uint8));
  	file_mem_start = file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if (*((short *) file_mem) != SDL_SwapLE16 (19778)) // BM (the identifier)
	{
		free (file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return;
	}
	file_mem += 18;
	x_size = SDL_SwapLE32 (*((int *) file_mem));
	file_mem += 4;
	y_size = SDL_SwapLE32 (*((int *) file_mem));
	file_mem += 6;
	if (*((short *)file_mem) != SDL_SwapLE16(8)) // 8 bit/pixel?
	{
		free (file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return;
	}

	file_mem += 2;
	if (*((int *)file_mem) != SDL_SwapLE32 (0)) // any compression?
	{
		free (file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return;
	}
	file_mem += 16;

	colors_no = SDL_SwapLE32 (*((int *)file_mem));
	if (colors_no == 0)
		colors_no = 256;
	file_mem += 8; // here comes the pallete

	color_pallete = file_mem + 4;
#ifdef	ZLIB
	gzread(f, file_mem, colors_no*4+4); // header only
#else	//ZLIB
	fread(file_mem, 1, colors_no*4+4, f); // header only
#endif	//ZLIB
	file_mem += colors_no * 4;

	x_padding = x_size % 4;
	if (x_padding)
		x_padding = 4 - x_padding;
	if (x_size <= x_padding)
		x_padding = 0;

	// now, allocate the memory for the file
	read_buffer = calloc (x_size + x_padding + 1, sizeof(Uint8));

	for (y = y_size - 1; y >= 0; y--)
	{
#ifdef	ZLIB
		gzread(f, read_buffer, x_size+x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size+x_padding, f);
#endif	//ZLIB
		for (x = 0; x < x_size; x++)
		{
			int texture_y;
			texture_y = (255 - (y + y_pos));
			current_pallete_entry = *(read_buffer+x);
			b = *(color_pallete+current_pallete_entry*4);
			g = *(color_pallete+current_pallete_entry*4+1);
			r = *(color_pallete+current_pallete_entry*4+2);
			*(texture_space+(texture_y*256+x+x_pos)*4) = r;
			*(texture_space+(texture_y*256+x+x_pos)*4+1) = g;
			*(texture_space+(texture_y*256+x+x_pos)*4+2) = b;
			*(texture_space+(texture_y*256+x+x_pos)*4+3) = alpha;
		}
	}

	free (file_mem_start);
	free (read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
}


#ifdef	ELC
int load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a)
{
	GLuint texture;
	Uint8 * texture_mem;

	texture_mem=(Uint8*)calloc(1,256*256*4);
	if(this_actor->pants_tex[0])load_bmp8_to_coordinates(this_actor->pants_tex,texture_mem,78,175,a);
	if(this_actor->boots_tex[0])load_bmp8_to_coordinates(this_actor->boots_tex,texture_mem,0,175,a);
#ifdef NEW_TEX
	if(this_actor->torso_tex[0])load_bmp8_to_coordinates(this_actor->torso_tex,texture_mem,158,149,a);
#else
	if(this_actor->torso_tex[0])load_bmp8_to_coordinates(this_actor->torso_tex,texture_mem,158,156,a);
#endif
	if(this_actor->arms_tex[0])load_bmp8_to_coordinates(this_actor->arms_tex,texture_mem,0,96,a);
	if(this_actor->hands_tex[0])load_bmp8_to_coordinates(this_actor->hands_tex,texture_mem,67,64,a);
	if(this_actor->head_tex[0])load_bmp8_to_coordinates(this_actor->head_tex,texture_mem,67,0,a);
	if(this_actor->hair_tex[0])load_bmp8_to_coordinates(this_actor->hair_tex,texture_mem,0,0,a);
#ifdef NEW_TEX
	if(this_actor->weapon_tex[0])load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,178,77,a);
	if(this_actor->shield_tex[0])load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,100,77,a);
#else
	if(this_actor->weapon_tex[0])load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,158,77,a);
	if(this_actor->shield_tex[0])load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,80,96,a);
#endif
	if(this_actor->helmet_tex[0])load_bmp8_to_coordinates(this_actor->helmet_tex,texture_mem,80,149,a);
	if(this_actor->cape_tex[0])load_bmp8_to_coordinates(this_actor->cape_tex,texture_mem,131,0,a);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	//failsafe
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(have_arb_compression)
	{
		if(have_s3_compression)
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_S3TC_DXT5_EXT,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
		else
			glTexImage2D(GL_TEXTURE_2D,0,COMPRESSED_RGBA_ARB,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);
	}

	CHECK_GL_ERRORS();
	free(texture_mem);
	return texture;
}
#endif	//ELC
#endif  //MAP_EDITOR2

#ifdef MAP_EDITOR2
/////////////////////////////////////////////////////////////////////////////////////
//load a bmp file, convert it to the rgba format, but don't assign it to any texture object
char * load_bmp8_color_key_no_texture_img(char * filename, img_struct * img)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	Uint8 *texture_mem;
#ifdef	ZLIB
	gzFile *f = NULL;
	{
		char	gzfilename[1024];
		strcpy(gzfilename, filename);
		strcat(gzfilename, ".gz");
		f= gzopen(gzfilename, "rb");
		if(!f){
			// didn't work, try the name that was specified
			f= gzopen(filename, "rb");
		}
	}
#else	//ZLIB
	FILE *f = NULL;
	f= fopen(filename, "rb");
#endif	//ZLIB
  	if (!f)
		return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
#ifdef	ZLIB
  	gzread(f, file_mem, 50);//header only
#else	//ZLIB
  	fread(file_mem, 1, 50, f);//header only
#endif	//ZLIB
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
	{
		free(file_mem_start);
		fclose (f);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
	{
		free(file_mem_start);
#ifdef	ZLIB
		gzclose(f);
#else	//ZLIB
		fclose(f);
#endif	//ZLIB
		return NULL;
	}
	file_mem+=16;

	colors_no=*((int *)file_mem);
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
#ifdef	ZLIB
	gzread(f, file_mem, colors_no*4+4);//header only
#else	//ZLIB
	fread(file_mem, 1, colors_no*4+4, f);//header only
#endif	//ZLIB
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)
		x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));

	for(y=0;y<y_size;y++)
	{
#ifdef	ZLIB
		gzread(f, read_buffer, x_size-x_padding);
#else	//ZLIB
		fread(read_buffer, 1, x_size-x_padding, f);
#endif	//ZLIB

		for(x=0;x<x_size;x++)
		{
			current_pallete_entry=*(read_buffer+x);
			b=*(color_pallete+current_pallete_entry*4);
			g=*(color_pallete+current_pallete_entry*4+1);
			r=*(color_pallete+current_pallete_entry*4+2);
			*(texture_mem+(y*x_size+x)*4)=r;
			*(texture_mem+(y*x_size+x)*4+1)=g;
			*(texture_mem+(y*x_size+x)*4+2)=b;
			a=(r+b+g)/3;
			*(texture_mem+(y*x_size+x)*4+3)=a;
		}

	}
	
	if(img)
	{
		img->x=x_size;
		img->y=y_size;
	}
	
	free(file_mem_start);
	free(read_buffer);
#ifdef	ZLIB
	gzclose(f);
#else	//ZLIB
	fclose(f);
#endif	//ZLIB
	return texture_mem;
}
#endif

/* currently UNUSED
/////////////////////////////////////////////////////////////////////////////////////
//load a bmp file, convert it to the rgba format, but don't assign it to any texture object
char * load_bmp8_color_key_no_texture(char * FileName)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;
	Uint8 *texture_mem;

  	f = my_fopen (FileName, "rb");
  	if (!f) return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=SDL_SwapLE16(19778))//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=SDL_SwapLE16(8))//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=16;

	colors_no=SDL_SwapLE32(*((int *)file_mem));
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
	fread (file_mem, 1, colors_no*4+4, f);//header only
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size*4, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));


	for(y=0;y<y_size;y++)
		{
			fread (read_buffer, 1, x_size-x_padding, f);

			for(x=0;x<x_size;x++)
				{
					current_pallete_entry=*(read_buffer+x);
					b=*(color_pallete+current_pallete_entry*4);
					g=*(color_pallete+current_pallete_entry*4+1);
					r=*(color_pallete+current_pallete_entry*4+2);
					*(texture_mem+(y*x_size+x)*4)=r;
					*(texture_mem+(y*x_size+x)*4+1)=g;
					*(texture_mem+(y*x_size+x)*4+2)=b;
					a=(r+b+g)/3;
					*(texture_mem+(y*x_size+x)*4+3)=a;
				}

		}

	free(file_mem_start);
	free(read_buffer);
	fclose (f);
	return texture_mem;
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such
}

/////////////////////////////////////////////////////////////////////////////////////
//load a bmp file, convert it to an alpha map
char * load_bmp8_alpha_map(char * FileName)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;
	Uint8 *texture_mem;

  	f = my_fopen (FileName, "rb");
  	if (!f) return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=SDL_SwapLE16(19778))//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=SDL_SwapLE16(8))//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=SDL_SwapLE32(0))//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=16;

	colors_no=SDL_SwapLE32(*((int *)file_mem));
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
	fread (file_mem, 1, colors_no*4+4, f);//header only
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)x_padding=4-x_padding;

	//now, allocate the memory for the file
	texture_mem = (Uint8 *) calloc ( x_size*y_size, sizeof(Uint8));
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));


	for(y=0;y<y_size;y++)
		{
			fread (read_buffer, 1, x_size-x_padding, f);

			for(x=0;x<x_size;x++)
				{
					current_pallete_entry=*(read_buffer+x);
					b=*(color_pallete+current_pallete_entry*4);
					g=*(color_pallete+current_pallete_entry*4+1);
					r=*(color_pallete+current_pallete_entry*4+2);
					a=(r+b+g)/3;
					*(texture_mem+(y*x_size+x))=a;
				}

		}

	free(file_mem_start);
	free(read_buffer);
	fclose(f);
	return texture_mem;
}
*/
