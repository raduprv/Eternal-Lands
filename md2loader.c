#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

int md2_mem_used;
//Tests to see if a MD2 is already loaded. If it is, return the handle.
//If not, load it, and return the handle
md2 * load_md2_cache(char * file_name)
{
	md2 * md2_id;

	//do we have it already?
	md2_id= cache_find_item(cache_md2, file_name);
	if(md2_id) return(md2_id);
	//md2 not found in the cache, so load it, and store it
	md2_id= load_md2(file_name);
	if(md2_id==NULL) return NULL;
	//and remember it
	md2_id->cache_ptr= cache_add_item(cache_md2, md2_id->file_name, md2_id, md2_mem_used+sizeof(*md2_id));

	return md2_id;
}

md2 * load_md2(char * file_name)
{
	int i,k;
	FILE *f = NULL;

	face_md2 *face_pointer;
	faces_file_md2 *file_face_pointer;

	text_coord_md2 *text_coord_pointer;
	textureCoordinate_file_md2 *file_text_coord_pointer;

	frame_md2 *frame_pointer;
	unsigned char *file_frame_pointer;

	header_file_md2 file_header;
	int frame_pointer_offset;

	float scale_x;
	float scale_y;
	float scale_z;

	float translate_x;
	float translate_y;
	float translate_z;

	float x,y,z;
	float x_offset=0,y_offset=0,z_offset=0;

	//we need our own structure, since the frame size is variable
	struct temp_frame_storage_s
		{
			float scale[3];
			float translate[3];
			char name[16];
			vertex_file_md2 vertices[2048];
		};
	struct temp_frame_storage_s temp_frame_storage;
	char *temp_frame_storage_pointer;
	md2 *our_md2;
	int first_box= 0;

	temp_frame_storage_pointer= (char *)&temp_frame_storage;

	f = fopen (file_name, "rb");
	if(!f)
		{
			char str[120];
			sprintf(str,"%s: %s: %s\n",reg_error_str,cant_open_file,file_name);
			log_error(str);
			return NULL;
		}
	fread (&file_header, 1, sizeof(header_file_md2), f);

	//allocate the main memory
	our_md2 = calloc(1, sizeof(md2));
	my_strncp(our_md2->file_name, file_name, 128);
	md2_mem_used = 0;

	//now, get the faces
	//alocate the memory for the faces
	file_face_pointer = calloc(file_header.numFaces, sizeof(faces_file_md2));
	face_pointer = calloc(file_header.numFaces, sizeof(face_md2));
	md2_mem_used += file_header.numFaces*sizeof(face_md2);
	//read the faces, from the file
	fseek (f, file_header.offsetFaces, SEEK_SET);
	fread (file_face_pointer, 1, sizeof(faces_file_md2)*file_header.numFaces, f);
	//convert them from the md2 format to our format
	for(i=0;i<file_header.numFaces;i++)
		{
			face_pointer[i].a= file_face_pointer[i].vertexIndices[0];
			face_pointer[i].b= file_face_pointer[i].vertexIndices[1];
			face_pointer[i].c= file_face_pointer[i].vertexIndices[2];

			face_pointer[i].at= file_face_pointer[i].textureIndices[0];
			face_pointer[i].bt= file_face_pointer[i].textureIndices[1];
			face_pointer[i].ct= file_face_pointer[i].textureIndices[2];
		}
	free(file_face_pointer);

	//now, convert the UV coordinates
	//alocate the memory for the texture coordinates
	file_text_coord_pointer = calloc(file_header.numTexCoords, sizeof(textureCoordinate_file_md2));
	text_coord_pointer = calloc(file_header.numTexCoords, sizeof(text_coord_md2));
	md2_mem_used += file_header.numTexCoords*sizeof(text_coord_md2);
	//read the coords, from the file
	fseek (f, file_header.offsetTexCoords, SEEK_SET);
	fread (file_text_coord_pointer, 1, sizeof(textureCoordinate_file_md2)*file_header.numTexCoords, f);
	for(i=0;i<file_header.numTexCoords;i++)
		{
			text_coord_pointer[i].u= (float)file_text_coord_pointer[i].u/(float)file_header.skinWidth;
			text_coord_pointer[i].v= 1.0f-(float)file_text_coord_pointer[i].v/(float)file_header.skinHeight;
		}
	free(file_text_coord_pointer);

	/*
	Now, the complicated thing: Load each frame, and convert the vertices, and the
	frame info to our format.
	*/

	file_frame_pointer = calloc(file_header.numFrames*file_header.frameSize, sizeof(char));
	frame_pointer = calloc(file_header.numFrames, sizeof(frame_md2));
	md2_mem_used += file_header.numFrames*sizeof(frame_md2);
	//read the coords, from the file
	fseek (f, file_header.offsetFrames, SEEK_SET);
	fread (file_frame_pointer, 1, file_header.frameSize*file_header.numFrames, f);

	for(i=0;i<file_header.numFrames;i++)
		{
		vertex_md2 *some_pointer;

		//alocate some memory for OUR list of vertices
		some_pointer= calloc(file_header.numVertices, sizeof(vertex_md2));
		frame_pointer[i].vertex_pointer= some_pointer;
		md2_mem_used += file_header.numVertices*sizeof(vertex_md2);

		//read the current frame into our temp structure
		frame_pointer_offset= file_header.frameSize*i;
		for(k=0;k<file_header.frameSize;k++)
			*(temp_frame_storage_pointer+k)= *(file_frame_pointer+frame_pointer_offset+k);


		scale_x= temp_frame_storage.scale[0];
		scale_y= temp_frame_storage.scale[1];
		scale_z= temp_frame_storage.scale[2];

		translate_x= temp_frame_storage.translate[0];
		translate_y= temp_frame_storage.translate[1];
		translate_z= temp_frame_storage.translate[2];

		//put the frame name too
		//sprintf(frame_pointer[i].name,"%s",temp_frame_storage.name);
		strcpy(frame_pointer[i].name, temp_frame_storage.name);

		//clear the bounding box
		frame_pointer[i].box.min_x= 1200.0f;
		frame_pointer[i].box.min_y= 1200.0f;
		frame_pointer[i].box.min_z= 1200.0f;

		frame_pointer[i].box.max_x= -1200.0f;
		frame_pointer[i].box.max_y= -1200.0f;
		frame_pointer[i].box.max_z= -1200.0f;

		//now, convert the vertices to our format
		for(k=0;k<file_header.numVertices;k++)
			{
				x= ((float)temp_frame_storage.vertices[k].vertex[0]*scale_x+translate_x)/20.f;
				y= ((float)temp_frame_storage.vertices[k].vertex[1]*scale_y+translate_y)/20.f;
				z= ((float)temp_frame_storage.vertices[k].vertex[2]*scale_z+translate_z)/20.f;

				some_pointer[k].x= x;
				some_pointer[k].y= y;
				some_pointer[k].z= z;

				//do the bounding box
				if(frame_pointer[i].box.min_x>x)frame_pointer[i].box.min_x= x;
				if(frame_pointer[i].box.max_x<x)frame_pointer[i].box.max_x= x;

				if(frame_pointer[i].box.min_y>y)frame_pointer[i].box.min_y= y;
				if(frame_pointer[i].box.max_y<y)frame_pointer[i].box.max_y= y;

				if(frame_pointer[i].box.min_z>z)frame_pointer[i].box.min_z= z;
				if(frame_pointer[i].box.max_z<z)frame_pointer[i].box.max_z= z;
			}

		//make sure the object is centered around the 0,0,0 coordinates
		if(!first_box)
			{
				x_offset= 0-(frame_pointer[i].box.min_x+(frame_pointer[i].box.max_x-frame_pointer[i].box.min_x)/2);
				y_offset= 0-(frame_pointer[i].box.min_y+(frame_pointer[i].box.max_y-frame_pointer[i].box.min_y)/2);
				first_box=1;
			}
		z_offset=0-frame_pointer[i].box.min_z;
		if(!no_bounding_box)
			{
				for(k=0;k<file_header.numVertices;k++)
					{
						some_pointer[k].x+= x_offset;
						some_pointer[k].y+= y_offset;
						some_pointer[k].z+= z_offset;
					}

				frame_pointer[i].box.min_x+= x_offset;
				frame_pointer[i].box.min_y+= y_offset;
				frame_pointer[i].box.min_z+= z_offset;

				frame_pointer[i].box.max_x+= x_offset;
				frame_pointer[i].box.max_y+= y_offset;
				frame_pointer[i].box.max_z+= z_offset;
			}
		}
	free(file_frame_pointer);

	our_md2->numVertices= file_header.numVertices;
	our_md2->numFaces= file_header.numFaces;
	our_md2->numFrames= file_header.numFrames;
	our_md2->numTexCoords= file_header.numTexCoords;

	our_md2->offsetFrames= frame_pointer;
	our_md2->offsetFaces= face_pointer;
	our_md2->offsetTexCoords= text_coord_pointer;

	//close the file, at exit
	fclose (f);
	return our_md2;
}

