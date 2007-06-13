#include <SDL.h>
#include "load_gl_extensions.h"

uint_fast32_t extensions = 0;
GLint texture_units = 0;
float max_anisotropic_filter = 1.0f;

/*	GL_ARB_multitexture	*/
PFNGLACTIVETEXTUREARBPROC ELglActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC ELglClientActiveTextureARB;
PFNGLMULTITEXCOORD1DARBPROC ELglMultiTexCoord1dARB;
PFNGLMULTITEXCOORD1DVARBPROC ELglMultiTexCoord1dvARB;
PFNGLMULTITEXCOORD1FARBPROC ELglMultiTexCoord1fARB;
PFNGLMULTITEXCOORD1FVARBPROC ELglMultiTexCoord1fvARB;
PFNGLMULTITEXCOORD1IARBPROC ELglMultiTexCoord1iARB;
PFNGLMULTITEXCOORD1IVARBPROC ELglMultiTexCoord1ivARB;
PFNGLMULTITEXCOORD1SARBPROC ELglMultiTexCoord1sARB;
PFNGLMULTITEXCOORD1SVARBPROC ELglMultiTexCoord1svARB;
PFNGLMULTITEXCOORD2DARBPROC ELglMultiTexCoord2dARB;
PFNGLMULTITEXCOORD2DVARBPROC ELglMultiTexCoord2dvARB;
PFNGLMULTITEXCOORD2FARBPROC ELglMultiTexCoord2fARB;
PFNGLMULTITEXCOORD2FVARBPROC ELglMultiTexCoord2fvARB;
PFNGLMULTITEXCOORD2IARBPROC ELglMultiTexCoord2iARB;
PFNGLMULTITEXCOORD2IVARBPROC ELglMultiTexCoord2ivARB;
PFNGLMULTITEXCOORD2SARBPROC ELglMultiTexCoord2sARB;
PFNGLMULTITEXCOORD2SVARBPROC ELglMultiTexCoord2svARB;
PFNGLMULTITEXCOORD3DARBPROC ELglMultiTexCoord3dARB;
PFNGLMULTITEXCOORD3DVARBPROC ELglMultiTexCoord3dvARB;
PFNGLMULTITEXCOORD3FARBPROC ELglMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3FVARBPROC ELglMultiTexCoord3fvARB;
PFNGLMULTITEXCOORD3IARBPROC ELglMultiTexCoord3iARB;
PFNGLMULTITEXCOORD3IVARBPROC ELglMultiTexCoord3ivARB;
PFNGLMULTITEXCOORD3SARBPROC ELglMultiTexCoord3sARB;
PFNGLMULTITEXCOORD3SVARBPROC ELglMultiTexCoord3svARB;
PFNGLMULTITEXCOORD4DARBPROC ELglMultiTexCoord4dARB;
PFNGLMULTITEXCOORD4DVARBPROC ELglMultiTexCoord4dvARB;
PFNGLMULTITEXCOORD4FARBPROC ELglMultiTexCoord4fARB;
PFNGLMULTITEXCOORD4FVARBPROC ELglMultiTexCoord4fvARB;
PFNGLMULTITEXCOORD4IARBPROC ELglMultiTexCoord4iARB;
PFNGLMULTITEXCOORD4IVARBPROC ELglMultiTexCoord4ivARB;
PFNGLMULTITEXCOORD4SARBPROC ELglMultiTexCoord4sARB;
PFNGLMULTITEXCOORD4SVARBPROC ELglMultiTexCoord4svARB;
/*	GL_ARB_multitexture	*/

/*	GL_EXT_compiled_vertex_array	*/
PFNGLLOCKARRAYSEXTPROC ELglLockArraysEXT;
PFNGLUNLOCKARRAYSEXTPROC ELglUnlockArraysEXT;
/*	GL_EXT_compiled_vertex_array	*/

/*	GL_EXT_draw_range_elements	*/
PFNGLDRAWRANGEELEMENTSEXTPROC ELglDrawRangeElementsEXT;
/*	GL_EXT_draw_range_elements	*/

/*	GL_ARB_point_parameters		*/
PFNGLPOINTPARAMETERFARBPROC ELglPointParameterfARB;
PFNGLPOINTPARAMETERFVARBPROC ELglPointParameterfvARB;
/*	GL_ARB_point_parameters		*/

/*	GL_ARB_vertex_buffer_object	*/
PFNGLBINDBUFFERARBPROC ELglBindBufferARB;
PFNGLBUFFERDATAARBPROC ELglBufferDataARB;
PFNGLBUFFERSUBDATAARBPROC ELglBufferSubDataARB;
PFNGLDELETEBUFFERSARBPROC ELglDeleteBuffersARB;
PFNGLGENBUFFERSARBPROC ELglGenBuffersARB;
PFNGLGETBUFFERPARAMETERIVARBPROC ELglGetBufferParameterivARB;
PFNGLGETBUFFERPOINTERVARBPROC ELglGetBufferPointervARB;
PFNGLGETBUFFERSUBDATAARBPROC ELglGetBufferSubDataARB;
PFNGLISBUFFERARBPROC ELglIsBufferARB;
PFNGLMAPBUFFERARBPROC ELglMapBufferARB;
PFNGLUNMAPBUFFERARBPROC ELglUnmapBufferARB;
/*	GL_ARB_vertex_buffer_object	*/

