#include <cassert>
// TODO: this is probably not portable
#include "GL/gl.h"
#include "GL/glu.h"

#include "ejsmodule.h"
#include "jsgl_glgetnumargs.h"

// TODO: this does not work with ints bigger than 32bit
template <typename I>
inline
JSBool to_int(JSContext* cx, JSObject*, jsval x,I &y)
{
  int32 i;
  JSBool r=JS_ValueToInt32(cx,x,&i);
  y=i; // TODO: check for overflow?
  return r;
}

template <typename F>
inline
JSBool to_float(JSContext* cx, JSObject*, jsval x,F &y)
{
  jsdouble d;
  JSBool r=JS_ValueToNumber(cx,x,&d);
  y=d; // TODO: check for overflow? - check for NaN?
  return r;
}

inline JSBool to_GLbyte (JSContext* cx, JSObject* obj, jsval x,GLbyte &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLboolean (JSContext* cx, JSObject* obj, jsval x,GLboolean &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLubyte (JSContext* cx, JSObject* obj, jsval x,GLubyte &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLushort (JSContext* cx, JSObject* obj, jsval x,GLushort &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLshort (JSContext* cx, JSObject* obj, jsval x,GLshort &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLint (JSContext* cx, JSObject* obj, jsval x,GLint &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLsizei (JSContext* cx, JSObject* obj, jsval x,GLsizei &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLuint (JSContext* cx, JSObject* obj, jsval x,GLuint &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLenum (JSContext* cx, JSObject* obj, jsval x,GLenum &y){return to_int(cx,obj,x,y);}
inline JSBool to_GLbitfield (JSContext* cx, JSObject* obj, jsval x,GLbitfield &y){return to_int(cx,obj,x,y);}

inline JSBool to_GLfloat (JSContext* cx, JSObject* obj, jsval x,GLfloat &y){return to_float(cx,obj,x,y);}
inline JSBool to_GLclampf (JSContext* cx, JSObject* obj, jsval x,GLclampf &y){return to_float(cx,obj,x,y);}
inline JSBool to_GLdouble (JSContext* cx, JSObject* obj, jsval x,GLdouble &y){return to_float(cx,obj,x,y);}
inline JSBool to_GLclampd (JSContext* cx, JSObject* obj, jsval x,GLclampd &y){return to_float(cx,obj,x,y);}

// we assume false==JS_FALSE
template <typename I>
inline JSBool to_intvec(JSContext* cx, JSObject* thisobj, jsval x,I* y, unsigned dim)
{
  if (!JSVAL_IS_OBJECT(x)) EJS_THROW_ERROR(cx,thisobj,"array object required");
  JSObject *obj=JSVAL_TO_OBJECT(x);
  jsuint l;
  if (!JS_GetArrayLength(cx, obj, &l)) return JS_FALSE;
  if (l!=dim) EJS_THROW_ERROR(cx,obj,"wrong array dimension");
  for (jsuint i=0;i<l;++i) {
    jsval elem;
    if (!JS_GetElement(cx, obj, i ,&elem)) return JS_FALSE;
    if (!to_int(cx,obj,elem,y[i])) return JS_FALSE;
  }
  return JS_TRUE;
}

template <typename I>
inline JSBool to_floatvec(JSContext* cx, JSObject* thisobj, jsval x,I* y, unsigned dim)
{
  if (!JSVAL_IS_OBJECT(x)) EJS_THROW_ERROR(cx,thisobj,"array object required");
  JSObject *obj=JSVAL_TO_OBJECT(x);
  jsuint l;
  if (!JS_GetArrayLength(cx, obj, &l)) return JS_FALSE;
  if (l!=dim) EJS_THROW_ERROR(cx,obj,"wrong array dimension");
  for (jsuint i=0;i<l;++i) {
    jsval elem;
    if (!JS_GetElement(cx, obj, i ,&elem)) return JS_FALSE;
    if (!to_float(cx,obj,elem,y[i])) return JS_FALSE;
  }
  return JS_TRUE;
}

inline JSBool to_GLushort_VEC (JSContext* cx, JSObject* obj, jsval x, GLushort* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}
inline JSBool to_GLshort_VEC (JSContext* cx, JSObject* obj, jsval x, GLshort* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}
inline JSBool to_GLbyte_VEC (JSContext* cx, JSObject* obj, jsval x, GLbyte* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}
inline JSBool to_GLubyte_VEC (JSContext* cx, JSObject* obj, jsval x, GLubyte* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}

inline JSBool to_GLuint_VEC (JSContext* cx, JSObject* obj, jsval x, GLuint* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}
inline JSBool to_GLint_VEC (JSContext* cx, JSObject* obj, jsval x, GLint* y, unsigned dim){return to_intvec(cx,obj,x,y,dim);}

inline JSBool to_GLfloat_VEC (JSContext* cx, JSObject* obj, jsval x, GLfloat* y, unsigned dim){return to_floatvec(cx,obj,x,y,dim);}
inline JSBool to_GLdouble_VEC (JSContext* cx, JSObject* obj, jsval x, GLdouble* y, unsigned dim){return to_floatvec(cx,obj,x,y,dim);}



inline JSBool from_GLboolean(JSContext*, JSObject*, GLboolean nres, jsval *rval)
{
  *rval=BOOLEAN_TO_JSVAL(nres==GL_TRUE);
  return JS_TRUE;
}

template <typename I>
inline JSBool from_int(JSContext* cx, JSObject*, I nres, jsval *rval)
{
  return JS_NewNumberValue(cx,nres,rval);
}

inline JSBool from_GLenum(JSContext* cx, JSObject* obj, GLenum nres, jsval *rval)
{
  return from_int(cx,obj,nres,rval);
}

inline JSBool from_GLint(JSContext* cx, JSObject* obj, GLint nres, jsval *rval)
{
  return from_int(cx,obj,nres,rval);
}

inline JSBool from_GLuint(JSContext* cx, JSObject* obj, GLuint nres, jsval *rval)
{
  return from_int(cx,obj,nres,rval);
}


inline JSBool from_const_GLubyte_ptr(JSContext* cx, JSObject*, const GLubyte* nres, jsval* rval)
{
  JSString *s=JS_NewStringCopyZ(cx, (const char *)nres);
  if (!s) return JS_FALSE;
  *rval=STRING_TO_JSVAL(s);
  return JS_TRUE;
}

inline JSBool from_boolean_vec(JSContext* cx, JSObject*, const GLboolean* v,int s, jsval* rval)
{
  JSObject *nobj=JS_NewArrayObject(cx, 0, NULL);
  if (!nobj) return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(nobj);
  for (int i=0;i<s;++i) {
    jsval n=BOOLEAN_TO_JSVAL(v[i]);
    if (!JS_SetElement(cx, nobj, i, &n))
      return JS_FALSE;
  }
  return JS_TRUE;
}

template <typename N>
inline JSBool from_number_vec(JSContext* cx, JSObject*, const N* v,int s, jsval* rval)
{
  JSObject *nobj=JS_NewArrayObject(cx, 0, NULL);
  if (!nobj) return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(nobj);
  for (int i=0;i<s;++i) {
    jsval n;
    if (!JS_NewNumberValue(cx, v[i], &n)) return JS_FALSE;
    if (!JS_SetElement(cx, nobj, i, &n)) return JS_FALSE;
  }
  return JS_TRUE;
}


extern "C" {
#include "jsgl_funcs.h"
  // some functions we do handcraft
  static JSBool Lightfv(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,3,argc);
    GLenum light;
    if (!to_GLenum (cx,obj,argv[0], light))
      return JS_FALSE;
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[1], pname))
      return JS_FALSE;
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
      EJS_THROW_ERROR(cx,obj,"argument 1 has wrong value");
    }
    GLfloat v[dim];
    if (!to_GLfloat_VEC (cx,obj,argv[2], v, dim))
      return JS_FALSE;
    glLightfv(light,pname,v);
    return JS_TRUE;
  }
  static JSBool Materialfv(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,3,argc);
    GLenum face;
    if (!to_GLenum (cx,obj,argv[0], face))
      return JS_FALSE;
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[1], pname))
      return JS_FALSE;
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
      EJS_THROW_ERROR(cx,obj,"argument 1 has wrong value");
    }
    GLfloat v[dim];
    if (!to_GLfloat_VEC (cx,obj,argv[2], v, dim))
      return JS_FALSE;
    glMaterialfv(face,pname,v);
    return JS_TRUE;
  }
  static JSBool GetBooleanv(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[0], pname))
      return JS_FALSE;
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      EJS_THROW_ERROR(cx,obj,"unknown parameter");
    GLboolean v[vecsize];
    glGetBooleanv(pname,v);
    if (!from_boolean_vec(cx,obj,v,vecsize,rval))
      return JS_FALSE;
    return JS_TRUE;
  }  
  static JSBool GetDoublev(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[0], pname))
      return JS_FALSE;
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      EJS_THROW_ERROR(cx,obj,"unknown parameter");
    GLdouble v[vecsize];
    glGetDoublev(pname,v);
    if (!from_number_vec(cx,obj,v,vecsize,rval))
      return JS_FALSE;
    return JS_TRUE;
  }
  static JSBool GetFloatv(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[0], pname))
      return JS_FALSE;
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      EJS_THROW_ERROR(cx,obj,"unknown parameter");
    GLfloat v[vecsize];
    glGetFloatv(pname,v);
    if (!from_number_vec(cx,obj,v,vecsize,rval))
      return JS_FALSE;
    return JS_TRUE;
  }
  static JSBool GetIntegerv(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GLenum pname;
    if (!to_GLenum (cx,obj,argv[0], pname))
      return JS_FALSE;
    int vecsize=jsgl_glGetNumArgs(pname);
    if (vecsize<=0)
      EJS_THROW_ERROR(cx,obj,"unknown parameter");
    GLint v[vecsize];
    glGetIntegerv(pname,v);
    if (!from_number_vec(cx,obj,v,vecsize,rval))
      return JS_FALSE;
    return JS_TRUE;
  }