Uint32 build_md2_va(md2 *cur_md2, frame_md2 *cur_frame)
{
	Uint32	used= 0;
	Uint32	j,k;

	if(use_vertex_array > 0 && vertex_arrays_built < use_vertex_array)
		{
			// do we need the texture array populated?
			if (!cur_md2->text_coord_array)
				{
					// allocate the space for a full texture coord array
					cur_md2->text_coord_array= calloc(cur_md2->numFaces, sizeof(text_coord_md2)*3);
					used += cur_md2->numFaces*sizeof(text_coord_md2)*3;
					vertex_arrays_built++;
					//now, convert the coords to VA format
					for(j=k=0;k<cur_md2->numFaces;j+=3,k++)
						{
							cur_md2->text_coord_array[j].u= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].at].u;
							cur_md2->text_coord_array[j].v= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].at].v;
							cur_md2->text_coord_array[j+1].u= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].bt].u;
							cur_md2->text_coord_array[j+1].v= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].bt].v;
							cur_md2->text_coord_array[j+2].u= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].ct].u;
							cur_md2->text_coord_array[j+2].v= cur_md2->offsetTexCoords[cur_md2->offsetFaces[k].ct].v;
						}
				}
			//do we need to allocate the VA array?
			if(!cur_frame->vertex_array)
				{
					//now, convert the vertices to VA format
					cur_frame->vertex_array= calloc(cur_md2->numFaces, sizeof(vertex_md2)*3);
					used += cur_md2->numFaces*sizeof(vertex_md2)*3;
				}
			vertex_arrays_built++;
			for(j=k=0;k<cur_md2->numFaces;j+=3,k++)
				{
					cur_frame->vertex_array[j].x= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].a].x;
					cur_frame->vertex_array[j].y= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].a].y;
					cur_frame->vertex_array[j].z= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].a].z;
					cur_frame->vertex_array[j+1].x= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].b].x;
					cur_frame->vertex_array[j+1].y= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].b].y;
					cur_frame->vertex_array[j+1].z= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].b].z;
					cur_frame->vertex_array[j+2].x= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].c].x;
					cur_frame->vertex_array[j+2].y= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].c].y;
					cur_frame->vertex_array[j+2].z= cur_frame->vertex_pointer[cur_md2->offsetFaces[k].c].z;
				}
		}
	cache_adj_size(cache_md2, used, cur_md2);

	return used;
}

