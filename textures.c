#include <stdlib.h>
#include <string.h>
#include "global.h"

//get a texture id out of the texture cache
//if null, then reload it (means it was previously freed)
int get_texture_id(int i)
{
    int new_texture_id;
    unsigned char alpha;

#ifndef	CACHE_SYSTEM
    texture_cache[i].last_access_time=cur_time;
#endif	//CACHE_SYSTEM
    if(!texture_cache[i].texture_id)
        {
            alpha=texture_cache[i].alpha;
            //our texture was freed, we have to reload it
        	if(alpha==0)new_texture_id=load_bmp8_color_key(texture_cache[i].file_name);
            else
				new_texture_id=load_bmp8_fixed_alpha(texture_cache[i].file_name, alpha);
        	texture_cache[i].texture_id=new_texture_id;
        }
    return texture_cache[i].texture_id;
}

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
	int	texture_id=get_texture_id(i);
	bind_texture_id(texture_id);

	return(texture_id);
}


//load a bmp texture, in respect to the color key
GLuint load_bmp8_color_key(char * FileName)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,a,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;
	GLuint texture;

	check_gl_errors();
  	f = fopen (FileName, "rb");
  	if (!f) return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
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

	free(read_buffer);
	fclose (f);
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	bind_texture_id(texture);
	if(poor_man)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);


	check_gl_errors();

	free(file_mem_start);
	free(texture_mem);
	return texture;
}

//load a bmp texture, with the specified global alpha
GLuint load_bmp8_fixed_alpha(char * FileName, Uint8 a)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,current_pallete_entry; //i unused?
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;
	GLuint texture;

  	f = fopen (FileName, "rb");
  	if (!f) return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
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
			//fread (texture_mem+y*x_size, 1, x_size-x_padding, f);
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
					*(texture_mem+(y*x_size+x)*4+3)=a;
				}

		}

	free(file_mem_start);
	free(read_buffer);
	fclose (f);
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	check_gl_errors();
	free(texture_mem);
	return texture;
}

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

  	f = fopen (FileName, "rb");
  	if (!f) return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
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
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
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

  	f = fopen (FileName, "rb");
  	if (!f) return NULL;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
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
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return NULL;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
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