/*	GL_EXT_framebuffer_object	*/
PFNGLBINDFRAMEBUFFEREXTPROC ELglBindFramebufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC ELglBindRenderbufferEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC ELglCheckFramebufferStatusEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC ELglDeleteFramebuffersEXT;
PFNGLDELETERENDERBUFFERSEXTPROC ELglDeleteRenderbuffersEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC ELglFramebufferRenderbufferEXT;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC ELglFramebufferTexture1DEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC ELglFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC ELglFramebufferTexture3DEXT;
PFNGLGENFRAMEBUFFERSEXTPROC ELglGenFramebuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC ELglGenRenderbuffersEXT;
PFNGLGENERATEMIPMAPEXTPROC ELglGenerateMipmapEXT;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC ELglGetFramebufferAttachmentParameterivEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC ELglGetRenderbufferParameterivEXT;
PFNGLISFRAMEBUFFEREXTPROC ELglIsFramebufferEXT;
PFNGLISRENDERBUFFEREXTPROC ELglIsRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC ELglRenderbufferStorageEXT;	
/*	GL_EXT_framebuffer_object	*/

/*	GL_ARB_texture_compression	*/
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC ELglCompressedTexImage1DARB;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC ELglCompressedTexImage2DARB;
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC ELglCompressedTexImage3DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC ELglCompressedTexSubImage1DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC ELglCompressedTexSubImage2DARB;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC ELglCompressedTexSubImage3DARB;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC ELglGetCompressedTexImageARB;
/*	GL_ARB_texture_compression	*/

/*	GL_ARB_occlusion_query		*/
PFNGLBEGINQUERYARBPROC ELglBeginQueryARB;
PFNGLDELETEQUERIESARBPROC ELglDeleteQueriesARB;
PFNGLENDQUERYARBPROC ELglEndQueryARB;
PFNGLGENQUERIESARBPROC ELglGenQueriesARB;
PFNGLGETQUERYOBJECTIVARBPROC ELglGetQueryObjectivARB;
PFNGLGETQUERYOBJECTUIVARBPROC ELglGetQueryObjectuivARB;
PFNGLGETQUERYIVARBPROC ELglGetQueryivARB;
PFNGLISQUERYARBPROC ELglIsQueryARB;
/*	GL_ARB_occlusion_query		*/

/*	GL_ARB_vertex_program		*/
PFNGLBINDPROGRAMARBPROC ELglBindProgramARB;
PFNGLDELETEPROGRAMSARBPROC ELglDeleteProgramsARB;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC ELglDisableVertexAttribArrayARB;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC ELglEnableVertexAttribArrayARB;
PFNGLGENPROGRAMSARBPROC ELglGenProgramsARB;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC ELglGetProgramEnvParameterdvARB;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC ELglGetProgramEnvParameterfvARB;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC ELglGetProgramLocalParameterdvARB;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC ELglGetProgramLocalParameterfvARB;
PFNGLGETPROGRAMSTRINGARBPROC ELglGetProgramStringARB;
PFNGLGETPROGRAMIVARBPROC ELglGetProgramivARB;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC ELglGetVertexAttribPointervARB;
PFNGLGETVERTEXATTRIBDVARBPROC ELglGetVertexAttribdvARB;
PFNGLGETVERTEXATTRIBFVARBPROC ELglGetVertexAttribfvARB;
PFNGLGETVERTEXATTRIBIVARBPROC ELglGetVertexAttribivARB;
PFNGLISPROGRAMARBPROC ELglIsProgramARB;
PFNGLPROGRAMENVPARAMETER4DARBPROC ELglProgramEnvParameter4dARB;
PFNGLPROGRAMENVPARAMETER4DVARBPROC ELglProgramEnvParameter4dvARB;
PFNGLPROGRAMENVPARAMETER4FARBPROC ELglProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4FVARBPROC ELglProgramEnvParameter4fvARB;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC ELglProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC ELglProgramLocalParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC ELglProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC ELglProgramLocalParameter4fvARB;
PFNGLPROGRAMSTRINGARBPROC ELglProgramStringARB;
PFNGLVERTEXATTRIB1DARBPROC ELglVertexAttrib1dARB;
PFNGLVERTEXATTRIB1DVARBPROC ELglVertexAttrib1dvARB;
PFNGLVERTEXATTRIB1FARBPROC ELglVertexAttrib1fARB;
PFNGLVERTEXATTRIB1FVARBPROC ELglVertexAttrib1fvARB;
PFNGLVERTEXATTRIB1SARBPROC ELglVertexAttrib1sARB;
PFNGLVERTEXATTRIB1SVARBPROC ELglVertexAttrib1svARB;
PFNGLVERTEXATTRIB2DARBPROC ELglVertexAttrib2dARB;
PFNGLVERTEXATTRIB2DVARBPROC ELglVertexAttrib2dvARB;
PFNGLVERTEXATTRIB2FARBPROC ELglVertexAttrib2fARB;
PFNGLVERTEXATTRIB2FVARBPROC ELglVertexAttrib2fvARB;
PFNGLVERTEXATTRIB2SARBPROC ELglVertexAttrib2sARB;
PFNGLVERTEXATTRIB2SVARBPROC ELglVertexAttrib2svARB;
PFNGLVERTEXATTRIB3DARBPROC ELglVertexAttrib3dARB;
PFNGLVERTEXATTRIB3DVARBPROC ELglVertexAttrib3dvARB;
PFNGLVERTEXATTRIB3FARBPROC ELglVertexAttrib3fARB;
PFNGLVERTEXATTRIB3FVARBPROC ELglVertexAttrib3fvARB;
PFNGLVERTEXATTRIB3SARBPROC ELglVertexAttrib3sARB;
PFNGLVERTEXATTRIB3SVARBPROC ELglVertexAttrib3svARB;
PFNGLVERTEXATTRIB4NBVARBPROC ELglVertexAttrib4NbvARB;
PFNGLVERTEXATTRIB4NIVARBPROC ELglVertexAttrib4NivARB;
PFNGLVERTEXATTRIB4NSVARBPROC ELglVertexAttrib4NsvARB;
PFNGLVERTEXATTRIB4NUBARBPROC ELglVertexAttrib4NubARB;
PFNGLVERTEXATTRIB4NUBVARBPROC ELglVertexAttrib4NubvARB;
PFNGLVERTEXATTRIB4NUIVARBPROC ELglVertexAttrib4NuivARB;
PFNGLVERTEXATTRIB4NUSVARBPROC ELglVertexAttrib4NusvARB;
PFNGLVERTEXATTRIB4BVARBPROC ELglVertexAttrib4bvARB;
PFNGLVERTEXATTRIB4DARBPROC ELglVertexAttrib4dARB;
PFNGLVERTEXATTRIB4DVARBPROC ELglVertexAttrib4dvARB;
PFNGLVERTEXATTRIB4FARBPROC ELglVertexAttrib4fARB;
PFNGLVERTEXATTRIB4FVARBPROC ELglVertexAttrib4fvARB;
PFNGLVERTEXATTRIB4IVARBPROC ELglVertexAttrib4ivARB;
PFNGLVERTEXATTRIB4SARBPROC ELglVertexAttrib4sARB;
PFNGLVERTEXATTRIB4SVARBPROC ELglVertexAttrib4svARB;
PFNGLVERTEXATTRIB4UBVARBPROC ELglVertexAttrib4ubvARB;
PFNGLVERTEXATTRIB4UIVARBPROC ELglVertexAttrib4uivARB;
PFNGLVERTEXATTRIB4USVARBPROC ELglVertexAttrib4usvARB;
PFNGLVERTEXATTRIBPOINTERARBPROC ELglVertexAttribPointerARB;
/*	GL_ARB_vertex_program		*/

