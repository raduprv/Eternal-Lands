#include <stdlib.h>
#include <math.h>
#include "global.h"

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
	int first_box=0;

	temp_frame_storage_pointer=(char *)&temp_frame_storage;
	our_md2 = calloc(1, sizeof(md2));

	f = fopen (file_name, "rb");
	if(!f)
		{
			char str[120];
			sprintf(str,"Error: Can't open file: %s\n",file_name);
			log_error(str);
			return NULL;
		}
	fread (&file_header, 1, sizeof(header_file_md2), f);

	//now, get the faces
	//alocate the memory for the faces
	file_face_pointer = calloc(file_header.numFaces, sizeof(faces_file_md2));
	face_pointer = calloc(file_header.numFaces, sizeof(face_md2));
	//read the faces, from the file
	fseek (f, file_header.offsetFaces, SEEK_SET);
	fread (file_face_pointer, 1, sizeof(faces_file_md2)*file_header.numFaces, f);
	//convert them from the md2 format to our format
	for(i=0;i<file_header.numFaces;i++)
		{
			face_pointer[i].a=file_face_pointer[i].vertexIndices[0];
			face_pointer[i].b=file_face_pointer[i].vertexIndices[1];
			face_pointer[i].c=file_face_pointer[i].vertexIndices[2];

			face_pointer[i].at=file_face_pointer[i].textureIndices[0];
			face_pointer[i].bt=file_face_pointer[i].textureIndices[1];
			face_pointer[i].ct=file_face_pointer[i].textureIndices[2];
		}
	free(file_face_pointer);

	//now, convert the UV coordinates
	//alocate the memory for the texture coordinates
	file_text_coord_pointer = calloc(file_header.numTexCoords, sizeof(textureCoordinate_file_md2));
	text_coord_pointer = calloc(file_header.numTexCoords, sizeof(text_coord_md2));
	//read the coords, from the file
	fseek (f, file_header.offsetTexCoords, SEEK_SET);
	fread (file_text_coord_pointer, 1, sizeof(textureCoordinate_file_md2)*file_header.numTexCoords, f);
	for(i=0;i<file_header.numTexCoords;i++)
		{
			text_coord_pointer[i].u=(float)file_text_coord_pointer[i].u/(float)file_header.skinWidth;
			text_coord_pointer[i].v=1.0f-(float)file_text_coord_pointer[i].v/(float)file_header.skinHeight;
		}
	free(file_text_coord_pointer);
#ifdef	USE_VERTEXARRAYS
	if(use_vertex_array)
		{
			// allocate the space for a full texture coord array
			our_md2->text_coord_array= calloc(file_header.numFaces, sizeof(text_coord_md2)*3);
			//now, convert the coords to VA format
			for(k=0;k<file_header.numFaces;k++)
				{
					our_md2->text_coord_array[k*3].u=text_coord_pointer[face_pointer[k].at].u;
					our_md2->text_coord_array[k*3].v=text_coord_pointer[face_pointer[k].at].v;
					our_md2->text_coord_array[k*3+1].u=text_coord_pointer[face_pointer[k].bt].u;
					our_md2->text_coord_array[k*3+1].v=text_coord_pointer[face_pointer[k].bt].v;
					our_md2->text_coord_array[k*3+2].u=text_coord_pointer[face_pointer[k].ct].u;
					our_md2->text_coord_array[k*3+2].v=text_coord_pointer[face_pointer[k].ct].v;
				}
		}
