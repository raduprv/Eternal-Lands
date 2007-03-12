/*!
 * \file
 * \ingroup display_2d
 * \brief loading, handling and displaying MD2 files.
 */
#ifndef __FILE_MD2_H__
#define __FILE_MD2_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Contains the header information of an MD2 file
 */
typedef struct
{
	int magic; /*!< file magic number */
	int version; /*!< MD2 version number */
    
    /*! \name Dimensions of the skin 
     * @{ */
	int skinWidth; /*!< width of the skin */
	int skinHeight; /*!< height of the skin */
    /*! @} */
    
	int frameSize; /*!< size of the frame */
    
    /*! \name Number of different objects declared in the file 
     * @{ */
	int numSkins; /*!< number of skins declared in the file */
	int numVertices; /*!< number of vertices declared in the file */
	int numTexCoords; /*!< number of texture coordinates declared in the file */
	int numFaces; /*!< number of faces declared in the file */
	int numGlCommands; /*!< number of OpenGL commands declared in the file */
	int numFrames; /*!< number of frames declares in the file */
    /*! @} */
    
    /*! \name Offsets of the different declared objects into the file. The offset is started after the header declaration. 
     * @{ */
	int offsetSkins; /*!< offset where skin declarations start */
	int offsetTexCoords; /*!< offset where texture coordinate declarations start */
	int offsetFaces; /*!< offset where face declarations start */
	int offsetFrames; /*!< offset where frame declarations start */
	int offsetGlCommands; /*!< offset where OpenGL command declarations start */
	int offsetEnd; /*!< offset after the header, where the file ends */
    /*! @} */
  
} header_file_md2;

/*!
 * A vertex file structure for a MD2 file
 */
typedef struct
{
	unsigned char vertex[3]; /*!< x, y and z coordinates for the vertex */
	char lightNormalIndex; /*!< index of the light normal used by this vertex */
} vertex_file_md2;

/*!
 * A frame file structure for a MD2 file
 */
typedef struct
{
	float scale[3]; /*!< a scale factor in x, y and z for the frame */
	float translate[3]; /*!< a translation vector in x, y and z for the frame */
	char name[16]; /*!< the name of the frame */
	vertex_file_md2 vertices[1]; /*!< an array of vertices attached to this frame */
} frame_file_md2;

/*!
 * A face file structure for a MD2 file. A face consists of three vertices and a texture coordinate.
 */
typedef struct
{
	short vertexIndices[3]; /*!< indices of the three vertices that compose the face */
	short textureIndices[3]; /*!< indices of the three textures used at each the vertices */
} faces_file_md2;

/*!
 * A texture file coordinate for a MD2 file.
 */
typedef struct
{
	short u, v; /*!< u and v coordinates of the texture */
} textureCoordinate_file_md2;

/*!
 * A bounding box file structure for a MD2 file
 */
typedef struct
{
    /*! \name Minimum value of the bounding box in x, y and z direction 
     * @{ */
	float min_x;
	float min_y;
	float min_z;
    /*! @} */
    
    /*! \name Maximum value of the bounding box in x, y and z direction 
     * @{ */
	float max_x;
	float max_y;
	float max_z;
    /*! @} */
    
}bounding_box_file_md2;


//now, we put our own structures (on how we'll store the md2 file)

/*!
 * The face structure we use for MD2 faces
 */
typedef struct
{
	short a; /*!< a triangle */
	short b; /*!< b triangle */
	short c; /*!< c triangle */
	short at; /*!< a texture */
	short bt; /*!< b texture */
	short ct; /*!< c texture */
} face_md2;

/*!
 * The vertex structure we use for MD2 vertices
 */
typedef struct
{
	float x; /*!< x coordinate of the vertex */
	float y; /*!< y coordinate of the vertex */
	float z; /*!< z coordinate of the vertex */
} vertex_md2;

