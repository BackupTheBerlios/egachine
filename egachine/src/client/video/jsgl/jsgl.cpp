#include <cassert>
#include "jsgl.h"
#include "ecmascript.h"
// TODO: this assumes we use sdl - it is here to include gl.h in a platform
// independant manner
#include "../sdlopengl/sdlgl.h"

#include "jsgl_convert.h"
#include "jsgl_glgetnumargs.h"

extern "C" {
#include "jsgl_funcs.h"
  // some functions we do handcraft
  ECMA_BEGIN_VOID_FUNC (Lightfv)
  {
    ECMA_CHECK_NUM_ARGS (3);
    assert (cx == ECMAScript::cx);
    GLenum light;
    if (!ecma_to_GLenum (argv[0], light))
      ECMA_ERROR ("argument 0 has wrong type");
    GLenum pname;
    if (!ecma_to_GLenum (argv[1], pname))
      ECMA_ERROR ("argument 1 has wrong type");
    int dim=0;
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_POSITION:
      dim=4;
      break;
    case GL_SPOT_DIRECTION:
      dim=3;
      break;
    default:
      ECMA_ERROR("argument 1 has wrong value");
    }
    GLfloat v[dim];
    if (!ecma_to_GLfloat_VEC (argv[2], v, dim))
      ECMA_ERROR ("argument 2 has wrong type");
    glLightfv(light,pname,v);
    return JS_TRUE;
  }
  ECMA_BEGIN_VOID_FUNC (Materialfv)
  {
    ECMA_CHECK_NUM_ARGS (3);
    assert (cx == ECMAScript::cx);
    GLenum face;
    if (!ecma_to_GLenum (argv[0], face))
      ECMA_ERROR ("argument 0 has wrong type");
    GLenum pname;
    if (!ecma_to_GLenum (argv[1], pname))
      ECMA_ERROR ("argument 1 has wrong type");
    int dim=0;
    switch (pname) {
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_AMBIENT_AND_DIFFUSE:
    case GL_SPECULAR:
    case GL_EMISSION:
      dim=4;
      break;
    case GL_COLOR_INDEXES:
      dim=3;
      break;
    default:
      ECMA_ERROR("argument 1 has wrong value");
    }
    GLfloat v[dim];
    if (!ecma_to_GLfloat_VEC (argv[2], v, dim))
      ECMA_ERROR ("argument 2 has wrong type");
    glMaterialfv(face,pname,v);
    return JS_TRUE;
  }
  ECMA_BEGIN_FUNC (GetBooleanv)
  {
    ECMA_CHECK_NUM_ARGS (1);
    assert (cx == ECMAScript::cx);
    GLenum pname;
    if (!ecma_to_GLenum (argv[0], pname))
      ECMA_ERROR ("argument 0 has wrong type");
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      ECMA_ERROR ("unknown parameter");
    GLboolean v[vecsize];
    glGetBooleanv(pname,v);
    if (!ecma_from_boolean_vec(v,vecsize,rval))
      ECMA_ERROR ("could not store result");
    return JS_TRUE;
  }  
  ECMA_BEGIN_FUNC (GetDoublev)
  {
    ECMA_CHECK_NUM_ARGS (1);
    assert (cx == ECMAScript::cx);
    GLenum pname;
    if (!ecma_to_GLenum (argv[0], pname))
      ECMA_ERROR ("argument 0 has wrong type");
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      ECMA_ERROR ("unknown parameter");
    GLdouble v[vecsize];
    glGetDoublev(pname,v);
    if (!ecma_from_number_vec(v,vecsize,rval))
      ECMA_ERROR ("could not store result");
    return JS_TRUE;
  }
  ECMA_BEGIN_FUNC (GetFloatv)
  {
    ECMA_CHECK_NUM_ARGS (1);
    assert (cx == ECMAScript::cx);
    GLenum pname;
    if (!ecma_to_GLenum (argv[0], pname))
      ECMA_ERROR ("argument 0 has wrong type");
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      ECMA_ERROR ("unknown parameter");
    GLfloat v[vecsize];
    glGetFloatv(pname,v);
    if (!ecma_from_number_vec(v,vecsize,rval))
      ECMA_ERROR ("could not store result");
    return JS_TRUE;
  }
  ECMA_BEGIN_FUNC (GetIntegerv)
  {
    ECMA_CHECK_NUM_ARGS (1);
    assert (cx == ECMAScript::cx);
    GLenum pname;
    if (!ecma_to_GLenum (argv[0], pname))
      ECMA_ERROR ("argument 0 has wrong type");
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      ECMA_ERROR ("unknown parameter");
    GLint v[vecsize];
    glGetIntegerv(pname,v);
    if (!ecma_from_number_vec(v,vecsize,rval))
      ECMA_ERROR ("could not store result");
    return JS_TRUE;
  }

