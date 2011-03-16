#ifndef PFNGLCOPYTEXSUBIMAGE3DPROC
 typedef void (* PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif
#ifndef PFNGLDRAWRANGEELEMENTSPROC
 typedef void (* PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
#endif
#ifndef PFNGLTEXIMAGE3DPROC
 typedef void (* PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
#endif
#ifndef PFNGLTEXSUBIMAGE3DPROC
 typedef void (* PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
#endif
#ifndef PFNGLACTIVETEXTUREPROC
 typedef void (* PFNGLACTIVETEXTUREPROC) (GLenum texture);
#endif
#ifndef PFNGLCLIENTACTIVETEXTUREPROC
 typedef void (* PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE1DPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE2DPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE3DPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLGETCOMPRESSEDTEXIMAGEPROC
 typedef void (* PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);
#endif
#ifndef PFNGLLOADTRANSPOSEMATRIXDPROC
 typedef void (* PFNGLLOADTRANSPOSEMATRIXDPROC) (const GLdouble *m);
#endif
#ifndef PFNGLLOADTRANSPOSEMATRIXFPROC
 typedef void (* PFNGLLOADTRANSPOSEMATRIXFPROC) (const GLfloat *m);
#endif
#ifndef PFNGLMULTTRANSPOSEMATRIXDPROC
 typedef void (* PFNGLMULTTRANSPOSEMATRIXDPROC) (const GLdouble *m);
#endif
#ifndef PFNGLMULTTRANSPOSEMATRIXFPROC
 typedef void (* PFNGLMULTTRANSPOSEMATRIXFPROC) (const GLfloat *m);
#endif
#ifndef PFNGLMULTITEXCOORD1DPROC
 typedef void (* PFNGLMULTITEXCOORD1DPROC) (GLenum target, GLdouble s);
#endif
#ifndef PFNGLMULTITEXCOORD1DVPROC
 typedef void (* PFNGLMULTITEXCOORD1DVPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD1FPROC
 typedef void (* PFNGLMULTITEXCOORD1FPROC) (GLenum target, GLfloat s);
#endif
#ifndef PFNGLMULTITEXCOORD1FVPROC
 typedef void (* PFNGLMULTITEXCOORD1FVPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD1IPROC
 typedef void (* PFNGLMULTITEXCOORD1IPROC) (GLenum target, GLint s);
#endif
#ifndef PFNGLMULTITEXCOORD1IVPROC
 typedef void (* PFNGLMULTITEXCOORD1IVPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD1SPROC
 typedef void (* PFNGLMULTITEXCOORD1SPROC) (GLenum target, GLshort s);
#endif
#ifndef PFNGLMULTITEXCOORD1SVPROC
 typedef void (* PFNGLMULTITEXCOORD1SVPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD2DPROC
 typedef void (* PFNGLMULTITEXCOORD2DPROC) (GLenum target, GLdouble s, GLdouble t);
#endif
#ifndef PFNGLMULTITEXCOORD2DVPROC
 typedef void (* PFNGLMULTITEXCOORD2DVPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD2FPROC
 typedef void (* PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);
#endif
#ifndef PFNGLMULTITEXCOORD2FVPROC
 typedef void (* PFNGLMULTITEXCOORD2FVPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD2IPROC
 typedef void (* PFNGLMULTITEXCOORD2IPROC) (GLenum target, GLint s, GLint t);
#endif
#ifndef PFNGLMULTITEXCOORD2IVPROC
 typedef void (* PFNGLMULTITEXCOORD2IVPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD2SPROC
 typedef void (* PFNGLMULTITEXCOORD2SPROC) (GLenum target, GLshort s, GLshort t);
#endif
#ifndef PFNGLMULTITEXCOORD2SVPROC
 typedef void (* PFNGLMULTITEXCOORD2SVPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD3DPROC
 typedef void (* PFNGLMULTITEXCOORD3DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
#endif
#ifndef PFNGLMULTITEXCOORD3DVPROC
 typedef void (* PFNGLMULTITEXCOORD3DVPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD3FPROC
 typedef void (* PFNGLMULTITEXCOORD3FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
#endif
#ifndef PFNGLMULTITEXCOORD3FVPROC
 typedef void (* PFNGLMULTITEXCOORD3FVPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD3IPROC
 typedef void (* PFNGLMULTITEXCOORD3IPROC) (GLenum target, GLint s, GLint t, GLint r);
#endif
#ifndef PFNGLMULTITEXCOORD3IVPROC
 typedef void (* PFNGLMULTITEXCOORD3IVPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD3SPROC
 typedef void (* PFNGLMULTITEXCOORD3SPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
#endif
#ifndef PFNGLMULTITEXCOORD3SVPROC
 typedef void (* PFNGLMULTITEXCOORD3SVPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD4DPROC
 typedef void (* PFNGLMULTITEXCOORD4DPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
#endif
#ifndef PFNGLMULTITEXCOORD4DVPROC
 typedef void (* PFNGLMULTITEXCOORD4DVPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD4FPROC
 typedef void (* PFNGLMULTITEXCOORD4FPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
#endif
#ifndef PFNGLMULTITEXCOORD4FVPROC
 typedef void (* PFNGLMULTITEXCOORD4FVPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD4IPROC
 typedef void (* PFNGLMULTITEXCOORD4IPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
#endif
#ifndef PFNGLMULTITEXCOORD4IVPROC
 typedef void (* PFNGLMULTITEXCOORD4IVPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD4SPROC
 typedef void (* PFNGLMULTITEXCOORD4SPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
#endif
#ifndef PFNGLMULTITEXCOORD4SVPROC
 typedef void (* PFNGLMULTITEXCOORD4SVPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLSAMPLECOVERAGEPROC
 typedef void (* PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
#endif
#ifndef PFNGLBLENDCOLORPROC
 typedef void (* PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
#endif
#ifndef PFNGLBLENDEQUATIONPROC
 typedef void (* PFNGLBLENDEQUATIONPROC) (GLenum mode);
#endif
#ifndef PFNGLBLENDFUNCSEPARATEPROC
 typedef void (* PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
#endif
#ifndef PFNGLFOGCOORDPOINTERPROC
 typedef void (* PFNGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLFOGCOORDDPROC
 typedef void (* PFNGLFOGCOORDDPROC) (GLdouble coord);
#endif
#ifndef PFNGLFOGCOORDDVPROC
 typedef void (* PFNGLFOGCOORDDVPROC) (const GLdouble *coord);
#endif
#ifndef PFNGLFOGCOORDFPROC
 typedef void (* PFNGLFOGCOORDFPROC) (GLfloat coord);
#endif
#ifndef PFNGLFOGCOORDFVPROC
 typedef void (* PFNGLFOGCOORDFVPROC) (const GLfloat *coord);
#endif
#ifndef PFNGLMULTIDRAWARRAYSPROC
 typedef void (* PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
#endif
#ifndef PFNGLMULTIDRAWELEMENTSPROC
 typedef void (* PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
#endif
#ifndef PFNGLPOINTPARAMETERFPROC
 typedef void (* PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
#endif
#ifndef PFNGLPOINTPARAMETERFVPROC
 typedef void (* PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
#endif
#ifndef PFNGLSECONDARYCOLOR3BPROC
 typedef void (* PFNGLSECONDARYCOLOR3BPROC) (GLbyte red, GLbyte green, GLbyte blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3BVPROC
 typedef void (* PFNGLSECONDARYCOLOR3BVPROC) (const GLbyte *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3DPROC
 typedef void (* PFNGLSECONDARYCOLOR3DPROC) (GLdouble red, GLdouble green, GLdouble blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3DVPROC
 typedef void (* PFNGLSECONDARYCOLOR3DVPROC) (const GLdouble *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3FPROC
 typedef void (* PFNGLSECONDARYCOLOR3FPROC) (GLfloat red, GLfloat green, GLfloat blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3FVPROC
 typedef void (* PFNGLSECONDARYCOLOR3FVPROC) (const GLfloat *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3IPROC
 typedef void (* PFNGLSECONDARYCOLOR3IPROC) (GLint red, GLint green, GLint blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3IVPROC
 typedef void (* PFNGLSECONDARYCOLOR3IVPROC) (const GLint *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3SPROC
 typedef void (* PFNGLSECONDARYCOLOR3SPROC) (GLshort red, GLshort green, GLshort blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3SVPROC
 typedef void (* PFNGLSECONDARYCOLOR3SVPROC) (const GLshort *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3UBPROC
 typedef void (* PFNGLSECONDARYCOLOR3UBPROC) (GLubyte red, GLubyte green, GLubyte blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3UBVPROC
 typedef void (* PFNGLSECONDARYCOLOR3UBVPROC) (const GLubyte *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3UIPROC
 typedef void (* PFNGLSECONDARYCOLOR3UIPROC) (GLuint red, GLuint green, GLuint blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3UIVPROC
 typedef void (* PFNGLSECONDARYCOLOR3UIVPROC) (const GLuint *v);
#endif
#ifndef PFNGLSECONDARYCOLOR3USPROC
 typedef void (* PFNGLSECONDARYCOLOR3USPROC) (GLushort red, GLushort green, GLushort blue);
#endif
#ifndef PFNGLSECONDARYCOLOR3USVPROC
 typedef void (* PFNGLSECONDARYCOLOR3USVPROC) (const GLushort *v);
#endif
#ifndef PFNGLSECONDARYCOLORPOINTERPROC
 typedef void (* PFNGLSECONDARYCOLORPOINTERPROC) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLWINDOWPOS2DPROC
 typedef void (* PFNGLWINDOWPOS2DPROC) (GLdouble x, GLdouble y);
#endif
#ifndef PFNGLWINDOWPOS2DVPROC
 typedef void (* PFNGLWINDOWPOS2DVPROC) (const GLdouble *v);
#endif
#ifndef PFNGLWINDOWPOS2FPROC
 typedef void (* PFNGLWINDOWPOS2FPROC) (GLfloat x, GLfloat y);
#endif
#ifndef PFNGLWINDOWPOS2FVPROC
 typedef void (* PFNGLWINDOWPOS2FVPROC) (const GLfloat *v);
#endif
#ifndef PFNGLWINDOWPOS2IPROC
 typedef void (* PFNGLWINDOWPOS2IPROC) (GLint x, GLint y);
#endif
#ifndef PFNGLWINDOWPOS2IVPROC
 typedef void (* PFNGLWINDOWPOS2IVPROC) (const GLint *v);
#endif
#ifndef PFNGLWINDOWPOS2SPROC
 typedef void (* PFNGLWINDOWPOS2SPROC) (GLshort x, GLshort y);
#endif
#ifndef PFNGLWINDOWPOS2SVPROC
 typedef void (* PFNGLWINDOWPOS2SVPROC) (const GLshort *v);
#endif
#ifndef PFNGLWINDOWPOS3DPROC
 typedef void (* PFNGLWINDOWPOS3DPROC) (GLdouble x, GLdouble y, GLdouble z);
#endif
#ifndef PFNGLWINDOWPOS3DVPROC
 typedef void (* PFNGLWINDOWPOS3DVPROC) (const GLdouble *v);
#endif
#ifndef PFNGLWINDOWPOS3FPROC
 typedef void (* PFNGLWINDOWPOS3FPROC) (GLfloat x, GLfloat y, GLfloat z);
#endif
#ifndef PFNGLWINDOWPOS3FVPROC
 typedef void (* PFNGLWINDOWPOS3FVPROC) (const GLfloat *v);
#endif
#ifndef PFNGLWINDOWPOS3IPROC
 typedef void (* PFNGLWINDOWPOS3IPROC) (GLint x, GLint y, GLint z);
#endif
#ifndef PFNGLWINDOWPOS3IVPROC
 typedef void (* PFNGLWINDOWPOS3IVPROC) (const GLint *v);
#endif
#ifndef PFNGLWINDOWPOS3SPROC
 typedef void (* PFNGLWINDOWPOS3SPROC) (GLshort x, GLshort y, GLshort z);
#endif
#ifndef PFNGLWINDOWPOS3SVPROC
 typedef void (* PFNGLWINDOWPOS3SVPROC) (const GLshort *v);
#endif
#ifndef PFNGLBEGINQUERYPROC
 typedef void (* PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
#endif
#ifndef PFNGLBINDBUFFERPROC
 typedef void (* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
#endif
#ifndef PFNGLBUFFERDATAPROC
 typedef void (* PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
#endif
#ifndef PFNGLBUFFERSUBDATAPROC
 typedef void (* PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
#endif
#ifndef PFNGLDELETEBUFFERSPROC
 typedef void (* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
#endif
#ifndef PFNGLDELETEQUERIESPROC
 typedef void (* PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
#endif
#ifndef PFNGLENDQUERYPROC
 typedef void (* PFNGLENDQUERYPROC) (GLenum target);
#endif
#ifndef PFNGLGENBUFFERSPROC
 typedef void (* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
#endif
#ifndef PFNGLGENQUERIESPROC
 typedef void (* PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
#endif
#ifndef PFNGLGETBUFFERPARAMETERIVPROC
 typedef void (* PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETBUFFERPOINTERVPROC
 typedef void (* PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif
#ifndef PFNGLGETBUFFERSUBDATAPROC
 typedef void (* PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
#endif
#ifndef PFNGLGETQUERYOBJECTIVPROC
 typedef void (* PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETQUERYOBJECTUIVPROC
 typedef void (* PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
#endif
#ifndef PFNGLGETQUERYIVPROC
 typedef void (* PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLISBUFFERPROC
 typedef GLboolean (* PFNGLISBUFFERPROC) (GLuint buffer);
#endif
#ifndef PFNGLISQUERYPROC
 typedef GLboolean (* PFNGLISQUERYPROC) (GLuint id);
#endif
#ifndef PFNGLMAPBUFFERPROC
 typedef GLvoid* (* PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
#endif
#ifndef PFNGLUNMAPBUFFERPROC
 typedef GLboolean (* PFNGLUNMAPBUFFERPROC) (GLenum target);
#endif
#ifndef PFNGLATTACHSHADERPROC
 typedef void (* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
#endif
#ifndef PFNGLBINDATTRIBLOCATIONPROC
 typedef void (* PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
#endif
#ifndef PFNGLBLENDEQUATIONSEPARATEPROC
 typedef void (* PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
#endif
#ifndef PFNGLCOMPILESHADERPROC
 typedef void (* PFNGLCOMPILESHADERPROC) (GLuint shader);
#endif
#ifndef PFNGLCREATEPROGRAMPROC
 typedef GLuint (* PFNGLCREATEPROGRAMPROC) (void);
#endif
#ifndef PFNGLCREATESHADERPROC
 typedef GLuint (* PFNGLCREATESHADERPROC) (GLenum type);
#endif
#ifndef PFNGLDELETEPROGRAMPROC
 typedef void (* PFNGLDELETEPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLDELETESHADERPROC
 typedef void (* PFNGLDELETESHADERPROC) (GLuint shader);
#endif
#ifndef PFNGLDETACHSHADERPROC
 typedef void (* PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
#endif
#ifndef PFNGLDISABLEVERTEXATTRIBARRAYPROC
 typedef void (* PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
#endif
#ifndef PFNGLDRAWBUFFERSPROC
 typedef void (* PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
#endif
#ifndef PFNGLENABLEVERTEXATTRIBARRAYPROC
 typedef void (* PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
#endif
#ifndef PFNGLGETACTIVEATTRIBPROC
 typedef void (* PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
#endif
#ifndef PFNGLGETACTIVEUNIFORMPROC
 typedef void (* PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
#endif
#ifndef PFNGLGETATTACHEDSHADERSPROC
 typedef void (* PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
#endif
#ifndef PFNGLGETATTRIBLOCATIONPROC
 typedef GLint (* PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
#endif
#ifndef PFNGLGETPROGRAMINFOLOGPROC
 typedef void (* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
#endif
#ifndef PFNGLGETPROGRAMIVPROC
 typedef void (* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETSHADERINFOLOGPROC
 typedef void (* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
#endif
#ifndef PFNGLGETSHADERSOURCEPROC
 typedef void (* PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
#endif
#ifndef PFNGLGETSHADERIVPROC
 typedef void (* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETUNIFORMLOCATIONPROC
 typedef GLint (* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
#endif
#ifndef PFNGLGETUNIFORMFVPROC
 typedef void (* PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
#endif
#ifndef PFNGLGETUNIFORMIVPROC
 typedef void (* PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBPOINTERVPROC
 typedef void (* PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
#endif
#ifndef PFNGLGETVERTEXATTRIBDVPROC
 typedef void (* PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBFVPROC
 typedef void (* PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBIVPROC
 typedef void (* PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
#endif
#ifndef PFNGLISPROGRAMPROC
 typedef GLboolean (* PFNGLISPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLISSHADERPROC
 typedef GLboolean (* PFNGLISSHADERPROC) (GLuint shader);
#endif
#ifndef PFNGLLINKPROGRAMPROC
 typedef void (* PFNGLLINKPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLSHADERSOURCEPROC
 typedef void (* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
#endif
#ifndef PFNGLSTENCILFUNCSEPARATEPROC
 typedef void (* PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
#endif
#ifndef PFNGLSTENCILMASKSEPARATEPROC
 typedef void (* PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
#endif
#ifndef PFNGLSTENCILOPSEPARATEPROC
 typedef void (* PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
#endif
#ifndef PFNGLUNIFORM1FPROC
 typedef void (* PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
#endif
#ifndef PFNGLUNIFORM1FVPROC
 typedef void (* PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM1IPROC
 typedef void (* PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
#endif
#ifndef PFNGLUNIFORM1IVPROC
 typedef void (* PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM2FPROC
 typedef void (* PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
#endif
#ifndef PFNGLUNIFORM2FVPROC
 typedef void (* PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM2IPROC
 typedef void (* PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
#endif
#ifndef PFNGLUNIFORM2IVPROC
 typedef void (* PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM3FPROC
 typedef void (* PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
#endif
#ifndef PFNGLUNIFORM3FVPROC
 typedef void (* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM3IPROC
 typedef void (* PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
#endif
#ifndef PFNGLUNIFORM3IVPROC
 typedef void (* PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM4FPROC
 typedef void (* PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
#endif
#ifndef PFNGLUNIFORM4FVPROC
 typedef void (* PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM4IPROC
 typedef void (* PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
#endif
#ifndef PFNGLUNIFORM4IVPROC
 typedef void (* PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORMMATRIX2FVPROC
 typedef void (* PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX3FVPROC
 typedef void (* PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX4FVPROC
 typedef void (* PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUSEPROGRAMPROC
 typedef void (* PFNGLUSEPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLVALIDATEPROGRAMPROC
 typedef void (* PFNGLVALIDATEPROGRAMPROC) (GLuint program);
#endif
#ifndef PFNGLVERTEXATTRIB1DPROC
 typedef void (* PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
#endif
#ifndef PFNGLVERTEXATTRIB1DVPROC
 typedef void (* PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB1FPROC
 typedef void (* PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
#endif
#ifndef PFNGLVERTEXATTRIB1FVPROC
 typedef void (* PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB1SPROC
 typedef void (* PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
#endif
#ifndef PFNGLVERTEXATTRIB1SVPROC
 typedef void (* PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB2DPROC
 typedef void (* PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
#endif
#ifndef PFNGLVERTEXATTRIB2DVPROC
 typedef void (* PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB2FPROC
 typedef void (* PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
#endif
#ifndef PFNGLVERTEXATTRIB2FVPROC
 typedef void (* PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB2SPROC
 typedef void (* PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
#endif
#ifndef PFNGLVERTEXATTRIB2SVPROC
 typedef void (* PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB3DPROC
 typedef void (* PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
#endif
#ifndef PFNGLVERTEXATTRIB3DVPROC
 typedef void (* PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB3FPROC
 typedef void (* PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
#endif
#ifndef PFNGLVERTEXATTRIB3FVPROC
 typedef void (* PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB3SPROC
 typedef void (* PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
#endif
#ifndef PFNGLVERTEXATTRIB3SVPROC
 typedef void (* PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NBVPROC
 typedef void (* PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NIVPROC
 typedef void (* PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NSVPROC
 typedef void (* PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUBPROC
 typedef void (* PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
#endif
#ifndef PFNGLVERTEXATTRIB4NUBVPROC
 typedef void (* PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUIVPROC
 typedef void (* PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUSVPROC
 typedef void (* PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4BVPROC
 typedef void (* PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4DPROC
 typedef void (* PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
#endif
#ifndef PFNGLVERTEXATTRIB4DVPROC
 typedef void (* PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB4FPROC
 typedef void (* PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
#endif
#ifndef PFNGLVERTEXATTRIB4FVPROC
 typedef void (* PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB4IVPROC
 typedef void (* PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4SPROC
 typedef void (* PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
#endif
#ifndef PFNGLVERTEXATTRIB4SVPROC
 typedef void (* PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4UBVPROC
 typedef void (* PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4UIVPROC
 typedef void (* PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4USVPROC
 typedef void (* PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
#endif
#ifndef PFNGLVERTEXATTRIBPOINTERPROC
 typedef void (* PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLUNIFORMMATRIX2X3FVPROC
 typedef void (* PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX2X4FVPROC
 typedef void (* PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX3X2FVPROC
 typedef void (* PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX3X4FVPROC
 typedef void (* PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX4X2FVPROC
 typedef void (* PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX4X3FVPROC
 typedef void (* PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLACTIVETEXTUREARBPROC
 typedef void (* PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
#endif
#ifndef PFNGLCLIENTACTIVETEXTUREARBPROC
 typedef void (* PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
#endif
#ifndef PFNGLMULTITEXCOORD1DARBPROC
 typedef void (* PFNGLMULTITEXCOORD1DARBPROC) (GLenum target, GLdouble s);
#endif
#ifndef PFNGLMULTITEXCOORD1DVARBPROC
 typedef void (* PFNGLMULTITEXCOORD1DVARBPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD1FARBPROC
 typedef void (* PFNGLMULTITEXCOORD1FARBPROC) (GLenum target, GLfloat s);
#endif
#ifndef PFNGLMULTITEXCOORD1FVARBPROC
 typedef void (* PFNGLMULTITEXCOORD1FVARBPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD1IARBPROC
 typedef void (* PFNGLMULTITEXCOORD1IARBPROC) (GLenum target, GLint s);
#endif
#ifndef PFNGLMULTITEXCOORD1IVARBPROC
 typedef void (* PFNGLMULTITEXCOORD1IVARBPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD1SARBPROC
 typedef void (* PFNGLMULTITEXCOORD1SARBPROC) (GLenum target, GLshort s);
#endif
#ifndef PFNGLMULTITEXCOORD1SVARBPROC
 typedef void (* PFNGLMULTITEXCOORD1SVARBPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD2DARBPROC
 typedef void (* PFNGLMULTITEXCOORD2DARBPROC) (GLenum target, GLdouble s, GLdouble t);
#endif
#ifndef PFNGLMULTITEXCOORD2DVARBPROC
 typedef void (* PFNGLMULTITEXCOORD2DVARBPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD2FARBPROC
 typedef void (* PFNGLMULTITEXCOORD2FARBPROC) (GLenum target, GLfloat s, GLfloat t);
#endif
#ifndef PFNGLMULTITEXCOORD2FVARBPROC
 typedef void (* PFNGLMULTITEXCOORD2FVARBPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD2IARBPROC
 typedef void (* PFNGLMULTITEXCOORD2IARBPROC) (GLenum target, GLint s, GLint t);
#endif
#ifndef PFNGLMULTITEXCOORD2IVARBPROC
 typedef void (* PFNGLMULTITEXCOORD2IVARBPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD2SARBPROC
 typedef void (* PFNGLMULTITEXCOORD2SARBPROC) (GLenum target, GLshort s, GLshort t);
#endif
#ifndef PFNGLMULTITEXCOORD2SVARBPROC
 typedef void (* PFNGLMULTITEXCOORD2SVARBPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD3DARBPROC
 typedef void (* PFNGLMULTITEXCOORD3DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
#endif
#ifndef PFNGLMULTITEXCOORD3DVARBPROC
 typedef void (* PFNGLMULTITEXCOORD3DVARBPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD3FARBPROC
 typedef void (* PFNGLMULTITEXCOORD3FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
#endif
#ifndef PFNGLMULTITEXCOORD3FVARBPROC
 typedef void (* PFNGLMULTITEXCOORD3FVARBPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD3IARBPROC
 typedef void (* PFNGLMULTITEXCOORD3IARBPROC) (GLenum target, GLint s, GLint t, GLint r);
#endif
#ifndef PFNGLMULTITEXCOORD3IVARBPROC
 typedef void (* PFNGLMULTITEXCOORD3IVARBPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD3SARBPROC
 typedef void (* PFNGLMULTITEXCOORD3SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r);
#endif
#ifndef PFNGLMULTITEXCOORD3SVARBPROC
 typedef void (* PFNGLMULTITEXCOORD3SVARBPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLMULTITEXCOORD4DARBPROC
 typedef void (* PFNGLMULTITEXCOORD4DARBPROC) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
#endif
#ifndef PFNGLMULTITEXCOORD4DVARBPROC
 typedef void (* PFNGLMULTITEXCOORD4DVARBPROC) (GLenum target, const GLdouble *v);
#endif
#ifndef PFNGLMULTITEXCOORD4FARBPROC
 typedef void (* PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
#endif
#ifndef PFNGLMULTITEXCOORD4FVARBPROC
 typedef void (* PFNGLMULTITEXCOORD4FVARBPROC) (GLenum target, const GLfloat *v);
#endif
#ifndef PFNGLMULTITEXCOORD4IARBPROC
 typedef void (* PFNGLMULTITEXCOORD4IARBPROC) (GLenum target, GLint s, GLint t, GLint r, GLint q);
#endif
#ifndef PFNGLMULTITEXCOORD4IVARBPROC
 typedef void (* PFNGLMULTITEXCOORD4IVARBPROC) (GLenum target, const GLint *v);
#endif
#ifndef PFNGLMULTITEXCOORD4SARBPROC
 typedef void (* PFNGLMULTITEXCOORD4SARBPROC) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
#endif
#ifndef PFNGLMULTITEXCOORD4SVARBPROC
 typedef void (* PFNGLMULTITEXCOORD4SVARBPROC) (GLenum target, const GLshort *v);
#endif
#ifndef PFNGLLOCKARRAYSEXTPROC
 typedef void (* PFNGLLOCKARRAYSEXTPROC) (GLint first, GLsizei count);
#endif
#ifndef PFNGLUNLOCKARRAYSEXTPROC
 typedef void (* PFNGLUNLOCKARRAYSEXTPROC) (void);
#endif
#ifndef PFNGLDRAWRANGEELEMENTSEXTPROC
 typedef void (* PFNGLDRAWRANGEELEMENTSEXTPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
#endif
#ifndef PFNGLPOINTPARAMETERFARBPROC
 typedef void (* PFNGLPOINTPARAMETERFARBPROC) (GLenum pname, GLfloat param);
#endif
#ifndef PFNGLPOINTPARAMETERFVARBPROC
 typedef void (* PFNGLPOINTPARAMETERFVARBPROC) (GLenum pname, const GLfloat *params);
#endif
#ifndef PFNGLBINDBUFFERARBPROC
 typedef void (* PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
#endif
#ifndef PFNGLBUFFERDATAARBPROC
 typedef void (* PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
#endif
#ifndef PFNGLBUFFERSUBDATAARBPROC
 typedef void (* PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
#endif
#ifndef PFNGLDELETEBUFFERSARBPROC
 typedef void (* PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
#endif
#ifndef PFNGLGENBUFFERSARBPROC
 typedef void (* PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
#endif
#ifndef PFNGLGETBUFFERPARAMETERIVARBPROC
 typedef void (* PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETBUFFERPOINTERVARBPROC
 typedef void (* PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
#endif
#ifndef PFNGLGETBUFFERSUBDATAARBPROC
 typedef void (* PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
#endif
#ifndef PFNGLISBUFFERARBPROC
 typedef GLboolean (* PFNGLISBUFFERARBPROC) (GLuint buffer);
#endif
#ifndef PFNGLMAPBUFFERARBPROC
 typedef GLvoid* (* PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
#endif
#ifndef PFNGLUNMAPBUFFERARBPROC
 typedef GLboolean (* PFNGLUNMAPBUFFERARBPROC) (GLenum target);
#endif
#ifndef PFNGLBINDFRAMEBUFFEREXTPROC
 typedef void (* PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
#endif
#ifndef PFNGLBINDRENDERBUFFEREXTPROC
 typedef void (* PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
#endif
#ifndef PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC
 typedef GLenum (* PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
#endif
#ifndef PFNGLDELETEFRAMEBUFFERSEXTPROC
 typedef void (* PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
#endif
#ifndef PFNGLDELETERENDERBUFFERSEXTPROC
 typedef void (* PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
#endif
#ifndef PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC
 typedef void (* PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
#endif
#ifndef PFNGLFRAMEBUFFERTEXTURE1DEXTPROC
 typedef void (* PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
#endif
#ifndef PFNGLFRAMEBUFFERTEXTURE2DEXTPROC
 typedef void (* PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
#endif
#ifndef PFNGLFRAMEBUFFERTEXTURE3DEXTPROC
 typedef void (* PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
#endif
#ifndef PFNGLGENFRAMEBUFFERSEXTPROC
 typedef void (* PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
#endif
#ifndef PFNGLGENRENDERBUFFERSEXTPROC
 typedef void (* PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
#endif
#ifndef PFNGLGENERATEMIPMAPEXTPROC
 typedef void (* PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);
#endif
#ifndef PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
 typedef void (* PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC
 typedef void (* PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLISFRAMEBUFFEREXTPROC
 typedef GLboolean (* PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
#endif
#ifndef PFNGLISRENDERBUFFEREXTPROC
 typedef GLboolean (* PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
#endif
#ifndef PFNGLRENDERBUFFERSTORAGEEXTPROC
 typedef void (* PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE1DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE1DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE2DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE2DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXIMAGE3DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXIMAGE3DARBPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE1DARBPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC
 typedef void (* PFNGLCOMPRESSEDTEXSUBIMAGE3DARBPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
#endif
#ifndef PFNGLGETCOMPRESSEDTEXIMAGEARBPROC
 typedef void (* PFNGLGETCOMPRESSEDTEXIMAGEARBPROC) (GLenum target, GLint level, GLvoid *img);
#endif
#ifndef PFNGLBEGINQUERYARBPROC
 typedef void (* PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
#endif
#ifndef PFNGLDELETEQUERIESARBPROC
 typedef void (* PFNGLDELETEQUERIESARBPROC) (GLsizei n, const GLuint *ids);
#endif
#ifndef PFNGLENDQUERYARBPROC
 typedef void (* PFNGLENDQUERYARBPROC) (GLenum target);
#endif
#ifndef PFNGLGENQUERIESARBPROC
 typedef void (* PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint *ids);
#endif
#ifndef PFNGLGETQUERYOBJECTIVARBPROC
 typedef void (* PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETQUERYOBJECTUIVARBPROC
 typedef void (* PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname, GLuint *params);
#endif
#ifndef PFNGLGETQUERYIVARBPROC
 typedef void (* PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLISQUERYARBPROC
 typedef GLboolean (* PFNGLISQUERYARBPROC) (GLuint id);
#endif
#ifndef PFNGLBINDPROGRAMARBPROC
 typedef void (* PFNGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
#endif
#ifndef PFNGLDELETEPROGRAMSARBPROC
 typedef void (* PFNGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
#endif
#ifndef PFNGLDISABLEVERTEXATTRIBARRAYARBPROC
 typedef void (* PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
#endif
#ifndef PFNGLENABLEVERTEXATTRIBARRAYARBPROC
 typedef void (* PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
#endif
#ifndef PFNGLGENPROGRAMSARBPROC
 typedef void (* PFNGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
#endif
#ifndef PFNGLGETPROGRAMENVPARAMETERDVARBPROC
 typedef void (* PFNGLGETPROGRAMENVPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
#endif
#ifndef PFNGLGETPROGRAMENVPARAMETERFVARBPROC
 typedef void (* PFNGLGETPROGRAMENVPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
#endif
#ifndef PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC
 typedef void (* PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) (GLenum target, GLuint index, GLdouble *params);
#endif
#ifndef PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC
 typedef void (* PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC) (GLenum target, GLuint index, GLfloat *params);
#endif
#ifndef PFNGLGETPROGRAMSTRINGARBPROC
 typedef void (* PFNGLGETPROGRAMSTRINGARBPROC) (GLenum target, GLenum pname, GLvoid *string);
#endif
#ifndef PFNGLGETPROGRAMIVARBPROC
 typedef void (* PFNGLGETPROGRAMIVARBPROC) (GLenum target, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBPOINTERVARBPROC
 typedef void (* PFNGLGETVERTEXATTRIBPOINTERVARBPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
#endif
#ifndef PFNGLGETVERTEXATTRIBDVARBPROC
 typedef void (* PFNGLGETVERTEXATTRIBDVARBPROC) (GLuint index, GLenum pname, GLdouble *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBFVARBPROC
 typedef void (* PFNGLGETVERTEXATTRIBFVARBPROC) (GLuint index, GLenum pname, GLfloat *params);
#endif
#ifndef PFNGLGETVERTEXATTRIBIVARBPROC
 typedef void (* PFNGLGETVERTEXATTRIBIVARBPROC) (GLuint index, GLenum pname, GLint *params);
#endif
#ifndef PFNGLISPROGRAMARBPROC
 typedef GLboolean (* PFNGLISPROGRAMARBPROC) (GLuint program);
#endif
#ifndef PFNGLPROGRAMENVPARAMETER4DARBPROC
 typedef void (* PFNGLPROGRAMENVPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
#endif
#ifndef PFNGLPROGRAMENVPARAMETER4DVARBPROC
 typedef void (* PFNGLPROGRAMENVPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
#endif
#ifndef PFNGLPROGRAMENVPARAMETER4FARBPROC
 typedef void (* PFNGLPROGRAMENVPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
#endif
#ifndef PFNGLPROGRAMENVPARAMETER4FVARBPROC
 typedef void (* PFNGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETER4DARBPROC
 typedef void (* PFNGLPROGRAMLOCALPARAMETER4DARBPROC) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETER4DVARBPROC
 typedef void (* PFNGLPROGRAMLOCALPARAMETER4DVARBPROC) (GLenum target, GLuint index, const GLdouble *params);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETER4FARBPROC
 typedef void (* PFNGLPROGRAMLOCALPARAMETER4FARBPROC) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETER4FVARBPROC
 typedef void (* PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
#endif
#ifndef PFNGLPROGRAMSTRINGARBPROC
 typedef void (* PFNGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
#endif
#ifndef PFNGLVERTEXATTRIB1DARBPROC
 typedef void (* PFNGLVERTEXATTRIB1DARBPROC) (GLuint index, GLdouble x);
#endif
#ifndef PFNGLVERTEXATTRIB1DVARBPROC
 typedef void (* PFNGLVERTEXATTRIB1DVARBPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB1FARBPROC
 typedef void (* PFNGLVERTEXATTRIB1FARBPROC) (GLuint index, GLfloat x);
#endif
#ifndef PFNGLVERTEXATTRIB1FVARBPROC
 typedef void (* PFNGLVERTEXATTRIB1FVARBPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB1SARBPROC
 typedef void (* PFNGLVERTEXATTRIB1SARBPROC) (GLuint index, GLshort x);
#endif
#ifndef PFNGLVERTEXATTRIB1SVARBPROC
 typedef void (* PFNGLVERTEXATTRIB1SVARBPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB2DARBPROC
 typedef void (* PFNGLVERTEXATTRIB2DARBPROC) (GLuint index, GLdouble x, GLdouble y);
#endif
#ifndef PFNGLVERTEXATTRIB2DVARBPROC
 typedef void (* PFNGLVERTEXATTRIB2DVARBPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB2FARBPROC
 typedef void (* PFNGLVERTEXATTRIB2FARBPROC) (GLuint index, GLfloat x, GLfloat y);
#endif
#ifndef PFNGLVERTEXATTRIB2FVARBPROC
 typedef void (* PFNGLVERTEXATTRIB2FVARBPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB2SARBPROC
 typedef void (* PFNGLVERTEXATTRIB2SARBPROC) (GLuint index, GLshort x, GLshort y);
#endif
#ifndef PFNGLVERTEXATTRIB2SVARBPROC
 typedef void (* PFNGLVERTEXATTRIB2SVARBPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB3DARBPROC
 typedef void (* PFNGLVERTEXATTRIB3DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
#endif
#ifndef PFNGLVERTEXATTRIB3DVARBPROC
 typedef void (* PFNGLVERTEXATTRIB3DVARBPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB3FARBPROC
 typedef void (* PFNGLVERTEXATTRIB3FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
#endif
#ifndef PFNGLVERTEXATTRIB3FVARBPROC
 typedef void (* PFNGLVERTEXATTRIB3FVARBPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB3SARBPROC
 typedef void (* PFNGLVERTEXATTRIB3SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
#endif
#ifndef PFNGLVERTEXATTRIB3SVARBPROC
 typedef void (* PFNGLVERTEXATTRIB3SVARBPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NBVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NBVARBPROC) (GLuint index, const GLbyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NIVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NIVARBPROC) (GLuint index, const GLint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NSVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NSVARBPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUBARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NUBARBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
#endif
#ifndef PFNGLVERTEXATTRIB4NUBVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NUBVARBPROC) (GLuint index, const GLubyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUIVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NUIVARBPROC) (GLuint index, const GLuint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4NUSVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4NUSVARBPROC) (GLuint index, const GLushort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4BVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4BVARBPROC) (GLuint index, const GLbyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4DARBPROC
 typedef void (* PFNGLVERTEXATTRIB4DARBPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
#endif
#ifndef PFNGLVERTEXATTRIB4DVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4DVARBPROC) (GLuint index, const GLdouble *v);
#endif
#ifndef PFNGLVERTEXATTRIB4FARBPROC
 typedef void (* PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
#endif
#ifndef PFNGLVERTEXATTRIB4FVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4FVARBPROC) (GLuint index, const GLfloat *v);
#endif
#ifndef PFNGLVERTEXATTRIB4IVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4IVARBPROC) (GLuint index, const GLint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4SARBPROC
 typedef void (* PFNGLVERTEXATTRIB4SARBPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
#endif
#ifndef PFNGLVERTEXATTRIB4SVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4SVARBPROC) (GLuint index, const GLshort *v);
#endif
#ifndef PFNGLVERTEXATTRIB4UBVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4UBVARBPROC) (GLuint index, const GLubyte *v);
#endif
#ifndef PFNGLVERTEXATTRIB4UIVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4UIVARBPROC) (GLuint index, const GLuint *v);
#endif
#ifndef PFNGLVERTEXATTRIB4USVARBPROC
 typedef void (* PFNGLVERTEXATTRIB4USVARBPROC) (GLuint index, const GLushort *v);
#endif
#ifndef PFNGLVERTEXATTRIBPOINTERARBPROC
 typedef void (* PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLBINDATTRIBLOCATIONARBPROC
 typedef void (* PFNGLBINDATTRIBLOCATIONARBPROC) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
#endif
#ifndef PFNGLGETACTIVEATTRIBARBPROC
 typedef void (* PFNGLGETACTIVEATTRIBARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
#endif
#ifndef PFNGLGETATTRIBLOCATIONARBPROC
 typedef GLint (* PFNGLGETATTRIBLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
#endif
#ifndef PFNGLATTACHOBJECTARBPROC
 typedef void (* PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
#endif
#ifndef PFNGLCOMPILESHADERARBPROC
 typedef void (* PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
#endif
#ifndef PFNGLCREATEPROGRAMOBJECTARBPROC
 typedef GLhandleARB (* PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
#endif
#ifndef PFNGLCREATESHADEROBJECTARBPROC
 typedef GLhandleARB (* PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
#endif
#ifndef PFNGLDELETEOBJECTARBPROC
 typedef void (* PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
#endif
#ifndef PFNGLDETACHOBJECTARBPROC
 typedef void (* PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
#endif
#ifndef PFNGLGETACTIVEUNIFORMARBPROC
 typedef void (* PFNGLGETACTIVEUNIFORMARBPROC) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
#endif
#ifndef PFNGLGETATTACHEDOBJECTSARBPROC
 typedef void (* PFNGLGETATTACHEDOBJECTSARBPROC) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
#endif
#ifndef PFNGLGETHANDLEARBPROC
 typedef GLhandleARB (* PFNGLGETHANDLEARBPROC) (GLenum pname);
#endif
#ifndef PFNGLGETINFOLOGARBPROC
 typedef void (* PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
#endif
#ifndef PFNGLGETOBJECTPARAMETERFVARBPROC
 typedef void (* PFNGLGETOBJECTPARAMETERFVARBPROC) (GLhandleARB obj, GLenum pname, GLfloat *params);
#endif
#ifndef PFNGLGETOBJECTPARAMETERIVARBPROC
 typedef void (* PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
#endif
#ifndef PFNGLGETSHADERSOURCEARBPROC
 typedef void (* PFNGLGETSHADERSOURCEARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
#endif
#ifndef PFNGLGETUNIFORMLOCATIONARBPROC
 typedef GLint (* PFNGLGETUNIFORMLOCATIONARBPROC) (GLhandleARB programObj, const GLcharARB *name);
#endif
#ifndef PFNGLGETUNIFORMFVARBPROC
 typedef void (* PFNGLGETUNIFORMFVARBPROC) (GLhandleARB programObj, GLint location, GLfloat *params);
#endif
#ifndef PFNGLGETUNIFORMIVARBPROC
 typedef void (* PFNGLGETUNIFORMIVARBPROC) (GLhandleARB programObj, GLint location, GLint *params);
#endif
#ifndef PFNGLLINKPROGRAMARBPROC
 typedef void (* PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
#endif
#ifndef PFNGLSHADERSOURCEARBPROC
 typedef void (* PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
#endif
#ifndef PFNGLUNIFORM1FARBPROC
 typedef void (* PFNGLUNIFORM1FARBPROC) (GLint location, GLfloat v0);
#endif
#ifndef PFNGLUNIFORM1FVARBPROC
 typedef void (* PFNGLUNIFORM1FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM1IARBPROC
 typedef void (* PFNGLUNIFORM1IARBPROC) (GLint location, GLint v0);
#endif
#ifndef PFNGLUNIFORM1IVARBPROC
 typedef void (* PFNGLUNIFORM1IVARBPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM2FARBPROC
 typedef void (* PFNGLUNIFORM2FARBPROC) (GLint location, GLfloat v0, GLfloat v1);
#endif
#ifndef PFNGLUNIFORM2FVARBPROC
 typedef void (* PFNGLUNIFORM2FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM2IARBPROC
 typedef void (* PFNGLUNIFORM2IARBPROC) (GLint location, GLint v0, GLint v1);
#endif
#ifndef PFNGLUNIFORM2IVARBPROC
 typedef void (* PFNGLUNIFORM2IVARBPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM3FARBPROC
 typedef void (* PFNGLUNIFORM3FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
#endif
#ifndef PFNGLUNIFORM3FVARBPROC
 typedef void (* PFNGLUNIFORM3FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM3IARBPROC
 typedef void (* PFNGLUNIFORM3IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2);
#endif
#ifndef PFNGLUNIFORM3IVARBPROC
 typedef void (* PFNGLUNIFORM3IVARBPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORM4FARBPROC
 typedef void (* PFNGLUNIFORM4FARBPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
#endif
#ifndef PFNGLUNIFORM4FVARBPROC
 typedef void (* PFNGLUNIFORM4FVARBPROC) (GLint location, GLsizei count, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORM4IARBPROC
 typedef void (* PFNGLUNIFORM4IARBPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
#endif
#ifndef PFNGLUNIFORM4IVARBPROC
 typedef void (* PFNGLUNIFORM4IVARBPROC) (GLint location, GLsizei count, const GLint *value);
#endif
#ifndef PFNGLUNIFORMMATRIX2FVARBPROC
 typedef void (* PFNGLUNIFORMMATRIX2FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX3FVARBPROC
 typedef void (* PFNGLUNIFORMMATRIX3FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUNIFORMMATRIX4FVARBPROC
 typedef void (* PFNGLUNIFORMMATRIX4FVARBPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#endif
#ifndef PFNGLUSEPROGRAMOBJECTARBPROC
 typedef void (* PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
#endif
#ifndef PFNGLVALIDATEPROGRAMARBPROC
 typedef void (* PFNGLVALIDATEPROGRAMARBPROC) (GLhandleARB programObj);
#endif
#ifndef PFNGLFOGCOORDPOINTEREXTPROC
 typedef void (* PFNGLFOGCOORDPOINTEREXTPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
#endif
#ifndef PFNGLFOGCOORDDEXTPROC
 typedef void (* PFNGLFOGCOORDDEXTPROC) (GLdouble coord);
#endif
#ifndef PFNGLFOGCOORDDVEXTPROC
 typedef void (* PFNGLFOGCOORDDVEXTPROC) (const GLdouble *coord);
#endif
#ifndef PFNGLFOGCOORDFEXTPROC
 typedef void (* PFNGLFOGCOORDFEXTPROC) (GLfloat coord);
#endif
#ifndef PFNGLFOGCOORDFVEXTPROC
 typedef void (* PFNGLFOGCOORDFVEXTPROC) (const GLfloat *coord);
#endif
#ifndef PFNGLPROGRAMENVPARAMETERS4FVEXTPROC
 typedef void (* PFNGLPROGRAMENVPARAMETERS4FVEXTPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif
#ifndef PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC
 typedef void (* PFNGLPROGRAMLOCALPARAMETERS4FVEXTPROC) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
#endif