#endif	//USE_VERTEXARRAYS

	/*
	Now, the complicated thing: Load each frame, and convert the vertices, and the
	frame info to our format.
	*/

	file_frame_pointer = calloc(file_header.numFrames*file_header.frameSize, sizeof(char));
	frame_pointer = calloc(file_header.numFrames, sizeof(frame_md2));
	//read the coords, from the file
	fseek (f, file_header.offsetFrames, SEEK_SET);
	fread (file_frame_pointer, 1, file_header.frameSize*file_header.numFrames, f);

	for(i=0;i<file_header.numFrames;i++)
		{
		vertex_md2 *some_pointer;

		//alocate some memory for OUR list of vertices
		some_pointer=calloc(file_header.numVertices, sizeof(vertex_md2));
		frame_pointer[i].vertex_pointer=some_pointer;

		//read the current frame into our temp structure
		frame_pointer_offset=file_header.frameSize*i;
		for(k=0;k<file_header.frameSize;k++)
		*(temp_frame_storage_pointer+k)=*(file_frame_pointer+frame_pointer_offset+k);


		scale_x=temp_frame_storage.scale[0];
		scale_y=temp_frame_storage.scale[1];
		scale_z=temp_frame_storage.scale[2];

		translate_x=temp_frame_storage.translate[0];
		translate_y=temp_frame_storage.translate[1];
		translate_z=temp_frame_storage.translate[2];

		//put the frame name too
		sprintf(frame_pointer[i].name,"%s",temp_frame_storage.name);

		//clear the bounding box
		frame_pointer[i].box.min_x=1200.0f;
		frame_pointer[i].box.min_y=1200.0f;
		frame_pointer[i].box.min_z=1200.0f;

		frame_pointer[i].box.max_x=-1200.0f;
		frame_pointer[i].box.max_y=-1200.0f;
		frame_pointer[i].box.max_z=-1200.0f;

		//now, convert the vertices to our format
		for(k=0;k<file_header.numVertices;k++)
			{
				x=((float)temp_frame_storage.vertices[k].vertex[0]*scale_x+translate_x)/20.f;
				y=((float)temp_frame_storage.vertices[k].vertex[1]*scale_y+translate_y)/20.f;
				z=((float)temp_frame_storage.vertices[k].vertex[2]*scale_z+translate_z)/20.f;

				some_pointer[k].x=x;
				some_pointer[k].y=y;
				some_pointer[k].z=z;

				//do the bounding box
				if(frame_pointer[i].box.min_x>x)frame_pointer[i].box.min_x=x;
				if(frame_pointer[i].box.max_x<x)frame_pointer[i].box.max_x=x;

				if(frame_pointer[i].box.min_y>y)frame_pointer[i].box.min_y=y;
				if(frame_pointer[i].box.max_y<y)frame_pointer[i].box.max_y=y;

				if(frame_pointer[i].box.min_z>z)frame_pointer[i].box.min_z=z;
				if(frame_pointer[i].box.max_z<z)frame_pointer[i].box.max_z=z;
			}

		//make sure the object is centered around the 0,0,0 coordinates
		if(!first_box)
			{
				x_offset=0-(frame_pointer[i].box.min_x+(frame_pointer[i].box.max_x-frame_pointer[i].box.min_x)/2);
				y_offset=0-(frame_pointer[i].box.min_y+(frame_pointer[i].box.max_y-frame_pointer[i].box.min_y)/2);
				first_box=1;
			}
		z_offset=0-frame_pointer[i].box.min_z;
		if(!no_bounding_box)
			{
				for(k=0;k<file_header.numVertices;k++)
					{
						some_pointer[k].x+=x_offset;
						some_pointer[k].y+=y_offset;
						some_pointer[k].z+=z_offset;
					}

				frame_pointer[i].box.min_x+=x_offset;
				frame_pointer[i].box.min_y+=y_offset;
				frame_pointer[i].box.min_z+=z_offset;

				frame_pointer[i].box.max_x+=x_offset;
				frame_pointer[i].box.max_y+=y_offset;
				frame_pointer[i].box.max_z+=z_offset;
			}
#ifdef	USE_VERTEXARRAYS
		if(use_vertex_array)
			{
				//now, convert the vertices to VA format
				frame_pointer[i].vertex_array= calloc(file_header.numFaces, sizeof(vertex_md2)*3);
				for(k=0;k<file_header.numFaces;k++)
					{
						frame_pointer[i].vertex_array[k*3].x=some_pointer[face_pointer[k].a].x;
						frame_pointer[i].vertex_array[k*3].y=some_pointer[face_pointer[k].a].y;
						frame_pointer[i].vertex_array[k*3].z=some_pointer[face_pointer[k].a].z;
						frame_pointer[i].vertex_array[k*3+1].x=some_pointer[face_pointer[k].b].x;
						frame_pointer[i].vertex_array[k*3+1].y=some_pointer[face_pointer[k].b].y;
						frame_pointer[i].vertex_array[k*3+1].z=some_pointer[face_pointer[k].b].z;
						frame_pointer[i].vertex_array[k*3+2].x=some_pointer[face_pointer[k].c].x;
						frame_pointer[i].vertex_array[k*3+2].y=some_pointer[face_pointer[k].c].y;
						frame_pointer[i].vertex_array[k*3+2].z=some_pointer[face_pointer[k].c].z;
					}
			}
#endif	//USE_VERTEXARRAYS
		}
	free(file_frame_pointer);

	our_md2->numVertices=file_header.numVertices;
	our_md2->numFaces=file_header.numFaces;
	our_md2->numFrames=file_header.numFrames;
	our_md2->numTexCoords=file_header.numTexCoords;

	our_md2->offsetFrames=frame_pointer;
	our_md2->offsetFaces=face_pointer;
	our_md2->offsetTexCoords=text_coord_pointer;

	//close the file, at exit
	fclose (f);
	return our_md2;
}

