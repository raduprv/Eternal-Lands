#ifdef WINDOWS
#include <windows.h>
#endif // WINDOWS
#include <SDL.h>
#include "load_gl_extensions.h"
#include "client_serv.h"
#include "text.h"
#include "errors.h"

const char* gl_versions_str[] = { "1.1", "1.2", "1.3", "1.4", "1.5", "2.0", "2.1" };
const Uint16 gl_versions[] = { 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0200, 0x0201 };

Uint32 gl_version = 0;
Uint64 extensions = 0;
GLint texture_units = 1;

/*	GL_VERSION_1_2		*/
PFNGLCOPYTEXSUBIMAGE3DPROC ELglCopyTexSubImage3D = NULL;
PFNGLDRAWRANGEELEMENTSPROC ELglDrawRangeElements = NULL;
PFNGLTEXIMAGE3DPROC ELglTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC ELglTexSubImage3D = NULL;
/*	GL_VERSION_1_2		*/

/*	GL_VERSION_1_3		*/
PFNGLACTIVETEXTUREPROC ELglActiveTexture = NULL;
PFNGLCLIENTACTIVETEXTUREPROC ELglClientActiveTexture = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DPROC ELglCompressedTexImage1D = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC ELglCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DPROC ELglCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC ELglCompressedTexSubImage1D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC ELglCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC ELglCompressedTexSubImage3D = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEPROC ELglGetCompressedTexImage = NULL;
PFNGLLOADTRANSPOSEMATRIXDPROC ELglLoadTransposeMatrixd = NULL;
PFNGLLOADTRANSPOSEMATRIXFPROC ELglLoadTransposeMatrixf = NULL;
PFNGLMULTTRANSPOSEMATRIXDPROC ELglMultTransposeMatrixd = NULL;
PFNGLMULTTRANSPOSEMATRIXFPROC ELglMultTransposeMatrixf = NULL;
PFNGLMULTITEXCOORD1DPROC ELglMultiTexCoord1d = NULL;
PFNGLMULTITEXCOORD1DVPROC ELglMultiTexCoord1dv = NULL;
PFNGLMULTITEXCOORD1FPROC ELglMultiTexCoord1f = NULL;
PFNGLMULTITEXCOORD1FVPROC ELglMultiTexCoord1fv = NULL;
PFNGLMULTITEXCOORD1IPROC ELglMultiTexCoord1i = NULL;
PFNGLMULTITEXCOORD1IVPROC ELglMultiTexCoord1iv = NULL;
PFNGLMULTITEXCOORD1SPROC ELglMultiTexCoord1s = NULL;
PFNGLMULTITEXCOORD1SVPROC ELglMultiTexCoord1sv = NULL;
PFNGLMULTITEXCOORD2DPROC ELglMultiTexCoord2d = NULL;
PFNGLMULTITEXCOORD2DVPROC ELglMultiTexCoord2dv = NULL;
PFNGLMULTITEXCOORD2FPROC ELglMultiTexCoord2f = NULL;
PFNGLMULTITEXCOORD2FVPROC ELglMultiTexCoord2fv = NULL;
PFNGLMULTITEXCOORD2IPROC ELglMultiTexCoord2i = NULL;
PFNGLMULTITEXCOORD2IVPROC ELglMultiTexCoord2iv = NULL;
PFNGLMULTITEXCOORD2SPROC ELglMultiTexCoord2s = NULL;
PFNGLMULTITEXCOORD2SVPROC ELglMultiTexCoord2sv = NULL;
PFNGLMULTITEXCOORD3DPROC ELglMultiTexCoord3d = NULL;
PFNGLMULTITEXCOORD3DVPROC ELglMultiTexCoord3dv = NULL;
PFNGLMULTITEXCOORD3FPROC ELglMultiTexCoord3f = NULL;
PFNGLMULTITEXCOORD3FVPROC ELglMultiTexCoord3fv = NULL;
PFNGLMULTITEXCOORD3IPROC ELglMultiTexCoord3i = NULL;
PFNGLMULTITEXCOORD3IVPROC ELglMultiTexCoord3iv = NULL;
PFNGLMULTITEXCOORD3SPROC ELglMultiTexCoord3s = NULL;
PFNGLMULTITEXCOORD3SVPROC ELglMultiTexCoord3sv = NULL;
PFNGLMULTITEXCOORD4DPROC ELglMultiTexCoord4d = NULL;
PFNGLMULTITEXCOORD4DVPROC ELglMultiTexCoord4dv = NULL;
PFNGLMULTITEXCOORD4FPROC ELglMultiTexCoord4f = NULL;
PFNGLMULTITEXCOORD4FVPROC ELglMultiTexCoord4fv = NULL;
PFNGLMULTITEXCOORD4IPROC ELglMultiTexCoord4i = NULL;
PFNGLMULTITEXCOORD4IVPROC ELglMultiTexCoord4iv = NULL;
PFNGLMULTITEXCOORD4SPROC ELglMultiTexCoord4s = NULL;
PFNGLMULTITEXCOORD4SVPROC ELglMultiTexCoord4sv = NULL;
PFNGLSAMPLECOVERAGEPROC ELglSampleCoverage = NULL;
/*	GL_VERSION_1_3		*/

/*	GL_VERSION_1_4		*/
PFNGLBLENDCOLORPROC ELglBlendColor = NULL;
PFNGLBLENDEQUATIONPROC ELglBlendEquation = NULL;
PFNGLBLENDFUNCSEPARATEPROC ELglBlendFuncSeparate = NULL;
PFNGLFOGCOORDPOINTERPROC ELglFogCoordPointer = NULL;
PFNGLFOGCOORDDPROC ELglFogCoordd = NULL;
PFNGLFOGCOORDDVPROC ELglFogCoorddv = NULL;
PFNGLFOGCOORDFPROC ELglFogCoordf = NULL;
PFNGLFOGCOORDFVPROC ELglFogCoordfv = NULL;
PFNGLMULTIDRAWARRAYSPROC ELglMultiDrawArrays = NULL;
PFNGLMULTIDRAWELEMENTSPROC ELglMultiDrawElements = NULL;
PFNGLPOINTPARAMETERFPROC ELglPointParameterf = NULL;
PFNGLPOINTPARAMETERFVPROC ELglPointParameterfv = NULL;
PFNGLSECONDARYCOLOR3BPROC ELglSecondaryColor3b = NULL;
PFNGLSECONDARYCOLOR3BVPROC ELglSecondaryColor3bv = NULL;
PFNGLSECONDARYCOLOR3DPROC ELglSecondaryColor3d = NULL;
PFNGLSECONDARYCOLOR3DVPROC ELglSecondaryColor3dv = NULL;
PFNGLSECONDARYCOLOR3FPROC ELglSecondaryColor3f = NULL;
PFNGLSECONDARYCOLOR3FVPROC ELglSecondaryColor3fv = NULL;
PFNGLSECONDARYCOLOR3IPROC ELglSecondaryColor3i = NULL;
PFNGLSECONDARYCOLOR3IVPROC ELglSecondaryColor3iv = NULL;
PFNGLSECONDARYCOLOR3SPROC ELglSecondaryColor3s = NULL;
PFNGLSECONDARYCOLOR3SVPROC ELglSecondaryColor3sv = NULL;
PFNGLSECONDARYCOLOR3UBPROC ELglSecondaryColor3ub = NULL;
PFNGLSECONDARYCOLOR3UBVPROC ELglSecondaryColor3ubv = NULL;
PFNGLSECONDARYCOLOR3UIPROC ELglSecondaryColor3ui = NULL;
PFNGLSECONDARYCOLOR3UIVPROC ELglSecondaryColor3uiv = NULL;
PFNGLSECONDARYCOLOR3USPROC ELglSecondaryColor3us = NULL;
PFNGLSECONDARYCOLOR3USVPROC ELglSecondaryColor3usv = NULL;
PFNGLSECONDARYCOLORPOINTERPROC ELglSecondaryColorPointer = NULL;
PFNGLWINDOWPOS2DPROC ELglWindowPos2d = NULL;
PFNGLWINDOWPOS2DVPROC ELglWindowPos2dv = NULL;
PFNGLWINDOWPOS2FPROC ELglWindowPos2f = NULL;
PFNGLWINDOWPOS2FVPROC ELglWindowPos2fv = NULL;
PFNGLWINDOWPOS2IPROC ELglWindowPos2i = NULL;
PFNGLWINDOWPOS2IVPROC ELglWindowPos2iv = NULL;
PFNGLWINDOWPOS2SPROC ELglWindowPos2s = NULL;
PFNGLWINDOWPOS2SVPROC ELglWindowPos2sv = NULL;
PFNGLWINDOWPOS3DPROC ELglWindowPos3d = NULL;
PFNGLWINDOWPOS3DVPROC ELglWindowPos3dv = NULL;
PFNGLWINDOWPOS3FPROC ELglWindowPos3f = NULL;
PFNGLWINDOWPOS3FVPROC ELglWindowPos3fv = NULL;
PFNGLWINDOWPOS3IPROC ELglWindowPos3i = NULL;
PFNGLWINDOWPOS3IVPROC ELglWindowPos3iv = NULL;
PFNGLWINDOWPOS3SPROC ELglWindowPos3s = NULL;
PFNGLWINDOWPOS3SVPROC ELglWindowPos3sv = NULL;
/*	GL_VERSION_1_4		*/

