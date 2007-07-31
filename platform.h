/*!
 * \file
 * \ingroup     misc
 * \brief       Various defines and constants to make EL compile on different platforms.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <math.h>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif //M_PI

#ifdef OSX
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <OpenGL/glext.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glext.h>
#endif


#endif // PLATFORM_H