/*	GL_ARB_vertex_shader		*/
PFNGLBINDATTRIBLOCATIONARBPROC ELglBindAttribLocationARB;
PFNGLGETACTIVEATTRIBARBPROC ELglGetActiveAttribARB;
PFNGLGETATTRIBLOCATIONARBPROC ELglGetAttribLocationARB;
/*	GL_ARB_vertex_shader		*/

/*	GL_ARB_shader_objects		*/
PFNGLATTACHOBJECTARBPROC ELglAttachObjectARB;
PFNGLCOMPILESHADERARBPROC ELglCompileShaderARB;
PFNGLCREATEPROGRAMOBJECTARBPROC ELglCreateProgramObjectARB;
PFNGLCREATESHADEROBJECTARBPROC ELglCreateShaderObjectARB;
PFNGLDELETEOBJECTARBPROC ELglDeleteObjectARB;
PFNGLDETACHOBJECTARBPROC ELglDetachObjectARB;
PFNGLGETACTIVEUNIFORMARBPROC ELglGetActiveUniformARB;
PFNGLGETATTACHEDOBJECTSARBPROC ELglGetAttachedObjectsARB;
PFNGLGETHANDLEARBPROC ELglGetHandleARB;
PFNGLGETINFOLOGARBPROC ELglGetInfoLogARB;
PFNGLGETOBJECTPARAMETERFVARBPROC ELglGetObjectParameterfvARB;
PFNGLGETOBJECTPARAMETERIVARBPROC ELglGetObjectParameterivARB;
PFNGLGETSHADERSOURCEARBPROC ELglGetShaderSourceARB;
PFNGLGETUNIFORMLOCATIONARBPROC ELglGetUniformLocationARB;
PFNGLGETUNIFORMFVARBPROC ELglGetUniformfvARB;
PFNGLGETUNIFORMIVARBPROC ELglGetUniformivARB;
PFNGLLINKPROGRAMARBPROC ELglLinkProgramARB;
PFNGLSHADERSOURCEARBPROC ELglShaderSourceARB;
PFNGLUNIFORM1FARBPROC ELglUniform1fARB;
PFNGLUNIFORM1FVARBPROC ELglUniform1fvARB;
PFNGLUNIFORM1IARBPROC ELglUniform1iARB;
PFNGLUNIFORM1IVARBPROC ELglUniform1ivARB;
PFNGLUNIFORM2FARBPROC ELglUniform2fARB;
PFNGLUNIFORM2FVARBPROC ELglUniform2fvARB;
PFNGLUNIFORM2IARBPROC ELglUniform2iARB;
PFNGLUNIFORM2IVARBPROC ELglUniform2ivARB;
PFNGLUNIFORM3FARBPROC ELglUniform3fARB;
PFNGLUNIFORM3FVARBPROC ELglUniform3fvARB;
PFNGLUNIFORM3IARBPROC ELglUniform3iARB;
PFNGLUNIFORM3IVARBPROC ELglUniform3ivARB;
PFNGLUNIFORM4FARBPROC ELglUniform4fARB;
PFNGLUNIFORM4FVARBPROC ELglUniform4fvARB;
PFNGLUNIFORM4IARBPROC ELglUniform4iARB;
PFNGLUNIFORM4IVARBPROC ELglUniform4ivARB;
PFNGLUNIFORMMATRIX2FVARBPROC ELglUniformMatrix2fvARB;
PFNGLUNIFORMMATRIX3FVARBPROC ELglUniformMatrix3fvARB;
PFNGLUNIFORMMATRIX4FVARBPROC ELglUniformMatrix4fvARB;
PFNGLUSEPROGRAMOBJECTARBPROC ELglUseProgramObjectARB;
PFNGLVALIDATEPROGRAMARBPROC ELglValidateProgramARB;
/*	GL_ARB_shader_objects		*/