/*	GL_VERSION_1_5		*/
PFNGLBEGINQUERYPROC ELglBeginQuery = NULL;
PFNGLBINDBUFFERPROC ELglBindBuffer = NULL;
PFNGLBUFFERDATAPROC ELglBufferData = NULL;
PFNGLBUFFERSUBDATAPROC ELglBufferSubData = NULL;
PFNGLDELETEBUFFERSPROC ELglDeleteBuffers = NULL;
PFNGLDELETEQUERIESPROC ELglDeleteQueries = NULL;
PFNGLENDQUERYPROC ELglEndQuery = NULL;
PFNGLGENBUFFERSPROC ELglGenBuffers = NULL;
PFNGLGENQUERIESPROC ELglGenQueries = NULL;
PFNGLGETBUFFERPARAMETERIVPROC ELglGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC ELglGetBufferPointerv = NULL;
PFNGLGETBUFFERSUBDATAPROC ELglGetBufferSubData = NULL;
PFNGLGETQUERYOBJECTIVPROC ELglGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUIVPROC ELglGetQueryObjectuiv = NULL;
PFNGLGETQUERYIVPROC ELglGetQueryiv = NULL;
PFNGLISBUFFERPROC ELglIsBuffer = NULL;
PFNGLISQUERYPROC ELglIsQuery = NULL;
PFNGLMAPBUFFERPROC ELglMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC ELglUnmapBuffer = NULL;
/*	GL_VERSION_1_5		*/

/*	GL_VERSION_2_0		*/
PFNGLATTACHSHADERPROC ELglAttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC ELglBindAttribLocation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC ELglBlendEquationSeparate = NULL;
PFNGLCOMPILESHADERPROC ELglCompileShader = NULL;
PFNGLCREATEPROGRAMPROC ELglCreateProgram = NULL;
PFNGLCREATESHADERPROC ELglCreateShader = NULL;
PFNGLDELETEPROGRAMPROC ELglDeleteProgram = NULL;
PFNGLDELETESHADERPROC ELglDeleteShader = NULL;
PFNGLDETACHSHADERPROC ELglDetachShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC ELglDisableVertexAttribArray = NULL;
PFNGLDRAWBUFFERSPROC ELglDrawBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC ELglEnableVertexAttribArray = NULL;
PFNGLGETACTIVEATTRIBPROC ELglGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC ELglGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC ELglGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC ELglGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC ELglGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC ELglGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC ELglGetShaderInfoLog = NULL;
PFNGLGETSHADERSOURCEPROC ELglGetShaderSource = NULL;
PFNGLGETSHADERIVPROC ELglGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC ELglGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC ELglGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC ELglGetUniformiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC ELglGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBDVPROC ELglGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFVPROC ELglGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC ELglGetVertexAttribiv = NULL;
PFNGLISPROGRAMPROC ELglIsProgram = NULL;
PFNGLISSHADERPROC ELglIsShader = NULL;
PFNGLLINKPROGRAMPROC ELglLinkProgram = NULL;
PFNGLSHADERSOURCEPROC ELglShaderSource = NULL;
PFNGLSTENCILFUNCSEPARATEPROC ELglStencilFuncSeparate = NULL;
PFNGLSTENCILMASKSEPARATEPROC ELglStencilMaskSeparate = NULL;
PFNGLSTENCILOPSEPARATEPROC ELglStencilOpSeparate = NULL;
PFNGLUNIFORM1FPROC ELglUniform1f = NULL;
PFNGLUNIFORM1FVPROC ELglUniform1fv = NULL;
PFNGLUNIFORM1IPROC ELglUniform1i = NULL;
PFNGLUNIFORM1IVPROC ELglUniform1iv = NULL;
PFNGLUNIFORM2FPROC ELglUniform2f = NULL;
PFNGLUNIFORM2FVPROC ELglUniform2fv = NULL;
PFNGLUNIFORM2IPROC ELglUniform2i = NULL;
PFNGLUNIFORM2IVPROC ELglUniform2iv = NULL;
PFNGLUNIFORM3FPROC ELglUniform3f = NULL;
PFNGLUNIFORM3FVPROC ELglUniform3fv = NULL;
PFNGLUNIFORM3IPROC ELglUniform3i = NULL;
PFNGLUNIFORM3IVPROC ELglUniform3iv = NULL;
PFNGLUNIFORM4FPROC ELglUniform4f = NULL;
PFNGLUNIFORM4FVPROC ELglUniform4fv = NULL;
PFNGLUNIFORM4IPROC ELglUniform4i = NULL;
PFNGLUNIFORM4IVPROC ELglUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC ELglUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC ELglUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC ELglUniformMatrix4fv = NULL;
PFNGLUSEPROGRAMPROC ELglUseProgram = NULL;
PFNGLVALIDATEPROGRAMPROC ELglValidateProgram = NULL;
PFNGLVERTEXATTRIB1DPROC ELglVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DVPROC ELglVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1FPROC ELglVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC ELglVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1SPROC ELglVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SVPROC ELglVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2DPROC ELglVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DVPROC ELglVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2FPROC ELglVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC ELglVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2SPROC ELglVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SVPROC ELglVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3DPROC ELglVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DVPROC ELglVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3FPROC ELglVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC ELglVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3SPROC ELglVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SVPROC ELglVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBVPROC ELglVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIVPROC ELglVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSVPROC ELglVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUBPROC ELglVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBVPROC ELglVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIVPROC ELglVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSVPROC ELglVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BVPROC ELglVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4DPROC ELglVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DVPROC ELglVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4FPROC ELglVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC ELglVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IVPROC ELglVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4SPROC ELglVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SVPROC ELglVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBVPROC ELglVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIVPROC ELglVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USVPROC ELglVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC ELglVertexAttribPointer = NULL;
/*	GL_VERSION_2_0		*/

/*	GL_VERSION_2_1		*/
PFNGLUNIFORMMATRIX2X3FVPROC ELglUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC ELglUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC ELglUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC ELglUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC ELglUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC ELglUniformMatrix4x3fv = NULL;
/*	GL_VERSION_2_1		*/

GLboolean is_GL_VERSION_1_2 = GL_FALSE;
GLboolean is_GL_VERSION_1_3 = GL_FALSE;
GLboolean is_GL_VERSION_1_4 = GL_FALSE;
GLboolean is_GL_VERSION_1_5 = GL_FALSE;
GLboolean is_GL_VERSION_2_0 = GL_FALSE;
GLboolean is_GL_VERSION_2_1 = GL_FALSE;

int vertex_program_problem=0;
int multitexture_problem=0;

static void check_for_problem_drivers()
{
	const char *my_string;
	int is_intel=0;


	my_string = (const char*) glGetString (GL_VENDOR);
	if(strstr(my_string,"Intel"))is_intel=1;
	else
	if(strstr(my_string,"SiS"))
		{
			multitexture_problem=1;
		}
	else
	if(strstr(my_string,"S3 "))
		{
			multitexture_problem=1;
		}

	my_string = (const char*) glGetString (GL_VERSION);
/*
	//should be fixed now
	//OpenGL Version Format: 2.0.0
	if(is_nvidia)
		{
			if(my_string[0]=='1')vertex_program_problem=1;
			if(my_string[0]=='2' && my_string[2]=='0')vertex_program_problem=1;
		}
*/
	if(is_intel)
		{
			my_string = (const char*) glGetString (GL_RENDERER);
			if(strstr(my_string,"965") || strstr(my_string,"945"))vertex_program_problem=1;
		}
#ifndef MAP_EDITOR
	//log the problems
	if(vertex_program_problem)
	LOG_TO_CONSOLE (c_red2, "Your card reports having vertex program capabilities, but the support is buggy, so we disabled it.");

	if(multitexture_problem)
	LOG_TO_CONSOLE (c_red2, "Your card reports having multitexturing capabilities, but the support is buggy, so we disabled it.");
#endif //!MAP_EDITOR

}


/*	GL_VERSION_1_2		*/
static GLboolean el_init_GL_VERSION_1_2()
{
	GLboolean r = GL_TRUE;

	r = ((ELglCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)SDL_GL_GetProcAddress("glCopyTexSubImage3D")) != NULL) && r;
	r = ((ELglDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)SDL_GL_GetProcAddress("glDrawRangeElements")) != NULL) && r;
	r = ((ELglTexImage3D = (PFNGLTEXIMAGE3DPROC)SDL_GL_GetProcAddress("glTexImage3D")) != NULL) && r;
	r = ((ELglTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)SDL_GL_GetProcAddress("glTexSubImage3D")) != NULL) && r;

	return r;
}
/*	GL_VERSION_1_2		*/