void free_md2(md2 *md2_ptr)
{
	Uint32	i;

	// free the per frame information
	for(i=0; i<md2_ptr->numFrames; i++)
		{
			if(md2_ptr->offsetFrames[i].vertex_pointer) free(md2_ptr->offsetFrames[i].vertex_pointer);
		}
	// release the VA data if it was allocated
	free_md2_va(md2_ptr);
	// release the rest of the memory we allocated
	if(md2_ptr->offsetTexCoords) free(md2_ptr->offsetTexCoords);
	if(md2_ptr->offsetFaces) free(md2_ptr->offsetFaces);
	if(md2_ptr->offsetFrames) free(md2_ptr->offsetFrames);
}

Uint32 free_md2_va(md2 *md2_ptr)
{
	Uint32	mem=0;
	Uint32	i;

	for(i=0; i<md2_ptr->numFrames; i++)
		{
			if(md2_ptr->offsetFrames[i].vertex_array)
				{
					// track how much memory we are freeing
					mem+= md2_ptr->numFaces*sizeof(vertex_md2)*3;
					// free the memory and clear the pointer
					free(md2_ptr->offsetFrames[i].vertex_array);
					md2_ptr->offsetFrames[i].vertex_array=NULL;
				}
		}
	if(md2_ptr->text_coord_array)
		{
			// track how much memory we are freeing
			mem+= md2_ptr->numFaces*sizeof(text_coord_md2)*3;
			// free the memory and clear the pointer
			free(md2_ptr->text_coord_array);
			md2_ptr->text_coord_array=NULL;
		}

	return	mem;
}

void destroy_md2(md2 *md2_ptr)
{
	// release the MD2 memory
	free_md2(md2_ptr);
}