//Tests to see if a texture is already loaded. If it is, return the handle.
//If not, load it, and return the handle
int load_texture_cache(char * file_name, unsigned char alpha)
{
/*
#ifdef	CACHE_SYSTEM
	int texture_id;
	texture_cache_struct	*texture_ptr;

	// search the cache for this entry
	texture_ptr=cache_find_item(cache_texture, file_name);
	// did we find it?
	if(texture_ptr)	return(texture_ptr->texture_id);
#else	//CACHE_SYSTEM
*/
	int i, j;
	int file_name_lenght;
	int texture_id;

	file_name_lenght=strlen(file_name);

	for(i=0;i<1000;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(texture_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, texture already loaded
				return i;
		}
//#endif	//CACHE_SYSTEN

	check_gl_errors();
	//texture not found in the cache, so load it, and store it
	if(alpha==0)texture_id=load_bmp8_color_key(file_name);
	else
		texture_id=load_bmp8_fixed_alpha(file_name, alpha);
	check_gl_errors();
	if(texture_id==0)
        {
            char str[120];
            sprintf(str,"Error: Problems loading texture: %s\n",file_name);
            log_error(str);
            return 0;
        }
	//find a place to store it
	i=0;
	while(i<1000)
		{
			if(!texture_cache[i].file_name[0])//we found a place to store it
				{
					sprintf(texture_cache[i].file_name, "%s", file_name);
					texture_cache[i].texture_id=texture_id;
					texture_cache[i].alpha=alpha;
					return i;
				}
			i++;
		}

	return texture_id;
}


//load a bmp texture, and remaps it
GLuint load_bmp8_remapped_skin(char * FileName, Uint8 a, short skin, short hair, short shirt,
							   short pants, short boots)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,current_pallete_entry;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * texture_mem;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;
	GLuint texture;

  	f = fopen (FileName, "rb");
  	if (!f) return 0;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return 0;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
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

	r=g=b=0;	//keep the compiler happy
	for(y=0;y<y_size;y++)
		{
			fread (read_buffer, 1, x_size-x_padding, f);

			for(x=0;x<x_size;x++)
				{
					current_pallete_entry=*(read_buffer+x);
					if(current_pallete_entry>1 && current_pallete_entry<22)
						{
							if(current_pallete_entry>1 && current_pallete_entry<6)//skin
								{
									int intensity;
									intensity=current_pallete_entry-1;

									if(intensity==1)
										{
											r=colors_list[skin].r1;
											g=colors_list[skin].g1;
											b=colors_list[skin].b1;
										}
									else if(intensity==2)
										{
											r=colors_list[skin].r2;
											g=colors_list[skin].g2;
											b=colors_list[skin].b2;
										}
									else if(intensity==3)
										{
											r=colors_list[skin].r3;
											g=colors_list[skin].g3;
											b=colors_list[skin].b3;
										}
									else if(intensity==4)
										{
											r=colors_list[skin].r4;
											g=colors_list[skin].g4;
											b=colors_list[skin].b4;
										}

								}
							else if(current_pallete_entry>5 && current_pallete_entry<10)//hair
								{
									int intensity;
									intensity=current_pallete_entry-5;

									if(intensity==1)
										{
											r=colors_list[hair].r1;
											g=colors_list[hair].g1;
											b=colors_list[hair].b1;
										}
									else if(intensity==2)
										{
											r=colors_list[hair].r2;
											g=colors_list[hair].g2;
											b=colors_list[hair].b2;
										}
									else if(intensity==3)
										{
											r=colors_list[hair].r3;
											g=colors_list[hair].g3;
											b=colors_list[hair].b3;
										}
									else if(intensity==4)
										{
											r=colors_list[hair].r4;
											g=colors_list[hair].g4;
											b=colors_list[hair].b4;
										}

								}
							else if(current_pallete_entry>9 && current_pallete_entry<14)//shirt
								{
									int intensity;
									intensity=current_pallete_entry-9;

									if(intensity==1)
										{
											r=colors_list[shirt].r1;
											g=colors_list[shirt].g1;
											b=colors_list[shirt].b1;
										}
									else if(intensity==2)
										{
											r=colors_list[shirt].r2;
											g=colors_list[shirt].g2;
											b=colors_list[shirt].b2;
										}
									else if(intensity==3)
										{
											r=colors_list[shirt].r3;
											g=colors_list[shirt].g3;
											b=colors_list[shirt].b3;
										}
									else if(intensity==4)
										{
											r=colors_list[shirt].r4;
											g=colors_list[shirt].g4;
											b=colors_list[shirt].b4;
										}
								}
							else if(current_pallete_entry>13 && current_pallete_entry<18)//pants
								{
									int intensity;
									intensity=current_pallete_entry-13;

									if(intensity==1)
										{
											r=colors_list[pants].r1;
											g=colors_list[pants].g1;
											b=colors_list[pants].b1;
										}
									else if(intensity==2)
										{
											r=colors_list[pants].r2;
											g=colors_list[pants].g2;
											b=colors_list[pants].b2;
										}
									else if(intensity==3)
										{
											r=colors_list[pants].r3;
											g=colors_list[pants].g3;
											b=colors_list[pants].b3;
										}
									else if(intensity==4)
										{
											r=colors_list[pants].r4;
											g=colors_list[pants].g4;
											b=colors_list[pants].b4;
										}
								}

							else if(current_pallete_entry>17 && current_pallete_entry<22)//boots
								{
									int intensity;
									intensity=current_pallete_entry-17;

									if(intensity==1)
										{
											r=colors_list[boots].r1;
											g=colors_list[boots].g1;
											b=colors_list[boots].b1;
										}
									else if(intensity==2)
										{
											r=colors_list[boots].r2;
											g=colors_list[boots].g2;
											b=colors_list[boots].b2;
										}
									else if(intensity==3)
										{
											r=colors_list[boots].r3;
											g=colors_list[boots].g3;
											b=colors_list[boots].b3;
										}
									else if(intensity==4)
										{
											r=colors_list[boots].r4;
											g=colors_list[boots].g4;
											b=colors_list[boots].b4;
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
	fclose (f);
	//ok, now, hopefully, the file is loaded and converted...
	//so, assign the texture, and such

	glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,x_size, y_size,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);


	check_gl_errors();
	free(texture_mem);
	return texture;
}




void load_bmp8_to_coordinates(char * FileName, Uint8 *texture_space,int x_pos,int y_pos,
							  Uint8 alpha)
{
	int x,y,x_padding,x_size,y_size,colors_no,r,g,b,current_pallete_entry;
	Uint8 * file_mem;
	Uint8 * file_mem_start;
	Uint8 * read_buffer;
	Uint8 * color_pallete;
	FILE *f = NULL;

  	f = fopen (FileName, "rb");
  	if (!f) return;
  	file_mem = (Uint8 *) calloc ( 20000, sizeof(Uint8));
  	file_mem_start=file_mem;
  	fread (file_mem, 1, 50, f);//header only
  	//now, check to see if our bmp file is indeed a bmp file, and if it is 8 bits, uncompressed
  	if(*((short *) file_mem)!=19778)//BM (the identifier)
		{
			free(file_mem_start);
			fclose (f);
			return;
		}
	file_mem+=18;
	x_size=*((int *) file_mem);
	file_mem+=4;
	y_size=*((int *) file_mem);
	file_mem+=6;
	if(*((short *)file_mem)!=8)//8 bit/pixel?
		{
			free(file_mem_start);
			fclose (f);
			return;
		}

	file_mem+=2;
	if(*((int *)file_mem)!=0)//any compression?
		{
			free(file_mem_start);
			fclose (f);
			return;
		}
	file_mem+=16;

	colors_no=*((int *)file_mem);
	if(!colors_no)colors_no=256;
	file_mem+=8;//here comes the pallete

	color_pallete=file_mem+4;
	fread (file_mem, 1, colors_no*4+4, f);//header only
	file_mem+=colors_no*4;

	x_padding=x_size%4;
	if(x_padding)x_padding=4-x_padding;

	//now, allocate the memory for the file
	read_buffer = (Uint8 *) calloc ( 2000, sizeof(Uint8));



	for(y=y_size-1;y>=0;y--)
		{
			fread (read_buffer, 1, x_size+x_padding, f);
			for(x=0;x<x_size;x++)
				{
					int texture_y;
					texture_y=(255-(y+y_pos));
					current_pallete_entry=*(read_buffer+x);
					b=*(color_pallete+current_pallete_entry*4);
					g=*(color_pallete+current_pallete_entry*4+1);
					r=*(color_pallete+current_pallete_entry*4+2);
					*(texture_space+(texture_y*256+x+x_pos)*4)=r;
					*(texture_space+(texture_y*256+x+x_pos)*4+1)=g;
					*(texture_space+(texture_y*256+x+x_pos)*4+2)=b;
					*(texture_space+(texture_y*256+x+x_pos)*4+3)=alpha;
				}

		}

	free(file_mem_start);
	free(read_buffer);
	fclose (f);
}


#ifdef	ELC
int load_bmp8_enhanced_actor(enhanced_actor *this_actor, Uint8 a)
{
	GLuint texture;
	Uint8 * texture_mem;

	texture_mem=(Uint8*)calloc(1,256*256*4);
	if(this_actor->pants_tex[0])load_bmp8_to_coordinates(this_actor->pants_tex,texture_mem,78,175,a);
	if(this_actor->boots_tex[0])load_bmp8_to_coordinates(this_actor->boots_tex,texture_mem,0,175,a);
	if(this_actor->torso_tex[0])load_bmp8_to_coordinates(this_actor->torso_tex,texture_mem,158,156,a);
	if(this_actor->arms_tex[0])load_bmp8_to_coordinates(this_actor->arms_tex,texture_mem,0,96,a);
	if(this_actor->hands_tex[0])load_bmp8_to_coordinates(this_actor->hands_tex,texture_mem,67,64,a);
	if(this_actor->head_tex[0])load_bmp8_to_coordinates(this_actor->head_tex,texture_mem,67,0,a);
	if(this_actor->hair_tex[0])load_bmp8_to_coordinates(this_actor->hair_tex,texture_mem,0,0,a);
	if(this_actor->weapon_tex[0])load_bmp8_to_coordinates(this_actor->weapon_tex,texture_mem,158,77,a);
	if(this_actor->shield_tex[0])load_bmp8_to_coordinates(this_actor->shield_tex,texture_mem,80,96,a);
	if(this_actor->helmet_tex[0])load_bmp8_to_coordinates(this_actor->helmet_tex,texture_mem,80,149,a);
	if(this_actor->cape_tex[0])load_bmp8_to_coordinates(this_actor->cape_tex,texture_mem,131,0,a);

	glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	bind_texture_id(texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(poor_man)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256, 256,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_mem);

	check_gl_errors();
	free(texture_mem);
	return texture;
}
#endif	//ELC