/*	GL_VERSION_1_3		*/
static GLboolean el_init_GL_VERSION_1_3()
{
	GLboolean r = GL_TRUE;

	r = ((ELglActiveTexture = (PFNGLACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glActiveTexture")) != NULL) && r;
	r = ((ELglClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)SDL_GL_GetProcAddress("glClientActiveTexture")) != NULL) && r;
	r = ((ELglCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)SDL_GL_GetProcAddress("glCompressedTexImage1D")) != NULL) && r;
	r = ((ELglCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)SDL_GL_GetProcAddress("glCompressedTexImage2D")) != NULL) && r;
	r = ((ELglCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)SDL_GL_GetProcAddress("glCompressedTexImage3D")) != NULL) && r;
	r = ((ELglCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage1D")) != NULL) && r;
	r = ((ELglCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage2D")) != NULL) && r;
	r = ((ELglCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)SDL_GL_GetProcAddress("glCompressedTexSubImage3D")) != NULL) && r;
	r = ((ELglGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)SDL_GL_GetProcAddress("glGetCompressedTexImage")) != NULL) && r;
	r = ((ELglLoadTransposeMatrixd = (PFNGLLOADTRANSPOSEMATRIXDPROC)SDL_GL_GetProcAddress("glLoadTransposeMatrixd")) != NULL) && r;
	r = ((ELglLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC)SDL_GL_GetProcAddress("glLoadTransposeMatrixf")) != NULL) && r;
	r = ((ELglMultTransposeMatrixd = (PFNGLMULTTRANSPOSEMATRIXDPROC)SDL_GL_GetProcAddress("glMultTransposeMatrixd")) != NULL) && r;
	r = ((ELglMultTransposeMatrixf = (PFNGLMULTTRANSPOSEMATRIXFPROC)SDL_GL_GetProcAddress("glMultTransposeMatrixf")) != NULL) && r;
	r = ((ELglMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC)SDL_GL_GetProcAddress("glMultiTexCoord1d")) != NULL) && r;
	r = ((ELglMultiTexCoord1dv = (PFNGLMULTITEXCOORD1DVPROC)SDL_GL_GetProcAddress("glMultiTexCoord1dv")) != NULL) && r;
	r = ((ELglMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)SDL_GL_GetProcAddress("glMultiTexCoord1f")) != NULL) && r;
	r = ((ELglMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVPROC)SDL_GL_GetProcAddress("glMultiTexCoord1fv")) != NULL) && r;
	r = ((ELglMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC)SDL_GL_GetProcAddress("glMultiTexCoord1i")) != NULL) && r;
	r = ((ELglMultiTexCoord1iv = (PFNGLMULTITEXCOORD1IVPROC)SDL_GL_GetProcAddress("glMultiTexCoord1iv")) != NULL) && r;
	r = ((ELglMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC)SDL_GL_GetProcAddress("glMultiTexCoord1s")) != NULL) && r;
	r = ((ELglMultiTexCoord1sv = (PFNGLMULTITEXCOORD1SVPROC)SDL_GL_GetProcAddress("glMultiTexCoord1sv")) != NULL) && r;
	r = ((ELglMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC)SDL_GL_GetProcAddress("glMultiTexCoord2d")) != NULL) && r;
	r = ((ELglMultiTexCoord2dv = (PFNGLMULTITEXCOORD2DVPROC)SDL_GL_GetProcAddress("glMultiTexCoord2dv")) != NULL) && r;
	r = ((ELglMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)SDL_GL_GetProcAddress("glMultiTexCoord2f")) != NULL) && r;
	r = ((ELglMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fv")) != NULL) && r;
	r = ((ELglMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)SDL_GL_GetProcAddress("glMultiTexCoord2i")) != NULL) && r;
	r = ((ELglMultiTexCoord2iv = (PFNGLMULTITEXCOORD2IVPROC)SDL_GL_GetProcAddress("glMultiTexCoord2iv")) != NULL) && r;
	r = ((ELglMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC)SDL_GL_GetProcAddress("glMultiTexCoord2s")) != NULL) && r;
	r = ((ELglMultiTexCoord2sv = (PFNGLMULTITEXCOORD2SVPROC)SDL_GL_GetProcAddress("glMultiTexCoord2sv")) != NULL) && r;
	r = ((ELglMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC)SDL_GL_GetProcAddress("glMultiTexCoord3d")) != NULL) && r;
	r = ((ELglMultiTexCoord3dv = (PFNGLMULTITEXCOORD3DVPROC)SDL_GL_GetProcAddress("glMultiTexCoord3dv")) != NULL) && r;
	r = ((ELglMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)SDL_GL_GetProcAddress("glMultiTexCoord3f")) != NULL) && r;
	r = ((ELglMultiTexCoord3fv = (PFNGLMULTITEXCOORD3FVPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fv")) != NULL) && r;
	r = ((ELglMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC)SDL_GL_GetProcAddress("glMultiTexCoord3i")) != NULL) && r;
	r = ((ELglMultiTexCoord3iv = (PFNGLMULTITEXCOORD3IVPROC)SDL_GL_GetProcAddress("glMultiTexCoord3iv")) != NULL) && r;
	r = ((ELglMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC)SDL_GL_GetProcAddress("glMultiTexCoord3s")) != NULL) && r;
	r = ((ELglMultiTexCoord3sv = (PFNGLMULTITEXCOORD3SVPROC)SDL_GL_GetProcAddress("glMultiTexCoord3sv")) != NULL) && r;
	r = ((ELglMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC)SDL_GL_GetProcAddress("glMultiTexCoord4d")) != NULL) && r;
	r = ((ELglMultiTexCoord4dv = (PFNGLMULTITEXCOORD4DVPROC)SDL_GL_GetProcAddress("glMultiTexCoord4dv")) != NULL) && r;
	r = ((ELglMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)SDL_GL_GetProcAddress("glMultiTexCoord4f")) != NULL) && r;
	r = ((ELglMultiTexCoord4fv = (PFNGLMULTITEXCOORD4FVPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fv")) != NULL) && r;
	r = ((ELglMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC)SDL_GL_GetProcAddress("glMultiTexCoord4i")) != NULL) && r;
	r = ((ELglMultiTexCoord4iv = (PFNGLMULTITEXCOORD4IVPROC)SDL_GL_GetProcAddress("glMultiTexCoord4iv")) != NULL) && r;
	r = ((ELglMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC)SDL_GL_GetProcAddress("glMultiTexCoord4s")) != NULL) && r;
	r = ((ELglMultiTexCoord4sv = (PFNGLMULTITEXCOORD4SVPROC)SDL_GL_GetProcAddress("glMultiTexCoord4sv")) != NULL) && r;
	r = ((ELglSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)SDL_GL_GetProcAddress("glSampleCoverage")) != NULL) && r;

	return r;
}
/*	GL_VERSION_1_3		*/

