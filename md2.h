#ifndef __file_md2_H__
#define __file_md2_H__

typedef struct
{
	int magic;
	int version;
	int skinWidth;
	int skinHeight;
	int frameSize;
	int numSkins;
	int numVertices;
	int numTexCoords;
	int numFaces;
	int numGlCommands;
	int numFrames;
	int offsetSkins;
	int offsetTexCoords;
	int offsetFaces;
	int offsetFrames;
	int offsetGlCommands;
	int offsetEnd;
} header_file_md2;

typedef struct
{
	unsigned char vertex[3];
	char lightNormalIndex;
} vertex_file_md2;

typedef struct
{
	float scale[3];
	float translate[3];
	char name[16];
	vertex_file_md2 vertices[1];
} frame_file_md2;

typedef struct
{
	short vertexIndices[3];
	short textureIndices[3];
} faces_file_md2;

typedef struct
{
	short u, v;
} textureCoordinate_file_md2;

typedef struct
{
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;

}bounding_box_file_md2;


//now, we put our own structures (on how we'll store the md2 file)
typedef struct
{
	short a;//a triangle
	short b;//b triangle
	short c;//c triangle
	short at;//a texture
	short bt;//b texture
	short ct;//b texture
} face_md2;

typedef struct
{
	float x;
	float y;
	float z;
} vertex_md2;

typedef struct
{
	float u;
	float v;
} text_coord_md2;

typedef struct
{
	char name[16];
	vertex_md2 *vertex_pointer;
	bounding_box_file_md2 box;
	vertex_md2 *vertex_array;
} frame_md2;

typedef struct
{
	Uint32 numVertices;
	Uint32 numTexCoords;
	Uint32 numFaces;
	Uint32 numFrames;
	text_coord_md2 *offsetTexCoords;
	face_md2 *offsetFaces;
	frame_md2 *offsetFrames;
	text_coord_md2	*text_coord_array;
    cache_item_struct	*cache_ptr;
	char file_name[128];
}md2;


//proto
md2 * load_md2_cache(char * file_name);
md2 * load_md2(char * file_name);
void destroy_md2(md2 *md2_ptr);
void free_md2(md2 *md2_ptr);
Uint32 free_md2_va(md2 *md2_ptr);
Uint32 build_md2_va(md2 *cur_md2, frame_md2 *cur_frame);

#endif
