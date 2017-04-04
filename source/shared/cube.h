#ifndef __CUBE_H__
#define __CUBE_H__

#define _FILE_OFFSET_BITS 64

#ifdef __GNUC__
#define gamma __gamma
#endif

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#ifdef __GNUC__
#undef gamma
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include <time.h>

//#define AS_USE_NAMESPACE false;
//#ifdef AS_USE_NAMESPACE
//#define BEGIN_AS_NAMESPACE namespace AngelScript {
//#define END_AS_NAMESPACE }
//#define AS_NAMESPACE_QUALIFIER AngelScript::
//#else
#define BEGIN_AS_NAMESPACE
#define END_AS_NAMESPACE
#define AS_NAMESPACE_QUALIFIER ::
//#endif

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #ifdef _WIN32_WINNT
  #undef _WIN32_WINNT
  #endif
  #define _WIN32_WINNT 0x0500
  #include "windows.h"
  #ifndef _WINDOWS
    #define _WINDOWS
  #endif
  #ifndef __GNUC__
    #include <eh.h>
    #include <dbghelp.h>
    #include <intrin.h>
  #endif
  #define ZLIB_DLL
#endif

// original
/*#ifndef STANDALONE
  #ifdef __APPLE__
    #include "SDL2/SDL.h"
    #include "SDL2/SDL_opengl.h"
    #define main SDL_main
  #else
    #include <SDL.h>
    #include <SDL_opengl.h>
  #endif
#endif
*/

#ifndef STANDALONE
  #ifdef __APPLE__
    #include "SDL2/SDL.h"
    #define GL_GLEXT_LEGACY
    #define __glext_h_
    #include <OpenGL/gl.h>
   // #define main SDL_main
    #define SDL_MAIN_NEEDED
  #else
    #include <SDL.h>
    #include <SDL_opengl.h>
  #endif
#endif

#include <enet/enet.h>

#include <zlib.h>

#include "tools.h"
#include "geom.h"
#include "halo.h"
#include "node.h"
#include "ents.h"

#include "command.h"

#ifndef STANDALONE
#include "glexts.h"
#include "glemu.h"
#endif

#include "iengine.h"
#include "igame.h"

#endif