static GLboolean el_init_GL_ARB_multitexture()
{
	GLboolean r = GL_TRUE;

	r = ((ELglActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB")) != NULL) && r;
	r = ((ELglClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1dARB = (PFNGLMULTITEXCOORD1DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1dvARB = (PFNGLMULTITEXCOORD1DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1fvARB = (PFNGLMULTITEXCOORD1FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1iARB = (PFNGLMULTITEXCOORD1IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1iARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1ivARB = (PFNGLMULTITEXCOORD1IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1ivARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1sARB = (PFNGLMULTITEXCOORD1SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1sARB")) != NULL) && r;
	r = ((ELglMultiTexCoord1svARB = (PFNGLMULTITEXCOORD1SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord1svARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2dARB = (PFNGLMULTITEXCOORD2DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2dvARB = (PFNGLMULTITEXCOORD2DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2iARB = (PFNGLMULTITEXCOORD2IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2iARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2ivARB = (PFNGLMULTITEXCOORD2IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2ivARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2sARB = (PFNGLMULTITEXCOORD2SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2sARB")) != NULL) && r;
	r = ((ELglMultiTexCoord2svARB = (PFNGLMULTITEXCOORD2SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2svARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3dARB = (PFNGLMULTITEXCOORD3DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3dvARB = (PFNGLMULTITEXCOORD3DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3fvARB = (PFNGLMULTITEXCOORD3FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3iARB = (PFNGLMULTITEXCOORD3IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3iARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3ivARB = (PFNGLMULTITEXCOORD3IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3ivARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3sARB = (PFNGLMULTITEXCOORD3SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3sARB")) != NULL) && r;
	r = ((ELglMultiTexCoord3svARB = (PFNGLMULTITEXCOORD3SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3svARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4dARB = (PFNGLMULTITEXCOORD4DARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4dvARB = (PFNGLMULTITEXCOORD4DVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4fvARB = (PFNGLMULTITEXCOORD4FVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fvARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4iARB = (PFNGLMULTITEXCOORD4IARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4iARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4ivARB = (PFNGLMULTITEXCOORD4IVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4ivARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4sARB = (PFNGLMULTITEXCOORD4SARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4sARB")) != NULL) && r;
	r = ((ELglMultiTexCoord4svARB = (PFNGLMULTITEXCOORD4SVARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4svARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_texture_compression()
{
	GLboolean r = GL_TRUE;

	r = ((ELglCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage1DARB")) != NULL) && r;
	r = ((ELglCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage2DARB")) != NULL) && r;
	r = ((ELglCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage3DARB")) != NULL) && r;
	r = ((ELglCompressedTexSubImage1DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage1DARB")) != NULL) && r;
	r = ((ELglCompressedTexSubImage2DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage2DARB")) != NULL) && r;
	r = ((ELglCompressedTexSubImage3DARB = (PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage3DARB")) != NULL) && r;
	r = ((ELglGetCompressedTexImageARB = (PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)SDL_GL_GetProcAddress("glGetCompressedTexImageARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_point_parameters()
{
	GLboolean r = GL_TRUE;

	r = ((ELglPointParameterfARB = (PFNGLPOINTPARAMETERFARBPROC)SDL_GL_GetProcAddress("glPointParameterfARB")) != NULL) && r;
	r = ((ELglPointParameterfvARB = (PFNGLPOINTPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glPointParameterfvARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_vertex_buffer_object()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB")) != NULL) && r;
	r = ((ELglBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB")) != NULL) && r;
	r = ((ELglBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glBufferSubDataARB")) != NULL) && r;
	r = ((ELglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB")) != NULL) && r;
	r = ((ELglGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB")) != NULL) && r;
	r = ((ELglGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetBufferParameterivARB")) != NULL) && r;
	r = ((ELglGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)SDL_GL_GetProcAddress("glGetBufferPointervARB")) != NULL) && r;
	r = ((ELglGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glGetBufferSubDataARB")) != NULL) && r;
	r = ((ELglIsBufferARB = (PFNGLISBUFFERARBPROC)SDL_GL_GetProcAddress("glIsBufferARB")) != NULL) && r;
	r = ((ELglMapBufferARB = (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB")) != NULL) && r;
	r = ((ELglUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_occlusion_query()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBeginQueryARB = (PFNGLBEGINQUERYARBPROC)SDL_GL_GetProcAddress("glBeginQueryARB")) != NULL) && r;
	r = ((ELglDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)SDL_GL_GetProcAddress("glDeleteQueriesARB")) != NULL) && r;
	r = ((ELglEndQueryARB = (PFNGLENDQUERYARBPROC)SDL_GL_GetProcAddress("glEndQueryARB")) != NULL) && r;
	r = ((ELglGenQueriesARB = (PFNGLGENQUERIESARBPROC)SDL_GL_GetProcAddress("glGenQueriesARB")) != NULL) && r;
	r = ((ELglGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)SDL_GL_GetProcAddress("glGetQueryObjectivARB")) != NULL) && r;
	r = ((ELglGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)SDL_GL_GetProcAddress("glGetQueryObjectuivARB")) != NULL) && r;
	r = ((ELglGetQueryivARB = (PFNGLGETQUERYIVARBPROC)SDL_GL_GetProcAddress("glGetQueryivARB")) != NULL) && r;
	r = ((ELglIsQueryARB = (PFNGLISQUERYARBPROC)SDL_GL_GetProcAddress("glIsQueryARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_vertex_program()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBindProgramARB = (PFNGLBINDPROGRAMARBPROC)SDL_GL_GetProcAddress("glBindProgramARB")) != NULL) && r;
	r = ((ELglDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)SDL_GL_GetProcAddress("glDeleteProgramsARB")) != NULL) && r;
	r = ((ELglDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)SDL_GL_GetProcAddress("glDisableVertexAttribArrayARB")) != NULL) && r;
	r = ((ELglEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArrayARB")) != NULL) && r;
	r = ((ELglGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)SDL_GL_GetProcAddress("glGenProgramsARB")) != NULL) && r;
	r = ((ELglGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARBPROC)SDL_GL_GetProcAddress("glGetProgramEnvParameterdvARB")) != NULL) && r;
	r = ((ELglGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetProgramEnvParameterfvARB")) != NULL) && r;
	r = ((ELglGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC)SDL_GL_GetProcAddress("glGetProgramLocalParameterdvARB")) != NULL) && r;
	r = ((ELglGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetProgramLocalParameterfvARB")) != NULL) && r;
	r = ((ELglGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARBPROC)SDL_GL_GetProcAddress("glGetProgramStringARB")) != NULL) && r;
	r = ((ELglGetProgramivARB = (PFNGLGETPROGRAMIVARBPROC)SDL_GL_GetProcAddress("glGetProgramivARB")) != NULL) && r;
	r = ((ELglGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)SDL_GL_GetProcAddress("glGetVertexAttribPointervARB")) != NULL) && r;
	r = ((ELglGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARBPROC)SDL_GL_GetProcAddress("glGetVertexAttribdvARB")) != NULL) && r;
	r = ((ELglGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARBPROC)SDL_GL_GetProcAddress("glGetVertexAttribfvARB")) != NULL) && r;
	r = ((ELglGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARBPROC)SDL_GL_GetProcAddress("glGetVertexAttribivARB")) != NULL) && r;
	r = ((ELglIsProgramARB = (PFNGLISPROGRAMARBPROC)SDL_GL_GetProcAddress("glIsProgramARB")) != NULL) && r;
	r = ((ELglProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARBPROC)SDL_GL_GetProcAddress("glProgramEnvParameter4dARB")) != NULL) && r;
	r = ((ELglProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARBPROC)SDL_GL_GetProcAddress("glProgramEnvParameter4dvARB")) != NULL) && r;
	r = ((ELglProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARBPROC)SDL_GL_GetProcAddress("glProgramEnvParameter4fARB")) != NULL) && r;
	r = ((ELglProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)SDL_GL_GetProcAddress("glProgramEnvParameter4fvARB")) != NULL) && r;
	r = ((ELglProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC)SDL_GL_GetProcAddress("glProgramLocalParameter4dARB")) != NULL) && r;
	r = ((ELglProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARBPROC)SDL_GL_GetProcAddress("glProgramLocalParameter4dvARB")) != NULL) && r;
	r = ((ELglProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)SDL_GL_GetProcAddress("glProgramLocalParameter4fARB")) != NULL) && r;
	r = ((ELglProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)SDL_GL_GetProcAddress("glProgramLocalParameter4fvARB")) != NULL) && r;
	r = ((ELglProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)SDL_GL_GetProcAddress("glProgramStringARB")) != NULL) && r;
	r = ((ELglVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1dARB")) != NULL) && r;
	r = ((ELglVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1dvARB")) != NULL) && r;
	r = ((ELglVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1fARB")) != NULL) && r;
	r = ((ELglVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1fvARB")) != NULL) && r;
	r = ((ELglVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1sARB")) != NULL) && r;
	r = ((ELglVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib1svARB")) != NULL) && r;
	r = ((ELglVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2dARB")) != NULL) && r;
	r = ((ELglVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2dvARB")) != NULL) && r;
	r = ((ELglVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2fARB")) != NULL) && r;
	r = ((ELglVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2fvARB")) != NULL) && r;
	r = ((ELglVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2sARB")) != NULL) && r;
	r = ((ELglVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib2svARB")) != NULL) && r;
	r = ((ELglVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3dARB")) != NULL) && r;
	r = ((ELglVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3dvARB")) != NULL) && r;
	r = ((ELglVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3fARB")) != NULL) && r;
	r = ((ELglVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3fvARB")) != NULL) && r;
	r = ((ELglVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3sARB")) != NULL) && r;
	r = ((ELglVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib3svARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NbvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NivARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NsvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NubARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NubvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NuivARB")) != NULL) && r;
	r = ((ELglVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4NusvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4bvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4dARB")) != NULL) && r;
	r = ((ELglVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4dvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4fARB")) != NULL) && r;
	r = ((ELglVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4fvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4ivARB")) != NULL) && r;
	r = ((ELglVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4sARB")) != NULL) && r;
	r = ((ELglVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4svARB")) != NULL) && r;
	r = ((ELglVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4ubvARB")) != NULL) && r;
	r = ((ELglVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4uivARB")) != NULL) && r;
	r = ((ELglVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARBPROC)SDL_GL_GetProcAddress("glVertexAttrib4usvARB")) != NULL) && r;
	r = ((ELglVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARBPROC)SDL_GL_GetProcAddress("glVertexAttribPointerARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_vertex_shader()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBindAttribLocationARB = (PFNGLBINDATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glBindAttribLocationARB")) != NULL) && r;
	r = ((ELglGetActiveAttribARB = (PFNGLGETACTIVEATTRIBARBPROC)SDL_GL_GetProcAddress("glGetActiveAttribARB")) != NULL) && r;
	r = ((ELglGetAttribLocationARB = (PFNGLGETATTRIBLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetAttribLocationARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_ARB_shader_objects()
{
	GLboolean r = GL_TRUE;

	r = ((ELglAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)SDL_GL_GetProcAddress("glAttachObjectARB")) != NULL) && r;
	r = ((ELglCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)SDL_GL_GetProcAddress("glCompileShaderARB")) != NULL) && r;
	r = ((ELglCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glCreateProgramObjectARB")) != NULL) && r;
	r = ((ELglCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)SDL_GL_GetProcAddress("glCreateShaderObjectARB")) != NULL) && r;
	r = ((ELglDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)SDL_GL_GetProcAddress("glDeleteObjectARB")) != NULL) && r;
	r = ((ELglDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)SDL_GL_GetProcAddress("glDetachObjectARB")) != NULL) && r;
	r = ((ELglGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)SDL_GL_GetProcAddress("glGetActiveUniformARB")) != NULL) && r;
	r = ((ELglGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)SDL_GL_GetProcAddress("glGetAttachedObjectsARB")) != NULL) && r;
	r = ((ELglGetHandleARB = (PFNGLGETHANDLEARBPROC)SDL_GL_GetProcAddress("glGetHandleARB")) != NULL) && r;
	r = ((ELglGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)SDL_GL_GetProcAddress("glGetInfoLogARB")) != NULL) && r;
	r = ((ELglGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterfvARB")) != NULL) && r;
	r = ((ELglGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)SDL_GL_GetProcAddress("glGetObjectParameterivARB")) != NULL) && r;
	r = ((ELglGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glGetShaderSourceARB")) != NULL) && r;
	r = ((ELglGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress("glGetUniformLocationARB")) != NULL) && r;
	r = ((ELglGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)SDL_GL_GetProcAddress("glGetUniformfvARB")) != NULL) && r;
	r = ((ELglGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)SDL_GL_GetProcAddress("glGetUniformivARB")) != NULL) && r;
	r = ((ELglLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)SDL_GL_GetProcAddress("glLinkProgramARB")) != NULL) && r;
	r = ((ELglShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)SDL_GL_GetProcAddress("glShaderSourceARB")) != NULL) && r;
	r = ((ELglUniform1fARB = (PFNGLUNIFORM1FARBPROC)SDL_GL_GetProcAddress("glUniform1fARB")) != NULL) && r;
	r = ((ELglUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)SDL_GL_GetProcAddress("glUniform1fvARB")) != NULL) && r;
	r = ((ELglUniform1iARB = (PFNGLUNIFORM1IARBPROC)SDL_GL_GetProcAddress("glUniform1iARB")) != NULL) && r;
	r = ((ELglUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)SDL_GL_GetProcAddress("glUniform1ivARB")) != NULL) && r;
	r = ((ELglUniform2fARB = (PFNGLUNIFORM2FARBPROC)SDL_GL_GetProcAddress("glUniform2fARB")) != NULL) && r;
	r = ((ELglUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)SDL_GL_GetProcAddress("glUniform2fvARB")) != NULL) && r;
	r = ((ELglUniform2iARB = (PFNGLUNIFORM2IARBPROC)SDL_GL_GetProcAddress("glUniform2iARB")) != NULL) && r;
	r = ((ELglUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)SDL_GL_GetProcAddress("glUniform2ivARB")) != NULL) && r;
	r = ((ELglUniform3fARB = (PFNGLUNIFORM3FARBPROC)SDL_GL_GetProcAddress("glUniform3fARB")) != NULL) && r;
	r = ((ELglUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)SDL_GL_GetProcAddress("glUniform3fvARB")) != NULL) && r;
	r = ((ELglUniform3iARB = (PFNGLUNIFORM3IARBPROC)SDL_GL_GetProcAddress("glUniform3iARB")) != NULL) && r;
	r = ((ELglUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)SDL_GL_GetProcAddress("glUniform3ivARB")) != NULL) && r;
	r = ((ELglUniform4fARB = (PFNGLUNIFORM4FARBPROC)SDL_GL_GetProcAddress("glUniform4fARB")) != NULL) && r;
	r = ((ELglUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)SDL_GL_GetProcAddress("glUniform4fvARB")) != NULL) && r;
	r = ((ELglUniform4iARB = (PFNGLUNIFORM4IARBPROC)SDL_GL_GetProcAddress("glUniform4iARB")) != NULL) && r;
	r = ((ELglUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)SDL_GL_GetProcAddress("glUniform4ivARB")) != NULL) && r;
	r = ((ELglUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix2fvARB")) != NULL) && r;
	r = ((ELglUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix3fvARB")) != NULL) && r;
	r = ((ELglUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)SDL_GL_GetProcAddress("glUniformMatrix4fvARB")) != NULL) && r;
	r = ((ELglUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)SDL_GL_GetProcAddress("glUseProgramObjectARB")) != NULL) && r;
	r = ((ELglValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)SDL_GL_GetProcAddress("glValidateProgramARB")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_EXT_compiled_vertex_array()
{
	GLboolean r = GL_TRUE;

	r = ((ELglLockArraysEXT = (PFNGLLOCKARRAYSEXTPROC)SDL_GL_GetProcAddress("glLockArraysEXT")) != NULL) && r;
	r = ((ELglUnlockArraysEXT = (PFNGLUNLOCKARRAYSEXTPROC)SDL_GL_GetProcAddress("glUnlockArraysEXT")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_EXT_draw_range_elements()
{
	GLboolean r = GL_TRUE;

	r = ((ELglDrawRangeElementsEXT = (PFNGLDRAWRANGEELEMENTSEXTPROC)SDL_GL_GetProcAddress("glDrawRangeElementsEXT")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_EXT_framebuffer_object()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT")) != NULL) && r;
	r = ((ELglBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT")) != NULL) && r;
	r = ((ELglCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT")) != NULL) && r;
	r = ((ELglDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT")) != NULL) && r;
	r = ((ELglDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT")) != NULL) && r;
	r = ((ELglFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT")) != NULL) && r;
	r = ((ELglFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture1DEXT")) != NULL) && r;
	r = ((ELglFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT")) != NULL) && r;
	r = ((ELglFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT")) != NULL) && r;
	r = ((ELglGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT")) != NULL) && r;
	r = ((ELglGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT")) != NULL) && r;
	r = ((ELglGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT")) != NULL) && r;
	r = ((ELglGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT")) != NULL) && r;
	r = ((ELglGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT")) != NULL) && r;
	r = ((ELglIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsFramebufferEXT")) != NULL) && r;
	r = ((ELglIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glIsRenderbufferEXT")) != NULL) && r;
	r = ((ELglRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT")) != NULL) && r;

	return r;
}

void init_opengl_extensions()
{
	GLboolean e;
	char* extensions_string;

	extensions_string = (char*)glGetString(GL_EXTENSIONS);

/*	GL_ARB_multitexture			*/
	if (strstr(extensions_string, "GL_ARB_multitexture") != NULL)
	{
		e = el_init_GL_ARB_multitexture();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_multitexture;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &texture_units);
		}
	}
/*	GL_ARB_multitexture			*/
/*	GL_ARB_texture_compression		*/
	if (strstr(extensions_string, "GL_ARB_texture_compression") != NULL)
	{
		e = el_init_GL_ARB_texture_compression();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_texture_compression;
		}
	}
/*	GL_ARB_texture_compression		*/
/*	GL_ARB_point_parameters			*/
	if (strstr(extensions_string, "GL_ARB_point_parameters") != NULL)
	{
		e = el_init_GL_ARB_point_parameters();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_point_parameters;
		}
	}
/*	GL_ARB_point_parameters			*/
/*	GL_ARB_point_sprite			*/
	if (strstr(extensions_string, "GL_ARB_point_sprite") != NULL)
	{
		extensions |= 1 << arb_point_sprite;
	}
/*	GL_ARB_point_sprite			*/
/*	GL_ARB_vertex_buffer_object		*/
	if (strstr(extensions_string, "GL_ARB_vertex_buffer_object") != NULL)
	{
		e = el_init_GL_ARB_vertex_buffer_object();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_vertex_buffer_object;
		}
	}
/*	GL_ARB_vertex_buffer_object		*/
/*	GL_ARB_shadow				*/
	if (strstr(extensions_string, "GL_ARB_shadow") != NULL)
	{
		extensions |= 1 << arb_shadow;
	}
/*	GL_ARB_shadow				*/
/*	GL_ARB_texture_env_combine		*/
	if (strstr(extensions_string, "GL_ARB_texture_env_combine") != NULL)
	{
		extensions |= 1 << arb_texture_env_combine;
	}
/*	GL_ARB_texture_env_combine		*/
/*	GL_ARB_texture_env_crossbar		*/
	if (strstr(extensions_string, "GL_ARB_texture_env_crossbar") != NULL)
	{
		extensions |= 1 << arb_texture_env_crossbar;
	}
/*	GL_ARB_texture_env_crossbar		*/
/*	GL_ARB_texture_env_dot3			*/
	if (strstr(extensions_string, "GL_ARB_texture_env_dot3") != NULL)
	{
		extensions |= 1 << arb_texture_env_dot3;
	}
/*	GL_ARB_texture_env_dot3			*/
/*	GL_ARB_occlusion_query			*/
	if (strstr(extensions_string, "GL_ARB_occlusion_query") != NULL)
	{
		e = el_init_GL_ARB_occlusion_query();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_occlusion_query;
		}
	}
/*	GL_ARB_occlusion_query			*/
/*	GL_ARB_depth_texture			*/
	if (strstr(extensions_string, "GL_ARB_depth_texture") != NULL)
	{
		extensions |= 1 << arb_depth_texture;
	}
/*	GL_ARB_depth_texture			*/
/*	GL_ARB_fragment_program			*/
	if (strstr(extensions_string, "GL_ARB_fragment_program") != NULL)
	{
		extensions |= 1 << arb_fragment_program;
	}
/*	GL_ARB_fragment_program			*/
/*	GL_ARB_vertex_program			*/
	if (strstr(extensions_string, "GL_ARB_vertex_program") != NULL)
	{
		e = el_init_GL_ARB_vertex_program();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_vertex_program;
		}
	}
/*	GL_ARB_vertex_program			*/
/*	GL_ARB_fragment_shader			*/
	if (strstr(extensions_string, "GL_ARB_fragment_shader") != NULL)
	{
		extensions |= 1 << arb_fragment_shader;
	}
/*	GL_ARB_fragment_shader			*/
/*	GL_ARB_vertex_shader			*/
	if (strstr(extensions_string, "GL_ARB_vertex_shader") != NULL)
	{
		e = el_init_GL_ARB_vertex_shader();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_vertex_shader;
		}
	}
/*	GL_ARB_vertex_shader			*/
/*	GL_ARB_shader_objects			*/
	if (strstr(extensions_string, "GL_ARB_shader_objects") != NULL)
	{
		e = el_init_GL_ARB_shader_objects();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_shader_objects;
		}
	}
/*	GL_ARB_shader_objects			*/
/*	GL_ARB_shading_language_100		*/
	if (strstr(extensions_string, "GL_ARB_shading_language_100") != NULL)
	{
		extensions |= 1 << arb_shading_language_100;
	}
/*	GL_ARB_shading_language_100		*/
	if (strstr(extensions_string, "GL_ARB_texture_non_power_of_two") != NULL)
	{
		extensions |= 1 << arb_texture_non_power_of_two;
	}
/*	GL_EXT_compiled_vertex_array		*/
	if (strstr(extensions_string, "GL_EXT_compiled_vertex_array") != NULL)
	{
		e = el_init_GL_EXT_compiled_vertex_array();
		if (e == GL_TRUE)
		{
			extensions |= 1 << ext_compiled_vertex_array;
		}
	}
/*	GL_EXT_compiled_vertex_array		*/
/*	GL_EXT_draw_range_elements		*/
	if (strstr(extensions_string, "GL_EXT_draw_range_elements") != NULL)
	{
		e = el_init_GL_EXT_draw_range_elements();
		if (e == GL_TRUE)
		{
			extensions |= 1 << ext_draw_range_elements;
		}
	}
/*	GL_EXT_draw_range_elements		*/
/*	GL_EXT_framebuffer_object		*/
	if (strstr(extensions_string, "GL_EXT_framebuffer_object") != NULL)
	{
		e = el_init_GL_EXT_framebuffer_object();
		if (e == GL_TRUE)
		{
			extensions |= 1 << ext_framebuffer_object;
		}
	}
/*	GL_EXT_framebuffer_object		*/
/*	GL_EXT_texture_compression_s3tc		*/
	if (strstr(extensions_string, "GL_EXT_texture_compression_s3tc") != NULL)
	{
		extensions |= 1 << ext_texture_compression_s3tc;
	}
/*	GL_EXT_texture_compression_s3tc		*/
/*	GL_EXT_texture_filter_anisotropic	*/
	if (strstr(extensions_string, "GL_EXT_texture_filter_anisotropic") != NULL)
	{
		extensions |= 1 << ext_texture_filter_anisotropic;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropic_filter);
	}
/*	GL_EXT_texture_filter_anisotropic	*/
/*	GL_SGIS_generate_mipmap			*/
	if (strstr(extensions_string, "GL_SGIS_generate_mipmap") != NULL)
	{
		extensions |= 1 << sgis_generate_mipmap;
	}
/*	GL_SGIS_generate_mipmap			*/
}

uint_fast32_t have_extension(extension_enum extension)
{
	return (extensions & (1 << extension)) != 0;
}

uint_fast32_t get_texture_units()
{
	return texture_units;
}

float get_max_anisotropic_filter()
{
	return max_anisotropic_filter;
}