/*	GL_VERSION_1_4		*/
static GLboolean el_init_GL_VERSION_1_4()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBlendColor = (PFNGLBLENDCOLORPROC)SDL_GL_GetProcAddress("glBlendColor")) != NULL) && r;
	r = ((ELglBlendEquation = (PFNGLBLENDEQUATIONPROC)SDL_GL_GetProcAddress("glBlendEquation")) != NULL) && r;
	r = ((ELglBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)SDL_GL_GetProcAddress("glBlendFuncSeparate")) != NULL) && r;
	r = ((ELglFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC)SDL_GL_GetProcAddress("glFogCoordPointer")) != NULL) && r;
	r = ((ELglFogCoordd = (PFNGLFOGCOORDDPROC)SDL_GL_GetProcAddress("glFogCoordd")) != NULL) && r;
	r = ((ELglFogCoorddv = (PFNGLFOGCOORDDVPROC)SDL_GL_GetProcAddress("glFogCoorddv")) != NULL) && r;
	r = ((ELglFogCoordf = (PFNGLFOGCOORDFPROC)SDL_GL_GetProcAddress("glFogCoordf")) != NULL) && r;
	r = ((ELglFogCoordfv = (PFNGLFOGCOORDFVPROC)SDL_GL_GetProcAddress("glFogCoordfv")) != NULL) && r;
	r = ((ELglMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)SDL_GL_GetProcAddress("glMultiDrawArrays")) != NULL) && r;
	r = ((ELglMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)SDL_GL_GetProcAddress("glMultiDrawElements")) != NULL) && r;
	r = ((ELglPointParameterf = (PFNGLPOINTPARAMETERFPROC)SDL_GL_GetProcAddress("glPointParameterf")) != NULL) && r;
	r = ((ELglPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)SDL_GL_GetProcAddress("glPointParameterfv")) != NULL) && r;
	r = ((ELglSecondaryColor3b = (PFNGLSECONDARYCOLOR3BPROC)SDL_GL_GetProcAddress("glSecondaryColor3b")) != NULL) && r;
	r = ((ELglSecondaryColor3bv = (PFNGLSECONDARYCOLOR3BVPROC)SDL_GL_GetProcAddress("glSecondaryColor3bv")) != NULL) && r;
	r = ((ELglSecondaryColor3d = (PFNGLSECONDARYCOLOR3DPROC)SDL_GL_GetProcAddress("glSecondaryColor3d")) != NULL) && r;
	r = ((ELglSecondaryColor3dv = (PFNGLSECONDARYCOLOR3DVPROC)SDL_GL_GetProcAddress("glSecondaryColor3dv")) != NULL) && r;
	r = ((ELglSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)SDL_GL_GetProcAddress("glSecondaryColor3f")) != NULL) && r;
	r = ((ELglSecondaryColor3fv = (PFNGLSECONDARYCOLOR3FVPROC)SDL_GL_GetProcAddress("glSecondaryColor3fv")) != NULL) && r;
	r = ((ELglSecondaryColor3i = (PFNGLSECONDARYCOLOR3IPROC)SDL_GL_GetProcAddress("glSecondaryColor3i")) != NULL) && r;
	r = ((ELglSecondaryColor3iv = (PFNGLSECONDARYCOLOR3IVPROC)SDL_GL_GetProcAddress("glSecondaryColor3iv")) != NULL) && r;
	r = ((ELglSecondaryColor3s = (PFNGLSECONDARYCOLOR3SPROC)SDL_GL_GetProcAddress("glSecondaryColor3s")) != NULL) && r;
	r = ((ELglSecondaryColor3sv = (PFNGLSECONDARYCOLOR3SVPROC)SDL_GL_GetProcAddress("glSecondaryColor3sv")) != NULL) && r;
	r = ((ELglSecondaryColor3ub = (PFNGLSECONDARYCOLOR3UBPROC)SDL_GL_GetProcAddress("glSecondaryColor3ub")) != NULL) && r;
	r = ((ELglSecondaryColor3ubv = (PFNGLSECONDARYCOLOR3UBVPROC)SDL_GL_GetProcAddress("glSecondaryColor3ubv")) != NULL) && r;
	r = ((ELglSecondaryColor3ui = (PFNGLSECONDARYCOLOR3UIPROC)SDL_GL_GetProcAddress("glSecondaryColor3ui")) != NULL) && r;
	r = ((ELglSecondaryColor3uiv = (PFNGLSECONDARYCOLOR3UIVPROC)SDL_GL_GetProcAddress("glSecondaryColor3uiv")) != NULL) && r;
	r = ((ELglSecondaryColor3us = (PFNGLSECONDARYCOLOR3USPROC)SDL_GL_GetProcAddress("glSecondaryColor3us")) != NULL) && r;
	r = ((ELglSecondaryColor3usv = (PFNGLSECONDARYCOLOR3USVPROC)SDL_GL_GetProcAddress("glSecondaryColor3usv")) != NULL) && r;
	r = ((ELglSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTERPROC)SDL_GL_GetProcAddress("glSecondaryColorPointer")) != NULL) && r;
	r = ((ELglWindowPos2d = (PFNGLWINDOWPOS2DPROC)SDL_GL_GetProcAddress("glWindowPos2d")) != NULL) && r;
	r = ((ELglWindowPos2dv = (PFNGLWINDOWPOS2DVPROC)SDL_GL_GetProcAddress("glWindowPos2dv")) != NULL) && r;
	r = ((ELglWindowPos2f = (PFNGLWINDOWPOS2FPROC)SDL_GL_GetProcAddress("glWindowPos2f")) != NULL) && r;
	r = ((ELglWindowPos2fv = (PFNGLWINDOWPOS2FVPROC)SDL_GL_GetProcAddress("glWindowPos2fv")) != NULL) && r;
	r = ((ELglWindowPos2i = (PFNGLWINDOWPOS2IPROC)SDL_GL_GetProcAddress("glWindowPos2i")) != NULL) && r;
	r = ((ELglWindowPos2iv = (PFNGLWINDOWPOS2IVPROC)SDL_GL_GetProcAddress("glWindowPos2iv")) != NULL) && r;
	r = ((ELglWindowPos2s = (PFNGLWINDOWPOS2SPROC)SDL_GL_GetProcAddress("glWindowPos2s")) != NULL) && r;
	r = ((ELglWindowPos2sv = (PFNGLWINDOWPOS2SVPROC)SDL_GL_GetProcAddress("glWindowPos2sv")) != NULL) && r;
	r = ((ELglWindowPos3d = (PFNGLWINDOWPOS3DPROC)SDL_GL_GetProcAddress("glWindowPos3d")) != NULL) && r;
	r = ((ELglWindowPos3dv = (PFNGLWINDOWPOS3DVPROC)SDL_GL_GetProcAddress("glWindowPos3dv")) != NULL) && r;
	r = ((ELglWindowPos3f = (PFNGLWINDOWPOS3FPROC)SDL_GL_GetProcAddress("glWindowPos3f")) != NULL) && r;
	r = ((ELglWindowPos3fv = (PFNGLWINDOWPOS3FVPROC)SDL_GL_GetProcAddress("glWindowPos3fv")) != NULL) && r;
	r = ((ELglWindowPos3i = (PFNGLWINDOWPOS3IPROC)SDL_GL_GetProcAddress("glWindowPos3i")) != NULL) && r;
	r = ((ELglWindowPos3iv = (PFNGLWINDOWPOS3IVPROC)SDL_GL_GetProcAddress("glWindowPos3iv")) != NULL) && r;
	r = ((ELglWindowPos3s = (PFNGLWINDOWPOS3SPROC)SDL_GL_GetProcAddress("glWindowPos3s")) != NULL) && r;
	r = ((ELglWindowPos3sv = (PFNGLWINDOWPOS3SVPROC)SDL_GL_GetProcAddress("glWindowPos3sv")) != NULL) && r;

	return r;
}
/*	GL_VERSION_1_4		*/