#include "jsglu_funcs.h"

  static JSBool Project(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,6,argc);

    // input
    GLdouble objX;
    GLdouble objY;
    GLdouble objZ;
    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];
    
    // output
    GLdouble win[3];

    if (!to_GLdouble(cx,obj,argv[0], objX)) return JS_FALSE;
    if (!to_GLdouble(cx,obj,argv[1], objY)) return JS_FALSE;
    if (!to_GLdouble(cx,obj,argv[2], objZ)) return JS_FALSE;
    if (!to_GLdouble_VEC (cx,obj,argv[3], model, 16)) return JS_FALSE;
    if (!to_GLdouble_VEC (cx,obj,argv[4], proj, 16)) return JS_FALSE;
    if (!to_GLint_VEC (cx,obj,argv[5], view, 4)) return JS_FALSE;

    if (!gluProject(objX, objY, objZ, model, proj, view, &win[0], &win[1], &win[2]))
      return from_GLboolean(cx,obj,GL_FALSE, rval) ? JS_TRUE : JS_FALSE;
    return from_number_vec(cx,obj,win, 3, rval) ? JS_TRUE : JS_FALSE;
  }

  static JSBool UnProject(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,6,argc);

    // input
    GLdouble winX;
    GLdouble winY;
    GLdouble winZ;
    GLdouble model[16];
    GLdouble proj[16];
    GLint view[4];
    
    // output
    GLdouble v[3];

    if (!to_GLdouble(cx,obj,argv[0], winX)) return JS_FALSE;
    if (!to_GLdouble(cx,obj,argv[1], winY)) return JS_FALSE;
    if (!to_GLdouble(cx,obj,argv[2], winZ)) return JS_FALSE;
    if (!to_GLdouble_VEC (cx,obj,argv[3], model, 16)) return JS_FALSE;
    if (!to_GLdouble_VEC (cx,obj,argv[4], proj, 16))  return JS_FALSE;
    if (!to_GLint_VEC (cx,obj,argv[5], view, 4))      return JS_FALSE;

    if (!gluUnProject(winX, winY, winZ, model, proj, view, &v[0], &v[1], &v[2]))
      return from_GLboolean(cx,obj,GL_FALSE, rval) ? JS_TRUE : JS_FALSE;
    return from_number_vec(cx,obj,v, 3, rval) ? JS_TRUE : JS_FALSE;
  }

  static JSBool Map1f(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,4,argc);
    GLenum target;
    GLfloat u1,u2;
    GLint stride,order;
    if (!to_GLenum(cx,obj,argv[0],target)) return JS_FALSE;
    if (!to_GLfloat(cx,obj,argv[1],u1)) return JS_FALSE;
    if (!to_GLfloat(cx,obj,argv[2],u2)) return JS_FALSE;
    if (!JSVAL_IS_OBJECT(argv[3])) EJS_THROW_ERROR(cx,obj,"array object required");
    JSObject *aobj=JSVAL_TO_OBJECT(argv[3]);
    jsuint l;
    if (!JS_GetArrayLength(cx, aobj, &l)) return JS_FALSE;
    switch(target){
    case GL_MAP1_INDEX:
    case GL_MAP1_TEXTURE_COORD_1:
      stride=1;
      break;
    case GL_MAP1_TEXTURE_COORD_2:
      stride=2;
      break;
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_3:
      stride=3;
      break;
    case GL_MAP1_VERTEX_4:
    case GL_MAP1_COLOR_4:
    case GL_MAP1_TEXTURE_COORD_4:
      stride=4;
      break;
    default:
      EJS_THROW_ERROR(cx,obj,"unknown target");
    }
    if (l%stride) EJS_THROW_ERROR(cx,obj,"array incomplete");
    order=l/stride;
    if (!order) EJS_THROW_ERROR(cx,obj,"order must be positive");
    GLfloat points[l];
    if (!to_floatvec(cx,obj,argv[3],points,l)) return JS_FALSE;
    glMap1f(target,u1,u2,stride,order,points);
    return JS_TRUE;
  }
  

#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec gl_static_methods[] = {
#include "jsgl_fspecs.h"
    FUNC (Lightfv, 3),
    FUNC (Materialfv, 3),
    FUNC (GetBooleanv, 1),
    FUNC (GetDoublev, 1),
    FUNC (GetFloatv, 1),
    FUNC (GetIntegerv, 1),
    FUNC (Map1f, 1),
    EJS_END_FUNCTIONSPEC
  };

  static JSFunctionSpec glu_static_methods[] = {
#include "jsglu_fspecs.h"
    FUNC (Project, 6),
    FUNC (UnProject, 6),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsgl_LTX_onLoad(JSContext* cx, JSObject* global)
  {
    JSObject *globj = JS_DefineObject(cx, global, "gl", NULL, NULL,
				      JSPROP_ENUMERATE);
    if (!globj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, globj, gl_static_methods)) return JS_FALSE;
    
    JSObject *gluobj = JS_DefineObject(cx, global, "glu", NULL, NULL,
				       JSPROP_ENUMERATE);
    if (!gluobj) return JS_FALSE;
    return JS_DefineFunctions(cx, gluobj, glu_static_methods);
  }
}
