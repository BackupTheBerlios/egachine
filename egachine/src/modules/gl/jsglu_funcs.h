/* DO NOT EDIT THIS FILE - it is generated by jsgl_gen */
/* TODO: gluBeginCurve */

/* TODO: gluBeginPolygon */

/* TODO: gluBeginSurface */

/* TODO: gluBeginTrim */

/* TODO: gluBuild1DMipmapLevels */

/* TODO: gluBuild1DMipmaps */

/* TODO: gluBuild2DMipmapLevels */

/* TODO: gluBuild2DMipmaps */

/* TODO: gluBuild3DMipmapLevels */

/* TODO: gluBuild3DMipmaps */

/* TODO: gluCheckExtension */

/* TODO: gluCylinder */

/* TODO: gluDeleteNurbsRenderer */

/* TODO: gluDeleteQuadric */

/* TODO: gluDeleteTess */

/* TODO: gluDisk */

/* TODO: gluEndCurve */

/* TODO: gluEndPolygon */

/* TODO: gluEndSurface */

/* TODO: gluEndTrim */

static JSBool
ejs_gluErrorString (JSContext * cx, JSObject * obj, uintN argc, jsval * argv,
		    jsval * rval)
{
  EJS_CHECK_NUM_ARGS (cx, obj, 1, argc);
  GLenum error;
  if (!to_GLenum (cx, obj, argv[0], error))
    return JS_FALSE;
  const GLubyte *nres = gluErrorString (error);
  if (!from_const_GLubyte_ptr (cx, obj, nres, rval))
    return JS_FALSE;
  return JS_TRUE;
}

/* TODO: gluGetNurbsProperty */

static JSBool
ejs_gluGetString (JSContext * cx, JSObject * obj, uintN argc, jsval * argv,
		  jsval * rval)
{
  EJS_CHECK_NUM_ARGS (cx, obj, 1, argc);
  GLenum name;
  if (!to_GLenum (cx, obj, argv[0], name))
    return JS_FALSE;
  const GLubyte *nres = gluGetString (name);
  if (!from_const_GLubyte_ptr (cx, obj, nres, rval))
    return JS_FALSE;
  return JS_TRUE;
}

/* TODO: gluGetTessProperty */

/* TODO: gluLoadSamplingMatrices */

static JSBool
ejs_gluLookAt (JSContext * cx, JSObject * obj, uintN argc, jsval * argv,
	       jsval *)
{
  EJS_CHECK_NUM_ARGS (cx, obj, 9, argc);
  GLdouble eyeX;
  if (!to_GLdouble (cx, obj, argv[0], eyeX))
    return JS_FALSE;
  GLdouble eyeY;
  if (!to_GLdouble (cx, obj, argv[1], eyeY))
    return JS_FALSE;
  GLdouble eyeZ;
  if (!to_GLdouble (cx, obj, argv[2], eyeZ))
    return JS_FALSE;
  GLdouble centerX;
  if (!to_GLdouble (cx, obj, argv[3], centerX))
    return JS_FALSE;
  GLdouble centerY;
  if (!to_GLdouble (cx, obj, argv[4], centerY))
    return JS_FALSE;
  GLdouble centerZ;
  if (!to_GLdouble (cx, obj, argv[5], centerZ))
    return JS_FALSE;
  GLdouble upX;
  if (!to_GLdouble (cx, obj, argv[6], upX))
    return JS_FALSE;
  GLdouble upY;
  if (!to_GLdouble (cx, obj, argv[7], upY))
    return JS_FALSE;
  GLdouble upZ;
  if (!to_GLdouble (cx, obj, argv[8], upZ))
    return JS_FALSE;
  gluLookAt (eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
  return JS_TRUE;
}

/* TODO: gluNextContour */

/* TODO: gluNurbsCallback */

/* TODO: gluNurbsCallbackData */

/* TODO: gluNurbsCallbackDataEXT */

/* TODO: gluNurbsCurve */

/* TODO: gluNurbsProperty */

/* TODO: gluNurbsSurface */

static JSBool
ejs_gluOrtho2D (JSContext * cx, JSObject * obj, uintN argc, jsval * argv,
		jsval *)
{
  EJS_CHECK_NUM_ARGS (cx, obj, 4, argc);
  GLdouble left;
  if (!to_GLdouble (cx, obj, argv[0], left))
    return JS_FALSE;
  GLdouble right;
  if (!to_GLdouble (cx, obj, argv[1], right))
    return JS_FALSE;
  GLdouble bottom;
  if (!to_GLdouble (cx, obj, argv[2], bottom))
    return JS_FALSE;
  GLdouble top;
  if (!to_GLdouble (cx, obj, argv[3], top))
    return JS_FALSE;
  gluOrtho2D (left, right, bottom, top);
  return JS_TRUE;
}

/* TODO: gluPartialDisk */

static JSBool
ejs_gluPerspective (JSContext * cx, JSObject * obj, uintN argc, jsval * argv,
		    jsval *)
{
  EJS_CHECK_NUM_ARGS (cx, obj, 4, argc);
  GLdouble fovy;
  if (!to_GLdouble (cx, obj, argv[0], fovy))
    return JS_FALSE;
  GLdouble aspect;
  if (!to_GLdouble (cx, obj, argv[1], aspect))
    return JS_FALSE;
  GLdouble zNear;
  if (!to_GLdouble (cx, obj, argv[2], zNear))
    return JS_FALSE;
  GLdouble zFar;
  if (!to_GLdouble (cx, obj, argv[3], zFar))
    return JS_FALSE;
  gluPerspective (fovy, aspect, zNear, zFar);
  return JS_TRUE;
}

/* TODO: gluPickMatrix */

/* TODO: gluProject */

/* TODO: gluPwlCurve */

/* TODO: gluQuadricCallback */

/* TODO: gluQuadricDrawStyle */

/* TODO: gluQuadricNormals */

/* TODO: gluQuadricOrientation */

/* TODO: gluQuadricTexture */

/* TODO: gluScaleImage */

/* TODO: gluSphere */

/* TODO: gluTessBeginContour */

/* TODO: gluTessBeginPolygon */

/* TODO: gluTessCallback */

/* TODO: gluTessEndContour */

/* TODO: gluTessEndPolygon */

/* TODO: gluTessNormal */

/* TODO: gluTessProperty */

/* TODO: gluTessVertex */

/* TODO: gluUnProject */

/* TODO: gluUnProject4 */