/*	GL_VERSION_1_5		*/
static GLboolean el_init_GL_VERSION_1_5()
{
	GLboolean r = GL_TRUE;

	r = ((ELglBeginQuery = (PFNGLBEGINQUERYPROC)SDL_GL_GetProcAddress("glBeginQuery")) != NULL) && r;
	r = ((ELglBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer")) != NULL) && r;
	r = ((ELglBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData")) != NULL) && r;
	r = ((ELglBufferSubData = (PFNGLBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glBufferSubData")) != NULL) && r;
	r = ((ELglDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers")) != NULL) && r;
	r = ((ELglDeleteQueries = (PFNGLDELETEQUERIESPROC)SDL_GL_GetProcAddress("glDeleteQueries")) != NULL) && r;
	r = ((ELglEndQuery = (PFNGLENDQUERYPROC)SDL_GL_GetProcAddress("glEndQuery")) != NULL) && r;
	r = ((ELglGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers")) != NULL) && r;
	r = ((ELglGenQueries = (PFNGLGENQUERIESPROC)SDL_GL_GetProcAddress("glGenQueries")) != NULL) && r;
	r = ((ELglGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)SDL_GL_GetProcAddress("glGetBufferParameteriv")) != NULL) && r;
	r = ((ELglGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)SDL_GL_GetProcAddress("glGetBufferPointerv")) != NULL) && r;
	r = ((ELglGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)SDL_GL_GetProcAddress("glGetBufferSubData")) != NULL) && r;
	r = ((ELglGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)SDL_GL_GetProcAddress("glGetQueryObjectiv")) != NULL) && r;
	r = ((ELglGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)SDL_GL_GetProcAddress("glGetQueryObjectuiv")) != NULL) && r;
	r = ((ELglGetQueryiv = (PFNGLGETQUERYIVPROC)SDL_GL_GetProcAddress("glGetQueryiv")) != NULL) && r;
	r = ((ELglIsBuffer = (PFNGLISBUFFERPROC)SDL_GL_GetProcAddress("glIsBuffer")) != NULL) && r;
	r = ((ELglIsQuery = (PFNGLISQUERYPROC)SDL_GL_GetProcAddress("glIsQuery")) != NULL) && r;
	r = ((ELglMapBuffer = (PFNGLMAPBUFFERPROC)SDL_GL_GetProcAddress("glMapBuffer")) != NULL) && r;
	r = ((ELglUnmapBuffer = (PFNGLUNMAPBUFFERPROC)SDL_GL_GetProcAddress("glUnmapBuffer")) != NULL) && r;

	return r;
}
/*	GL_VERSION_1_5		*/

/*	GL_VERSION_2_0		*/
static GLboolean el_init_GL_VERSION_2_0()
{
	GLboolean r = GL_TRUE;

	r = ((ELglAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader")) != NULL) && r;
	r = ((ELglBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glBindAttribLocation")) != NULL) && r;
	r = ((ELglBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)SDL_GL_GetProcAddress("glBlendEquationSeparate")) != NULL) && r;
	r = ((ELglCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader")) != NULL) && r;
	r = ((ELglCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram")) != NULL) && r;
	r = ((ELglCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader")) != NULL) && r;
	r = ((ELglDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram")) != NULL) && r;
	r = ((ELglDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader")) != NULL) && r;
	r = ((ELglDetachShader = (PFNGLDETACHSHADERPROC)SDL_GL_GetProcAddress("glDetachShader")) != NULL) && r;
	r = ((ELglDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glDisableVertexAttribArray")) != NULL) && r;
	r = ((ELglDrawBuffers = (PFNGLDRAWBUFFERSPROC)SDL_GL_GetProcAddress("glDrawBuffers")) != NULL) && r;
	r = ((ELglEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray")) != NULL) && r;
	r = ((ELglGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)SDL_GL_GetProcAddress("glGetActiveAttrib")) != NULL) && r;
	r = ((ELglGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)SDL_GL_GetProcAddress("glGetActiveUniform")) != NULL) && r;
	r = ((ELglGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)SDL_GL_GetProcAddress("glGetAttachedShaders")) != NULL) && r;
	r = ((ELglGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glGetAttribLocation")) != NULL) && r;
	r = ((ELglGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog")) != NULL) && r;
	r = ((ELglGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv")) != NULL) && r;
	r = ((ELglGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog")) != NULL) && r;
	r = ((ELglGetShaderSource = (PFNGLGETSHADERSOURCEPROC)SDL_GL_GetProcAddress("glGetShaderSource")) != NULL) && r;
	r = ((ELglGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv")) != NULL) && r;
	r = ((ELglGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation")) != NULL) && r;
	r = ((ELglGetUniformfv = (PFNGLGETUNIFORMFVPROC)SDL_GL_GetProcAddress("glGetUniformfv")) != NULL) && r;
	r = ((ELglGetUniformiv = (PFNGLGETUNIFORMIVPROC)SDL_GL_GetProcAddress("glGetUniformiv")) != NULL) && r;
	r = ((ELglGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)SDL_GL_GetProcAddress("glGetVertexAttribPointerv")) != NULL) && r;
	r = ((ELglGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)SDL_GL_GetProcAddress("glGetVertexAttribdv")) != NULL) && r;
	r = ((ELglGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)SDL_GL_GetProcAddress("glGetVertexAttribfv")) != NULL) && r;
	r = ((ELglGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)SDL_GL_GetProcAddress("glGetVertexAttribiv")) != NULL) && r;
	r = ((ELglIsProgram = (PFNGLISPROGRAMPROC)SDL_GL_GetProcAddress("glIsProgram")) != NULL) && r;
	r = ((ELglIsShader = (PFNGLISSHADERPROC)SDL_GL_GetProcAddress("glIsShader")) != NULL) && r;
	r = ((ELglLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram")) != NULL) && r;
	r = ((ELglShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource")) != NULL) && r;
	r = ((ELglStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)SDL_GL_GetProcAddress("glStencilFuncSeparate")) != NULL) && r;
	r = ((ELglStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)SDL_GL_GetProcAddress("glStencilMaskSeparate")) != NULL) && r;
	r = ((ELglStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)SDL_GL_GetProcAddress("glStencilOpSeparate")) != NULL) && r;
	r = ((ELglUniform1f = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f")) != NULL) && r;
	r = ((ELglUniform1fv = (PFNGLUNIFORM1FVPROC)SDL_GL_GetProcAddress("glUniform1fv")) != NULL) && r;
	r = ((ELglUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i")) != NULL) && r;
	r = ((ELglUniform1iv = (PFNGLUNIFORM1IVPROC)SDL_GL_GetProcAddress("glUniform1iv")) != NULL) && r;
	r = ((ELglUniform2f = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f")) != NULL) && r;
	r = ((ELglUniform2fv = (PFNGLUNIFORM2FVPROC)SDL_GL_GetProcAddress("glUniform2fv")) != NULL) && r;
	r = ((ELglUniform2i = (PFNGLUNIFORM2IPROC)SDL_GL_GetProcAddress("glUniform2i")) != NULL) && r;
	r = ((ELglUniform2iv = (PFNGLUNIFORM2IVPROC)SDL_GL_GetProcAddress("glUniform2iv")) != NULL) && r;
	r = ((ELglUniform3f = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f")) != NULL) && r;
	r = ((ELglUniform3fv = (PFNGLUNIFORM3FVPROC)SDL_GL_GetProcAddress("glUniform3fv")) != NULL) && r;
	r = ((ELglUniform3i = (PFNGLUNIFORM3IPROC)SDL_GL_GetProcAddress("glUniform3i")) != NULL) && r;
	r = ((ELglUniform3iv = (PFNGLUNIFORM3IVPROC)SDL_GL_GetProcAddress("glUniform3iv")) != NULL) && r;
	r = ((ELglUniform4f = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f")) != NULL) && r;
	r = ((ELglUniform4fv = (PFNGLUNIFORM4FVPROC)SDL_GL_GetProcAddress("glUniform4fv")) != NULL) && r;
	r = ((ELglUniform4i = (PFNGLUNIFORM4IPROC)SDL_GL_GetProcAddress("glUniform4i")) != NULL) && r;
	r = ((ELglUniform4iv = (PFNGLUNIFORM4IVPROC)SDL_GL_GetProcAddress("glUniform4iv")) != NULL) && r;
	r = ((ELglUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2fv")) != NULL) && r;
	r = ((ELglUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3fv")) != NULL) && r;
	r = ((ELglUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv")) != NULL) && r;
	r = ((ELglUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram")) != NULL) && r;
	r = ((ELglValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram")) != NULL) && r;
	r = ((ELglVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)SDL_GL_GetProcAddress("glVertexAttrib1d")) != NULL) && r;
	r = ((ELglVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)SDL_GL_GetProcAddress("glVertexAttrib1dv")) != NULL) && r;
	r = ((ELglVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)SDL_GL_GetProcAddress("glVertexAttrib1f")) != NULL) && r;
	r = ((ELglVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)SDL_GL_GetProcAddress("glVertexAttrib1fv")) != NULL) && r;
	r = ((ELglVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)SDL_GL_GetProcAddress("glVertexAttrib1s")) != NULL) && r;
	r = ((ELglVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)SDL_GL_GetProcAddress("glVertexAttrib1sv")) != NULL) && r;
	r = ((ELglVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)SDL_GL_GetProcAddress("glVertexAttrib2d")) != NULL) && r;
	r = ((ELglVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)SDL_GL_GetProcAddress("glVertexAttrib2dv")) != NULL) && r;
	r = ((ELglVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)SDL_GL_GetProcAddress("glVertexAttrib2f")) != NULL) && r;
	r = ((ELglVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)SDL_GL_GetProcAddress("glVertexAttrib2fv")) != NULL) && r;
	r = ((ELglVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)SDL_GL_GetProcAddress("glVertexAttrib2s")) != NULL) && r;
	r = ((ELglVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)SDL_GL_GetProcAddress("glVertexAttrib2sv")) != NULL) && r;
	r = ((ELglVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)SDL_GL_GetProcAddress("glVertexAttrib3d")) != NULL) && r;
	r = ((ELglVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)SDL_GL_GetProcAddress("glVertexAttrib3dv")) != NULL) && r;
	r = ((ELglVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)SDL_GL_GetProcAddress("glVertexAttrib3f")) != NULL) && r;
	r = ((ELglVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)SDL_GL_GetProcAddress("glVertexAttrib3fv")) != NULL) && r;
	r = ((ELglVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)SDL_GL_GetProcAddress("glVertexAttrib3s")) != NULL) && r;
	r = ((ELglVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)SDL_GL_GetProcAddress("glVertexAttrib3sv")) != NULL) && r;
	r = ((ELglVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nbv")) != NULL) && r;
	r = ((ELglVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Niv")) != NULL) && r;
	r = ((ELglVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nsv")) != NULL) && r;
	r = ((ELglVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nub")) != NULL) && r;
	r = ((ELglVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nubv")) != NULL) && r;
	r = ((ELglVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nuiv")) != NULL) && r;
	r = ((ELglVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)SDL_GL_GetProcAddress("glVertexAttrib4Nusv")) != NULL) && r;
	r = ((ELglVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)SDL_GL_GetProcAddress("glVertexAttrib4bv")) != NULL) && r;
	r = ((ELglVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)SDL_GL_GetProcAddress("glVertexAttrib4d")) != NULL) && r;
	r = ((ELglVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)SDL_GL_GetProcAddress("glVertexAttrib4dv")) != NULL) && r;
	r = ((ELglVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)SDL_GL_GetProcAddress("glVertexAttrib4f")) != NULL) && r;
	r = ((ELglVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)SDL_GL_GetProcAddress("glVertexAttrib4fv")) != NULL) && r;
	r = ((ELglVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)SDL_GL_GetProcAddress("glVertexAttrib4iv")) != NULL) && r;
	r = ((ELglVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)SDL_GL_GetProcAddress("glVertexAttrib4s")) != NULL) && r;
	r = ((ELglVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)SDL_GL_GetProcAddress("glVertexAttrib4sv")) != NULL) && r;
	r = ((ELglVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)SDL_GL_GetProcAddress("glVertexAttrib4ubv")) != NULL) && r;
	r = ((ELglVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)SDL_GL_GetProcAddress("glVertexAttrib4uiv")) != NULL) && r;
	r = ((ELglVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)SDL_GL_GetProcAddress("glVertexAttrib4usv")) != NULL) && r;
	r = ((ELglVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer")) != NULL) && r;

	return r;
}
/*	GL_VERSION_2_0		*/

/*	GL_VERSION_2_1		*/
static GLboolean el_init_GL_VERSION_2_1()
{
	GLboolean r = GL_TRUE;

	r = ((ELglUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2x3fv")) != NULL) && r;
	r = ((ELglUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix2x4fv")) != NULL) && r;
	r = ((ELglUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3x2fv")) != NULL) && r;
	r = ((ELglUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix3x4fv")) != NULL) && r;
	r = ((ELglUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4x2fv")) != NULL) && r;
	r = ((ELglUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4x3fv")) != NULL) && r;

	return r;
}
/*	GL_VERSION_2_1		*/

/*	GL_ARB_multitexture	*/
PFNGLACTIVETEXTUREARBPROC ELglActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC ELglClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD1DARBPROC ELglMultiTexCoord1dARB = NULL;
PFNGLMULTITEXCOORD1DVARBPROC ELglMultiTexCoord1dvARB = NULL;
PFNGLMULTITEXCOORD1FARBPROC ELglMultiTexCoord1fARB = NULL;
PFNGLMULTITEXCOORD1FVARBPROC ELglMultiTexCoord1fvARB = NULL;
PFNGLMULTITEXCOORD1IARBPROC ELglMultiTexCoord1iARB = NULL;
PFNGLMULTITEXCOORD1IVARBPROC ELglMultiTexCoord1ivARB = NULL;
PFNGLMULTITEXCOORD1SARBPROC ELglMultiTexCoord1sARB = NULL;
PFNGLMULTITEXCOORD1SVARBPROC ELglMultiTexCoord1svARB = NULL;
PFNGLMULTITEXCOORD2DARBPROC ELglMultiTexCoord2dARB = NULL;
PFNGLMULTITEXCOORD2DVARBPROC ELglMultiTexCoord2dvARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC ELglMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC ELglMultiTexCoord2fvARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC ELglMultiTexCoord2iARB = NULL;
PFNGLMULTITEXCOORD2IVARBPROC ELglMultiTexCoord2ivARB = NULL;
PFNGLMULTITEXCOORD2SARBPROC ELglMultiTexCoord2sARB = NULL;
PFNGLMULTITEXCOORD2SVARBPROC ELglMultiTexCoord2svARB = NULL;
PFNGLMULTITEXCOORD3DARBPROC ELglMultiTexCoord3dARB = NULL;
PFNGLMULTITEXCOORD3DVARBPROC ELglMultiTexCoord3dvARB = NULL;
PFNGLMULTITEXCOORD3FARBPROC ELglMultiTexCoord3fARB = NULL;
PFNGLMULTITEXCOORD3FVARBPROC ELglMultiTexCoord3fvARB = NULL;
PFNGLMULTITEXCOORD3IARBPROC ELglMultiTexCoord3iARB = NULL;
PFNGLMULTITEXCOORD3IVARBPROC ELglMultiTexCoord3ivARB = NULL;
PFNGLMULTITEXCOORD3SARBPROC ELglMultiTexCoord3sARB = NULL;
PFNGLMULTITEXCOORD3SVARBPROC ELglMultiTexCoord3svARB = NULL;
PFNGLMULTITEXCOORD4DARBPROC ELglMultiTexCoord4dARB = NULL;
PFNGLMULTITEXCOORD4DVARBPROC ELglMultiTexCoord4dvARB = NULL;
PFNGLMULTITEXCOORD4FARBPROC ELglMultiTexCoord4fARB = NULL;
PFNGLMULTITEXCOORD4FVARBPROC ELglMultiTexCoord4fvARB = NULL;
PFNGLMULTITEXCOORD4IARBPROC ELglMultiTexCoord4iARB = NULL;
PFNGLMULTITEXCOORD4IVARBPROC ELglMultiTexCoord4ivARB = NULL;
PFNGLMULTITEXCOORD4SARBPROC ELglMultiTexCoord4sARB = NULL;
PFNGLMULTITEXCOORD4SVARBPROC ELglMultiTexCoord4svARB = NULL;
/*	GL_ARB_multitexture		*/

/*	GL_EXT_compiled_vertex_array	*/
PFNGLLOCKARRAYSEXTPROC ELglLockArraysEXT = NULL;
PFNGLUNLOCKARRAYSEXTPROC ELglUnlockArraysEXT = NULL;
/*	GL_EXT_compiled_vertex_array	*/

/*	GL_EXT_draw_range_elements	*/
PFNGLDRAWRANGEELEMENTSEXTPROC ELglDrawRangeElementsEXT = NULL;
/*	GL_EXT_draw_range_elements	*/

/*	GL_ARB_point_parameters		*/
PFNGLPOINTPARAMETERFARBPROC ELglPointParameterfARB = NULL;
PFNGLPOINTPARAMETERFVARBPROC ELglPointParameterfvARB = NULL;
/*	GL_ARB_point_parameters		*/

/*	GL_ARB_vertex_buffer_object	*/
PFNGLBINDBUFFERARBPROC ELglBindBufferARB = NULL;
PFNGLBUFFERDATAARBPROC ELglBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC ELglBufferSubDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC ELglDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC ELglGenBuffersARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC ELglGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC ELglGetBufferPointervARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC ELglGetBufferSubDataARB = NULL;
PFNGLISBUFFERARBPROC ELglIsBufferARB = NULL;
PFNGLMAPBUFFERARBPROC ELglMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC ELglUnmapBufferARB = NULL;
/*	GL_ARB_vertex_buffer_object	*/

/*	GL_EXT_framebuffer_object	*/
PFNGLBINDFRAMEBUFFEREXTPROC ELglBindFramebufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC ELglBindRenderbufferEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC ELglCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC ELglDeleteFramebuffersEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC ELglDeleteRenderbuffersEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC ELglFramebufferRenderbufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC ELglFramebufferTexture1DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC ELglFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC ELglFramebufferTexture3DEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC ELglGenFramebuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC ELglGenRenderbuffersEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC ELglGenerateMipmapEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC ELglGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC ELglGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC ELglIsFramebufferEXT = NULL;
PFNGLISRENDERBUFFEREXTPROC ELglIsRenderbufferEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC ELglRenderbufferStorageEXT = NULL;
/*	GL_EXT_framebuffer_object	*/

/*	GL_ARB_texture_compression	*/
PFNGLCOMPRESSEDTEXIMAGE1DARBPROC ELglCompressedTexImage1DARB = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC ELglCompressedTexImage2DARB = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DARBPROC ELglCompressedTexImage3DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC ELglCompressedTexSubImage1DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC ELglCompressedTexSubImage2DARB = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC ELglCompressedTexSubImage3DARB = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC ELglGetCompressedTexImageARB = NULL;
/*	GL_ARB_texture_compression	*/

/*	GL_ARB_occlusion_query		*/
PFNGLBEGINQUERYARBPROC ELglBeginQueryARB = NULL;
PFNGLDELETEQUERIESARBPROC ELglDeleteQueriesARB = NULL;
PFNGLENDQUERYARBPROC ELglEndQueryARB = NULL;
PFNGLGENQUERIESARBPROC ELglGenQueriesARB = NULL;
PFNGLGETQUERYOBJECTIVARBPROC ELglGetQueryObjectivARB = NULL;
PFNGLGETQUERYOBJECTUIVARBPROC ELglGetQueryObjectuivARB = NULL;
PFNGLGETQUERYIVARBPROC ELglGetQueryivARB = NULL;
PFNGLISQUERYARBPROC ELglIsQueryARB = NULL;
/*	GL_ARB_occlusion_query		*/

/*	GL_ARB_vertex_program		*/
PFNGLBINDPROGRAMARBPROC ELglBindProgramARB = NULL;
PFNGLDELETEPROGRAMSARBPROC ELglDeleteProgramsARB = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC ELglDisableVertexAttribArrayARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC ELglEnableVertexAttribArrayARB = NULL;
PFNGLGENPROGRAMSARBPROC ELglGenProgramsARB = NULL;
PFNGLGETPROGRAMENVPARAMETERDVARBPROC ELglGetProgramEnvParameterdvARB = NULL;
PFNGLGETPROGRAMENVPARAMETERFVARBPROC ELglGetProgramEnvParameterfvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC ELglGetProgramLocalParameterdvARB = NULL;
PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC ELglGetProgramLocalParameterfvARB = NULL;
PFNGLGETPROGRAMSTRINGARBPROC ELglGetProgramStringARB = NULL;
PFNGLGETPROGRAMIVARBPROC ELglGetProgramivARB = NULL;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC ELglGetVertexAttribPointervARB = NULL;
PFNGLGETVERTEXATTRIBDVARBPROC ELglGetVertexAttribdvARB = NULL;
PFNGLGETVERTEXATTRIBFVARBPROC ELglGetVertexAttribfvARB = NULL;
PFNGLGETVERTEXATTRIBIVARBPROC ELglGetVertexAttribivARB = NULL;
PFNGLISPROGRAMARBPROC ELglIsProgramARB = NULL;
PFNGLPROGRAMENVPARAMETER4DARBPROC ELglProgramEnvParameter4dARB = NULL;
PFNGLPROGRAMENVPARAMETER4DVARBPROC ELglProgramEnvParameter4dvARB = NULL;
PFNGLPROGRAMENVPARAMETER4FARBPROC ELglProgramEnvParameter4fARB = NULL;
PFNGLPROGRAMENVPARAMETER4FVARBPROC ELglProgramEnvParameter4fvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DARBPROC ELglProgramLocalParameter4dARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4DVARBPROC ELglProgramLocalParameter4dvARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC ELglProgramLocalParameter4fARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FVARBPROC ELglProgramLocalParameter4fvARB = NULL;
PFNGLPROGRAMSTRINGARBPROC ELglProgramStringARB = NULL;
PFNGLVERTEXATTRIB1DARBPROC ELglVertexAttrib1dARB = NULL;
PFNGLVERTEXATTRIB1DVARBPROC ELglVertexAttrib1dvARB = NULL;
PFNGLVERTEXATTRIB1FARBPROC ELglVertexAttrib1fARB = NULL;
PFNGLVERTEXATTRIB1FVARBPROC ELglVertexAttrib1fvARB = NULL;
PFNGLVERTEXATTRIB1SARBPROC ELglVertexAttrib1sARB = NULL;
PFNGLVERTEXATTRIB1SVARBPROC ELglVertexAttrib1svARB = NULL;
PFNGLVERTEXATTRIB2DARBPROC ELglVertexAttrib2dARB = NULL;
PFNGLVERTEXATTRIB2DVARBPROC ELglVertexAttrib2dvARB = NULL;
PFNGLVERTEXATTRIB2FARBPROC ELglVertexAttrib2fARB = NULL;
PFNGLVERTEXATTRIB2FVARBPROC ELglVertexAttrib2fvARB = NULL;
PFNGLVERTEXATTRIB2SARBPROC ELglVertexAttrib2sARB = NULL;
PFNGLVERTEXATTRIB2SVARBPROC ELglVertexAttrib2svARB = NULL;
PFNGLVERTEXATTRIB3DARBPROC ELglVertexAttrib3dARB = NULL;
PFNGLVERTEXATTRIB3DVARBPROC ELglVertexAttrib3dvARB = NULL;
PFNGLVERTEXATTRIB3FARBPROC ELglVertexAttrib3fARB = NULL;
PFNGLVERTEXATTRIB3FVARBPROC ELglVertexAttrib3fvARB = NULL;
PFNGLVERTEXATTRIB3SARBPROC ELglVertexAttrib3sARB = NULL;
PFNGLVERTEXATTRIB3SVARBPROC ELglVertexAttrib3svARB = NULL;
PFNGLVERTEXATTRIB4NBVARBPROC ELglVertexAttrib4NbvARB = NULL;
PFNGLVERTEXATTRIB4NIVARBPROC ELglVertexAttrib4NivARB = NULL;
PFNGLVERTEXATTRIB4NSVARBPROC ELglVertexAttrib4NsvARB = NULL;
PFNGLVERTEXATTRIB4NUBARBPROC ELglVertexAttrib4NubARB = NULL;
PFNGLVERTEXATTRIB4NUBVARBPROC ELglVertexAttrib4NubvARB = NULL;
PFNGLVERTEXATTRIB4NUIVARBPROC ELglVertexAttrib4NuivARB = NULL;
PFNGLVERTEXATTRIB4NUSVARBPROC ELglVertexAttrib4NusvARB = NULL;
PFNGLVERTEXATTRIB4BVARBPROC ELglVertexAttrib4bvARB = NULL;
PFNGLVERTEXATTRIB4DARBPROC ELglVertexAttrib4dARB = NULL;
PFNGLVERTEXATTRIB4DVARBPROC ELglVertexAttrib4dvARB = NULL;
PFNGLVERTEXATTRIB4FARBPROC ELglVertexAttrib4fARB = NULL;
PFNGLVERTEXATTRIB4FVARBPROC ELglVertexAttrib4fvARB = NULL;
PFNGLVERTEXATTRIB4IVARBPROC ELglVertexAttrib4ivARB = NULL;
PFNGLVERTEXATTRIB4SARBPROC ELglVertexAttrib4sARB = NULL;
PFNGLVERTEXATTRIB4SVARBPROC ELglVertexAttrib4svARB = NULL;
PFNGLVERTEXATTRIB4UBVARBPROC ELglVertexAttrib4ubvARB = NULL;
PFNGLVERTEXATTRIB4UIVARBPROC ELglVertexAttrib4uivARB = NULL;
PFNGLVERTEXATTRIB4USVARBPROC ELglVertexAttrib4usvARB = NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC ELglVertexAttribPointerARB = NULL;
/*	GL_ARB_vertex_program		*/

/*	GL_ARB_vertex_shader		*/
PFNGLBINDATTRIBLOCATIONARBPROC ELglBindAttribLocationARB = NULL;
PFNGLGETACTIVEATTRIBARBPROC ELglGetActiveAttribARB = NULL;
PFNGLGETATTRIBLOCATIONARBPROC ELglGetAttribLocationARB = NULL;
/*	GL_ARB_vertex_shader		*/

/*	GL_ARB_shader_objects		*/
PFNGLATTACHOBJECTARBPROC ELglAttachObjectARB = NULL;
PFNGLCOMPILESHADERARBPROC ELglCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC ELglCreateProgramObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC ELglCreateShaderObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC ELglDeleteObjectARB = NULL;
PFNGLDETACHOBJECTARBPROC ELglDetachObjectARB = NULL;
PFNGLGETACTIVEUNIFORMARBPROC ELglGetActiveUniformARB = NULL;
PFNGLGETATTACHEDOBJECTSARBPROC ELglGetAttachedObjectsARB = NULL;
PFNGLGETHANDLEARBPROC ELglGetHandleARB = NULL;
PFNGLGETINFOLOGARBPROC ELglGetInfoLogARB = NULL;
PFNGLGETOBJECTPARAMETERFVARBPROC ELglGetObjectParameterfvARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC ELglGetObjectParameterivARB = NULL;
PFNGLGETSHADERSOURCEARBPROC ELglGetShaderSourceARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC ELglGetUniformLocationARB = NULL;
PFNGLGETUNIFORMFVARBPROC ELglGetUniformfvARB = NULL;
PFNGLGETUNIFORMIVARBPROC ELglGetUniformivARB = NULL;
PFNGLLINKPROGRAMARBPROC ELglLinkProgramARB = NULL;
PFNGLSHADERSOURCEARBPROC ELglShaderSourceARB = NULL;
PFNGLUNIFORM1FARBPROC ELglUniform1fARB = NULL;
PFNGLUNIFORM1FVARBPROC ELglUniform1fvARB = NULL;
PFNGLUNIFORM1IARBPROC ELglUniform1iARB = NULL;
PFNGLUNIFORM1IVARBPROC ELglUniform1ivARB = NULL;
PFNGLUNIFORM2FARBPROC ELglUniform2fARB = NULL;
PFNGLUNIFORM2FVARBPROC ELglUniform2fvARB = NULL;
PFNGLUNIFORM2IARBPROC ELglUniform2iARB = NULL;
PFNGLUNIFORM2IVARBPROC ELglUniform2ivARB = NULL;
PFNGLUNIFORM3FARBPROC ELglUniform3fARB = NULL;
PFNGLUNIFORM3FVARBPROC ELglUniform3fvARB = NULL;
PFNGLUNIFORM3IARBPROC ELglUniform3iARB = NULL;
PFNGLUNIFORM3IVARBPROC ELglUniform3ivARB = NULL;
PFNGLUNIFORM4FARBPROC ELglUniform4fARB = NULL;
PFNGLUNIFORM4FVARBPROC ELglUniform4fvARB = NULL;
PFNGLUNIFORM4IARBPROC ELglUniform4iARB = NULL;
PFNGLUNIFORM4IVARBPROC ELglUniform4ivARB = NULL;
PFNGLUNIFORMMATRIX2FVARBPROC ELglUniformMatrix2fvARB = NULL;
PFNGLUNIFORMMATRIX3FVARBPROC ELglUniformMatrix3fvARB = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC ELglUniformMatrix4fvARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC ELglUseProgramObjectARB = NULL;
PFNGLVALIDATEPROGRAMARBPROC ELglValidateProgramARB = NULL;
/*	GL_ARB_shader_objects		*/

/*	GL_EXT_fog_coord		*/
PFNGLFOGCOORDPOINTEREXTPROC ELglFogCoordPointerEXT = NULL;
PFNGLFOGCOORDDEXTPROC ELglFogCoorddEXT = NULL;
PFNGLFOGCOORDDVEXTPROC ELglFogCoorddvEXT = NULL;
PFNGLFOGCOORDFEXTPROC ELglFogCoordfEXT = NULL;
PFNGLFOGCOORDFVEXTPROC ELglFogCoordfvEXT = NULL;
/*	GL_EXT_fog_coord		*/

/*	GL_EXT_gpu_program_parameters	*/
PFNGLPROGRAMENVPARAMETERS4FVEXTPROC ELglProgramEnvParameters4fvEXT = NULL;
PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC ELglProgramLocalParameters4fvEXT = NULL;
/*	GL_EXT_gpu_program_parameters	*/

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

static GLboolean el_init_GL_EXT_fog_coord()
{
	GLboolean r = GL_TRUE;

	r = ((ELglFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)SDL_GL_GetProcAddress("glFogCoordPointerEXT")) != NULL) && r;
	r = ((ELglFogCoorddEXT = (PFNGLFOGCOORDDEXTPROC)SDL_GL_GetProcAddress("glFogCoorddEXT")) != NULL) && r;
	r = ((ELglFogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC)SDL_GL_GetProcAddress("glFogCoorddvEXT")) != NULL) && r;
	r = ((ELglFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)SDL_GL_GetProcAddress("glFogCoordfEXT")) != NULL) && r;
	r = ((ELglFogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC)SDL_GL_GetProcAddress("glFogCoordfvEXT")) != NULL) && r;

	return r;
}

static GLboolean el_init_GL_EXT_gpu_program_parameters()
{
	GLboolean r = GL_TRUE;

	r = ((ELglProgramEnvParameters4fvEXT = (PFNGLPROGRAMENVPARAMETERS4FVEXTPROC)SDL_GL_GetProcAddress("glProgramEnvParameters4fvEXT")) != NULL) && r;
	r = ((ELglProgramLocalParameters4fvEXT = (PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC)SDL_GL_GetProcAddress("glProgramLocalParameters4fvEXT")) != NULL) && r;

	return r;
}

void init_opengl_extensions()
{
	GLboolean e;
	char* extensions_string;

	check_for_problem_drivers();

	is_GL_VERSION_1_2 = el_init_GL_VERSION_1_2();
	is_GL_VERSION_1_3 = is_GL_VERSION_1_2 && el_init_GL_VERSION_1_3();
	is_GL_VERSION_1_4 = is_GL_VERSION_1_3 && el_init_GL_VERSION_1_4();
	is_GL_VERSION_1_5 = is_GL_VERSION_1_4 && el_init_GL_VERSION_1_5();
	is_GL_VERSION_2_0 = is_GL_VERSION_1_5 && el_init_GL_VERSION_2_0();
	is_GL_VERSION_2_1 = is_GL_VERSION_2_0 && el_init_GL_VERSION_2_1();

	gl_version = 0;

	if (is_GL_VERSION_1_2)
	{
		gl_version++;
	}

	if (is_GL_VERSION_1_3)
	{
		gl_version++;
	}

	if (is_GL_VERSION_1_4)
	{
		gl_version++;
	}

	if (is_GL_VERSION_1_5)
	{
		gl_version++;
	}

	if (is_GL_VERSION_2_0)
	{
		gl_version++;
	}

	if (is_GL_VERSION_2_1)
	{
		gl_version++;
	}

	extensions_string = (char*)glGetString(GL_EXTENSIONS);

/*	GL_ARB_multitexture			*/
	texture_units = 1;
	if (strstr(extensions_string, "GL_ARB_multitexture") != NULL && !multitexture_problem)
	{
		e = el_init_GL_ARB_multitexture();
		if (e == GL_TRUE)
		{
			extensions |= 1 << arb_multitexture;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &texture_units);
		}
	}
	LOG_DEBUG("GL_MAX_TEXTURE_UNITS_ARB: %d", texture_units);
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
	if (strstr(extensions_string, "GL_ARB_vertex_program") != NULL && !vertex_program_problem)
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
	}
/*	GL_EXT_texture_filter_anisotropic	*/
/*	GL_SGIS_generate_mipmap			*/
	if (strstr(extensions_string, "GL_SGIS_generate_mipmap") != NULL)
	{
		extensions |= 1 << sgis_generate_mipmap;
	}
/*	GL_SGIS_generate_mipmap			*/
/*	GL_ARB_texture_mirrored_repeat		*/
	if (strstr(extensions_string, "GL_ARB_texture_mirrored_repeat") != NULL)
	{
		extensions |= 1 << arb_texture_mirrored_repeat;
	}
/*	GL_ARB_texture_mirrored_repeat		*/
/*	GL_ARB_texture_rectangle		*/
	if (strstr(extensions_string, "GL_ARB_texture_rectangle") != NULL)
	{
		extensions |= 1 << arb_texture_rectangle;
	}
/*	GL_ARB_texture_rectangle		*/
/*	GL_EXT_fog_coord			*/
	if (strstr(extensions_string, "GL_EXT_fog_coord") != NULL)
	{
		e = el_init_GL_EXT_fog_coord();
		if (e == GL_TRUE)
		{
			extensions |= 1 << ext_fog_coord;
		}
	}
/*	GL_EXT_fog_coord			*/
/*	GL_ATI_texture_compression_3dc		*/
	if (strstr(extensions_string, "GL_ATI_texture_compression_3dc") != NULL)
	{
		extensions |= 1 << ati_texture_compression_3dc;
	}
/*	GL_ATI_texture_compression_3dc		*/
/*	GL_EXT_texture_compression_latc		*/
	if (strstr(extensions_string, "GL_EXT_texture_compression_latc") != NULL)
	{
		extensions |= 1 << ext_texture_compression_latc;
	}
/*	GL_EXT_texture_compression_latc		*/
/*	GL_EXT_texture_compression_rgtc		*/
	if (strstr(extensions_string, "GL_EXT_texture_compression_rgtc") != NULL)
	{
		extensions |= 1 << ext_texture_compression_rgtc;
	}
/*	GL_EXT_texture_compression_rgtc		*/
/*	GL_ARB_texture_cube_map			*/
	if (strstr(extensions_string, "GL_ARB_texture_cube_map") != NULL)
	{
		extensions |= 1 << arb_texture_cube_map;
	}
/*	GL_ARB_texture_cube_map			*/
/*	GL_ARB_texture_float			*/
	if (strstr(extensions_string, "GL_ARB_texture_float") != NULL)
	{
		extensions |= 1 << arb_texture_float;
	}
/*	GL_ARB_texture_float			*/
/*	GL_EXT_abgr			*/
	if (strstr(extensions_string, "GL_EXT_abgr") != NULL)
	{
		extensions |= ((Uint64)1) << ext_abgr;
	}
/*	GL_EXT_abgr			*/
/*	GL_EXT_gpu_program_parameters	*/
	if (strstr(extensions_string, "GL_EXT_gpu_program_parameters") != NULL)
	{
		e = el_init_GL_EXT_gpu_program_parameters();
		if (e == GL_TRUE)
		{
			extensions |= ((Uint64)1) << ext_gpu_program_parameters;
		}
	}
/*	GL_EXT_gpu_program_parameters	*/
}

Uint32 have_extension(extension_enum extension)
{
	return (extensions & (1 << extension)) != 0;
}

Uint32 get_texture_units()
{
	return texture_units;
}

const char* get_gl_version_str()
{
	return gl_versions_str[gl_version];
}

GLboolean supports_gl_version(Uint8 major, Uint8 minor)
{
	if (gl_versions[gl_version] >= ((major << 8) + minor))
	{
		return GL_TRUE;
	}
	else
	{
		return GL_FALSE;
	}
}

