/*
for a in $(grep ecma_to jsgl_funcs.h|cut -f 2 -d \!|cut -f 1 -d " "|sort|uniq); do echo "inline bool $a (jsval x,${a#ecma_to_*} y){return true;}"; done|sed 's/,GL\([a-z]*\)_VEC y/, GL\1\* y, unsigned dim/'
*/


// TODO: this does not work with ints bigger than 32bit
template <typename I>
inline bool ecma_to_int(jsval x,I &y)
{
  int32 i;
  bool r=JS_ValueToInt32(ECMAScript::cx,x,&i)==JS_TRUE;
  y=i; // TODO: check for overflow?
  return r;
}

template <typename F>
inline bool ecma_to_float(jsval x,F &y)
{
  jsdouble d;
  bool r=JS_ValueToNumber(ECMAScript::cx,x,&d)==JS_TRUE;
  y=d; // TODO: check for overflow? - check for NaN?
  return r;
}

inline bool ecma_to_GLbyte (jsval x,GLbyte &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLboolean (jsval x,GLboolean &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLubyte (jsval x,GLubyte &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLushort (jsval x,GLushort &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLshort (jsval x,GLshort &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLint (jsval x,GLint &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLsizei (jsval x,GLsizei &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLuint (jsval x,GLuint &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLenum (jsval x,GLenum &y){return ecma_to_int(x,y);}
inline bool ecma_to_GLbitfield (jsval x,GLbitfield &y){return ecma_to_int(x,y);}

inline bool ecma_to_GLfloat (jsval x,GLfloat &y){return ecma_to_float(x,y);}
inline bool ecma_to_GLclampf (jsval x,GLclampf &y){return ecma_to_float(x,y);}
inline bool ecma_to_GLdouble (jsval x,GLdouble &y){return ecma_to_float(x,y);}
inline bool ecma_to_GLclampd (jsval x,GLclampd &y){return ecma_to_float(x,y);}

// we assume false==JS_FALSE
template <typename I>
inline bool ecma_to_intvec(jsval x,I* y, unsigned dim)
{
  JSContext* cx=ECMAScript::cx;
  if (!JSVAL_IS_OBJECT(x)) ECMA_THROW_ERROR("array object required");
  JSObject *obj=JSVAL_TO_OBJECT(x);
  jsuint l;
  if (!JS_GetArrayLength(cx, obj, &l)) return false;
  if (l!=dim) ECMA_THROW_ERROR("wrong array dimension");
  for (jsuint i=0;i<l;++i) {
    jsval elem;
    if (!JS_GetElement(cx, obj, i ,&elem)) return false;
    if (!ecma_to_int(elem,y[i])) return false;
  }
  return true;
}

template <typename I>
inline bool ecma_to_floatvec(jsval x,I* y, unsigned dim)
{
  JSContext* cx=ECMAScript::cx;
  if (!JSVAL_IS_OBJECT(x)) ECMA_THROW_ERROR("array object required");
  JSObject *obj=JSVAL_TO_OBJECT(x);
  jsuint l;
  if (!JS_GetArrayLength(cx, obj, &l)) return false;
  if (l!=dim) ECMA_THROW_ERROR("wrong array dimension");
  for (jsuint i=0;i<l;++i) {
    jsval elem;
    if (!JS_GetElement(cx, obj, i ,&elem)) return false;
    if (!ecma_to_float(elem,y[i])) return false;
  }
  return true;
}

inline bool ecma_to_GLushort_VEC (jsval x, GLushort* y, unsigned dim){return ecma_to_intvec(x,y,dim);}
inline bool ecma_to_GLshort_VEC (jsval x, GLshort* y, unsigned dim){return ecma_to_intvec(x,y,dim);}
inline bool ecma_to_GLbyte_VEC (jsval x, GLbyte* y, unsigned dim){return ecma_to_intvec(x,y,dim);}
inline bool ecma_to_GLubyte_VEC (jsval x, GLubyte* y, unsigned dim){return ecma_to_intvec(x,y,dim);}

inline bool ecma_to_GLuint_VEC (jsval x, GLuint* y, unsigned dim){return ecma_to_intvec(x,y,dim);}
inline bool ecma_to_GLint_VEC (jsval x, GLint* y, unsigned dim){return ecma_to_intvec(x,y,dim);}

inline bool ecma_to_GLfloat_VEC (jsval x, GLfloat* y, unsigned dim){return ecma_to_floatvec(x,y,dim);}
inline bool ecma_to_GLdouble_VEC (jsval x, GLdouble* y, unsigned dim){return ecma_to_floatvec(x,y,dim);}



inline bool ecma_from_GLboolean(GLboolean nres, jsval *rval)
{
  *rval=BOOLEAN_TO_JSVAL(nres==GL_TRUE);
  return true;
}

template <typename I>
inline bool ecma_from_int(I nres, jsval *rval)
{
  return JS_NewNumberValue(ECMAScript::cx,nres,rval)==JS_TRUE;
}

inline bool ecma_from_GLenum(GLenum nres, jsval *rval)
{
  return ecma_from_int(nres,rval);
}

inline bool ecma_from_GLint(GLint nres, jsval *rval)
{
  return ecma_from_int(nres,rval);
}

inline bool ecma_from_GLuint(GLuint nres, jsval *rval)
{
  return ecma_from_int(nres,rval);
}


inline bool ecma_from_const_GLubyte_ptr(const GLubyte* nres, jsval* rval)
{
  JSString *s=JS_NewStringCopyZ(ECMAScript::cx, (const char *)nres);
  if (!s) return false;
  *rval=STRING_TO_JSVAL(s);
  return true;
}

inline bool ecma_from_boolean_vec(const GLboolean* v,int s, jsval* rval)
{
  JSObject *nobj=JS_NewArrayObject(ECMAScript::cx, 0, NULL);
  if (!nobj) return false;
  *rval=OBJECT_TO_JSVAL(nobj);
  for (int i=0;i<s;++i) {
    jsval n=BOOLEAN_TO_JSVAL(v[i]);
    if (!JS_SetElement(ECMAScript::cx, nobj, i, &n))
      return false;
  }
  return true;
}

template <typename N>
inline bool ecma_from_number_vec(const N* v,int s, jsval* rval)
{
  JSObject *nobj=JS_NewArrayObject(ECMAScript::cx, 0, NULL);
  if (!nobj) return false;
  *rval=OBJECT_TO_JSVAL(nobj);
  for (int i=0;i<s;++i) {
    jsval n;
    if (!JS_NewNumberValue(ECMAScript::cx, v[i], &n)) return false;
    if (!JS_SetElement(ECMAScript::cx, nobj, i, &n)) return false;
  }
  return true;
}
