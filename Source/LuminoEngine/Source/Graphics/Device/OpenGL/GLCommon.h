
#pragma once

#define GLEW_STATIC
#include "../External/GL/glew.h"
//#include <GL/gl.h>
#include "../External/GL/glext.h"

#if defined(LN_OS_WIN32)
#include <Objbase.h>
//#include <GL/wglew.h>
#include "../External/GL/wglext.h"
#elif defined(LN_OS_X11)
#include <GL/glx.h>
#include "../External/GL/glxext.h"
#endif

#include "LNGL.h"

// glGetError() でエラーチェック (各 gl〜 の後で必ず呼ばないと正しいエラーが取れないので注意)
#define LN_CHECK_GLERROR()		{ GLenum lnglerr = glGetError(); LN_THROW(lnglerr == GL_NO_ERROR , OpenGLException, lnglerr); } 