#include "jsglu_funcs.h"

  ECMA_BEGIN_FUNC (Project)
  {
    ECMA_CHECK_NUM_ARGS (6);
    assert (cx == ECMAScript::cx);

    // input
    GLdouble objX;
    GLdouble objY;
    GLdouble objZ;
    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];
    
    // output
    GLdouble win[3];

    if (!ecma_to_GLdouble(argv[0], objX)) ECMA_ERROR ("argument 0 has wrong type");
    if (!ecma_to_GLdouble(argv[1], objY)) ECMA_ERROR ("argument 1 has wrong type");
    if (!ecma_to_GLdouble(argv[2], objZ)) ECMA_ERROR ("argument 2 has wrong type");
    if (!ecma_to_GLdouble_VEC (argv[3], model, 16)) ECMA_ERROR ("argument 3 has wrong type");
    if (!ecma_to_GLdouble_VEC (argv[4], proj, 16)) ECMA_ERROR ("argument 4 has wrong type");
    if (!ecma_to_GLint_VEC (argv[5], view, 4)) ECMA_ERROR ("argument 5 has wrong type");

    if (!gluProject(objX, objY, objZ, model, proj, view, &win[0], &win[1], &win[2]))
      return ecma_from_GLboolean(GL_FALSE, rval) ? JS_TRUE : JS_FALSE;
    return ecma_from_number_vec(win, 3, rval) ? JS_TRUE : JS_FALSE;
  }

  ECMA_BEGIN_FUNC (UnProject)
  {
    ECMA_CHECK_NUM_ARGS (6);
    assert (cx == ECMAScript::cx);

    // input
    GLdouble winX;
    GLdouble winY;
    GLdouble winZ;
    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];
    
    // output
    GLdouble obj[3];

    if (!ecma_to_GLdouble(argv[0], winX)) ECMA_ERROR ("argument 0 has wrong type");
    if (!ecma_to_GLdouble(argv[1], winY)) ECMA_ERROR ("argument 1 has wrong type");
    if (!ecma_to_GLdouble(argv[2], winZ)) ECMA_ERROR ("argument 2 has wrong type");
    if (!ecma_to_GLdouble_VEC (argv[3], model, 16)) ECMA_ERROR ("argument 3 has wrong type");
    if (!ecma_to_GLdouble_VEC (argv[4], proj, 16))  ECMA_ERROR ("argument 4 has wrong type");
    if (!ecma_to_GLint_VEC (argv[5], view, 4))      ECMA_ERROR ("argument 5 has wrong type");

    if (!gluUnProject(winX, winY, winZ, model, proj, view, &obj[0], &obj[1], &obj[2]))
      return ecma_from_GLboolean(GL_FALSE, rval) ? JS_TRUE : JS_FALSE;
    return ecma_from_number_vec(obj, 3, rval) ? JS_TRUE : JS_FALSE;
  }

}

static JSFunctionSpec gl_static_methods[] = {
#include "jsgl_fspecs.h"
  ECMA_FUNCSPEC (Lightfv, 3),
  ECMA_FUNCSPEC (Materialfv, 3),
  ECMA_FUNCSPEC (GetBooleanv, 1),
  ECMA_FUNCSPEC (GetDoublev, 1),
  ECMA_FUNCSPEC (GetFloatv, 1),
  ECMA_FUNCSPEC (GetIntegerv, 1),
  ECMA_END_FUNCSPECS
};

static JSFunctionSpec glu_static_methods[] = {
#include "jsglu_fspecs.h"
  ECMA_FUNCSPEC (Project, 6),
  ECMA_FUNCSPEC (UnProject, 6),
  ECMA_END_FUNCSPECS
};

/*
static JSClass gl_class = {
  "gl",0,
  JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
  JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
  ECMA_END_CLASS_SPEC
  };*/

using namespace ECMAScript;

bool
JSGL::init()
{
  JSObject *globj = JS_DefineObject(cx, glob, "gl", NULL, NULL,
				    JSPROP_ENUMERATE);
  if (!globj) return JS_FALSE;
  if (!JS_DefineFunctions(cx, globj, gl_static_methods)) return false;
  
  JSObject *gluobj = JS_DefineObject(cx, glob, "glu", NULL, NULL,
				     JSPROP_ENUMERATE);
  if (!gluobj) return JS_FALSE;
  return JS_DefineFunctions(cx, gluobj, glu_static_methods);
}

bool
JSGL::deinit()
{
  return true;
}