/*!
 * The texture coordinates we use for MD2 texture coordinates
 */
typedef struct
{
	float u; /*!< u coordinate of the texture */
	float v; /*!< v coordainte of the texture */
} text_coord_md2;

/*!
 * The frame structure we use for MD2 frames
 */
typedef struct
{
	char name[16]; /*!< the name of the frame */
	vertex_md2 *vertex_pointer; /*!< a pointer to the associated vertex */
	bounding_box_file_md2 box; /*!< the bounding box for this frame */
	vertex_md2 *vertex_array; /*!< an array of vertices that make up this frame */
} frame_md2;

/*!
 * The MD2 structure we use
 */
typedef struct
{
    /*! \name Number of different fields used 
     * @{ */
	Uint32 numVertices; /*!< number of vertices */
	Uint32 numTexCoords; /*!< number of texture coordinates */
	Uint32 numFaces; /*!< number of faces */
	Uint32 numFrames; /*!< number of frames */
    /*! @} */
    
    /*! \name Offsets for differend fields used. Note that vertices don't have an offset, as they immediately start after the header. 
     * @{ */
	text_coord_md2 *offsetTexCoords; /*!< offset of the texture coordinates */
	face_md2 *offsetFaces; /*!< offset of the faces */
	frame_md2 *offsetFrames; /*!< offset of the frames */
    /*! @} */
    
	text_coord_md2	*text_coord_array; /*!< an array of texture coordinates in the MD2 */
    cache_item_struct	*cache_ptr; /*!< a pointer to a cache item for this MD2 */
	char file_name[128];
}md2;


//proto

/*!
 * \ingroup cache
 * \brief   Loads the file given by \a file_name.
 *
 *      Loads the file given by \a file_name. The functions first looks up the cache whether the file is already loaded. If the file was not loaded before, load_md2_cache tries to load the fike and stores it in cache for future use.
 *
 * \param file_name the filename of the MD2 file to load
 * \return md2*     a pointer to the newly created \ref md2 structure, or NULL if the loading fails
 * \callgraph
 *
 * \pre If the \a file_name was loaded before, this function returns the pointer that is stored in the cache.
 * \pre If the functions fails to load the file via \ref load_md2, NULL will be returned.
 */
md2 * load_md2_cache(char * file_name);

/*!
 * \ingroup display_2d
 * \brief   Frees the memory that is used by \a md2_ptr.
 *
 *      Frees the memory that is used by the \ref md2 object pointed to by \a md2_ptr.
 *
 * \param md2_ptr   the pointer to an \ref md2 object to free.
 *
 * \callgraph
 * \note This function simply calls \ref free_md2 with the given parameter \a md2_ptr.
 */
void destroy_md2(md2 *md2_ptr);

/*!
 * \ingroup display_2d
 * \brief   Frees the memory that is used by the vertex arrays of the given \a md2_ptr.
 *
 *      Frees the memory used by the vertex arrays of the \ref md2 object pointed to by \a md2_ptr. The amount of memory freed will be returned.
 *
 * \param md2_ptr   a pointer to an \ref md2 object to free
 * \return Uint32   the amount of memory freed
 */
Uint32 free_md2_va(md2 *md2_ptr);

/*!
 * \ingroup display_2d
 * \brief   Builds the vertex arrays for the given \ref md2 object \a cur_md2 and the \ref frame_md2 \a cur_frame
 *
 *      Builds the vertex arrays for the given \ref md2 object \a cur_md2 and the \ref frame_md2 \a cur_frame. The built vertex arrays will be stored in the cache for future use.
 *
 * \param cur_md2       the \ref md2 object for which to build the vertex arrays
 * \param cur_frame     the \ref frame_md2 frame object for which to build the vertex arrays
 * \return Uint32       the amount of memory used by the built vertex arrays.
 * \callgraph
 */
Uint32 build_md2_va(md2 *cur_md2, frame_md2 *cur_frame);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
