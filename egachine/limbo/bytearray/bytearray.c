#include <string.h>
#include "jsapi.h"
#include "jscntxt.h"

#include "bytearray.h"

/*
 * A JavaScript byte array primitive.
 *
 * SSR
 *
 */

// Default capacity of a byte array is 8 bytes
#define DEFAULT_CAPACITY ((jsint) 8 )

#define BYTE_ARRAY_FLAGS_OWNS_DATA 0x1
#define BYTE_ARRAY_FLAGS_RESIZABLE 0x2

/*
 * Tagged array internal data structure. The private array
 * and data are maintained by ByteArray instances.
 */
typedef struct {
  unsigned char *data;
  jsuint capacity;
  jsuint len;
  jsuint flags;
} byte_array_private_data;


/*
 * Early bound property id's.
 */
typedef enum {
  byte_array_prop_len_id = -1,
  byte_array_prop_capacity_id = -2,
} js_byte_array_prop_ids;


// ByteArray class name
static char js_byte_array_str[] = "ByteArray";
// length property name
static char js_byte_array_prop_len_str[] = "length";
// capacity property name
static char js_byte_array_prop_capacity_str[] = "capacity";
// append method name
static char js_byte_array_method_append_str[] = "append";
// toString method name
static char js_byte_array_method_toString_str[] = "toString";
// copy static method name
static char js_byte_array_static_method_copy_str[] = "copy";


/*
 * ByteArray - native constructor
 * Expects arguments: ByteArray()
 *                    ByteArray(ByteArray)
 *                    ByteArray(int capacity)
 *                    ByteArray(ByteArray array, int capacity)
 */
static JSBool ByteArray(JSContext *cx, JSObject *obj, 
			uintN argc, jsval *argv, jsval *rval);


/*
 * byte_array_copy
 *
 * Really ByteArray.copy(srcArray, srcPos, dstArray, dstPos, len)
 * Ripped straight from System.arrayCopy in Java.
 * A class-wide static method.
 */
static JSBool byte_array_copy(JSContext *cx, JSObject *obj, uintN argc,
			      jsval *argv, jsval *rval);

/*
 * byte_array_append
 *
 * Appends one array passed as an argument to the end of another.
 */
static JSBool byte_array_append(JSContext *cx, JSObject *obj, uintN argc,
				jsval *argv, jsval *rval);

/*
 * byte_array_toString
 *
 * converts a byte array to a string
 */
static JSBool byte_array_toString(JSContext *cx, JSObject *obj, uintN argc,
				  jsval *argv, jsval *rval);

/*
 * byte_array_setProperty
 *
 * HOOK function called by the scripting engine when setting a property
 */
static JSBool
byte_array_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
     

/*
 * byte_array_getProperty
 *
 * HOOK function called by the scripting engine when getting a property
 */
static JSBool
byte_array_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
     

/*
 * byte_array_resolve
 *
 * Property resolution function. See detailed description in function
 * definiton.
 */
static JSBool byte_array_resolve(JSContext *cx, JSObject *obj, jsval id,
				 uintN flags, JSObject **objp);


JSBool byte_array_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp);

/*
 * byte_array_finalize
 *
 * Finalizer called by SpiderMonkey when a byte array instance
 * is garbage collected. The internal buffer is only freed if this
 * ByteArray instance owned the data.
 */
JS_PUBLIC_API(void)
byte_array_finalize(JSContext *cx, JSObject *obj);

/*
 * ByteArray JavaScript class
 */
JSClass byte_array_class = {
  /*
   * The user-visible name of the ByteArray class.
   */
  js_byte_array_str,
  /* 
   * JSCLASS_HAS_PRIVATE indicates the first slot, is occupied by a void*. In
   * the byte array's case it is a char*.
   *
   * JSCLASS_NEW_RESOLVE indicates that properties not currently defined
   * will be lazily resolved using the byte_array_resolve function.
   */
  JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
  JS_PropertyStub,  JS_PropertyStub, 
  byte_array_getProperty, byte_array_setProperty,
  JS_EnumerateStub, (JSResolveOp) byte_array_resolve,
  byte_array_convert, byte_array_finalize, 
  NULL, NULL, NULL, NULL, NULL, NULL, NULL
};


/*
 * Instance methods of the ByteArray class
 */
struct JSFunctionSpec byte_array_methods[] = {
  {js_byte_array_method_append_str, byte_array_append, 0},
  {js_byte_array_method_toString_str, byte_array_toString, 0},
  {0}
};


/*
 * Static class-wide methods of the ByteArray class
 */
struct JSFunctionSpec byte_array_static_methods[] = {
  {js_byte_array_static_method_copy_str, byte_array_copy, 0},
  {0}
};


/*
 * ByteArray named properties.
 */
struct JSPropertySpec byte_array_props[] = {
  // length property
  { js_byte_array_prop_len_str,
    byte_array_prop_len_id,
    JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED,
    0, 0 },
  // capacity property
  { js_byte_array_prop_capacity_str,
    byte_array_prop_capacity_id,
    JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED,
    0, 0 },
  {0}
};


/*
 * js_InitByteArrayClass
 *
 * Initialize the byte array class, and make it available globally
 */
JSObject* js_InitByteArrayClass(JSContext *cx, JSObject *parentObj)
{
  JSObject *obj = JS_InitClass(cx, parentObj, NULL, &byte_array_class,
			       ByteArray, 2, byte_array_props, byte_array_methods, NULL,
			       byte_array_static_methods);
  return obj;
}


/*
 * js_NewByteArray
 *
 * Creates and returns a new byte array instance of 
 * capacity len. The data passed to this function will
 * be owned by the newly constructed ByteArray and finalized
 * when its destructors kick in.
 */
JSObject* js_NewByteArray(JSContext *cx,
			  unsigned char *data,
			  jsuint len, 
			  JSBool copyData)
{
  JSObject *bar = NULL;
  byte_array_private_data *private = NULL;

  bar = JS_NewObject(cx,
		     &byte_array_class,
		     NULL, 
		     NULL);

  if (!bar) return JS_FALSE;

  private = JS_malloc(cx, sizeof(byte_array_private_data));
  if (!private) return JS_FALSE;
  private->flags = 0;

  if (copyData) {
    private->data = JS_malloc(cx, len);
    if (!private->data) return JS_FALSE;
    memcpy(private->data, data, len);
  } else {
    private->data = data;
  }

  if (!data)
    private->capacity = private->len = 0;
  else
    private->capacity = private->len = len;

  if (copyData) {
    private->flags = BYTE_ARRAY_FLAGS_RESIZABLE | BYTE_ARRAY_FLAGS_OWNS_DATA;
  }

  JS_SetPrivate(cx, bar, (void*)private);

  return bar;
}


JSBool js_GetByteArrayLength(JSContext *cx,
			     JSObject *obj,
			     jsint *len)
{
  byte_array_private_data *private;

#ifdef JS_THREADSAFE
  if (JS_GetClass(cx, obj) == &byte_array_class
#else
  if (JS_GetClass(obj) == &byte_array_class
#endif
      && (private = JS_GetPrivate(cx, obj))) {
    *len = private->len;
    return JS_TRUE;
  }

  *len = 0;
  return JS_FALSE;
}


JSBool js_GetByteArrayBytes(JSContext *cx,
			    JSObject *obj,
			    unsigned char** data)
{
  byte_array_private_data *private;

#ifdef JS_THREADSAFE
  if (JS_GetClass(cx, obj) == &byte_array_class
#else
  if (JS_GetClass(obj) == &byte_array_class
#endif
      && (private = JS_GetPrivate(cx, obj))) {
    *data = private->data;
    return JS_TRUE;
  }

  *data = NULL;
  return JS_FALSE;
}


/*
 * ByteArray - Constructor
 */
JSBool ByteArray(JSContext *cx, JSObject *obj, 
		 uintN argc, jsval *argv, jsval *rval)
{
  JSObject *argArray=NULL;
  JSClass *class=NULL;
  byte_array_private_data *private=NULL, *argPrivate=NULL;

  private = JS_malloc(cx, sizeof(byte_array_private_data));
  if (!private) return JS_FALSE;

  if (!(cx->fp->flags & JSFRAME_CONSTRUCTING)) {
    obj = js_NewObject(cx, &byte_array_class, NULL, NULL);
    if (!obj)
      return JS_FALSE;
    *rval = OBJECT_TO_JSVAL(obj);
  }

  if (argc==0) {
    private->capacity = DEFAULT_CAPACITY;
  } else {
    if (JSVAL_IS_OBJECT(argv[0])) {
      argArray = JSVAL_TO_OBJECT(argv[0]);
#ifdef JS_THREADSAFE
      class=JS_GetClass(cx, argArray);
#else
      class=JS_GetClass(argArray);
#endif

      if (class == &byte_array_class) {
	argPrivate = JS_GetPrivate(cx, argArray);
	if (!argPrivate) {
	  return JS_FALSE;
	}
      }
    }

    if (!JS_ValueToInt32(cx, argv[ ((argc==1)?0:1) ], &(private->capacity))
	  && (private->capacity > 0))
      private->capacity = DEFAULT_CAPACITY;

    if (argPrivate && (private->capacity < argPrivate->capacity))
      private->capacity = argPrivate->capacity;
  }

  private->data = JS_malloc(cx, private->capacity);

  if (! private->data) {
    return JS_FALSE;
  }
	
  if (argPrivate && argPrivate->data) {
    memcpy(private->data, argPrivate->data, argPrivate->len);
  }
  
  JS_SetPrivate(cx, obj, private);

  if (argPrivate && argPrivate->len)
    private->len = argPrivate->len;
  else
    private->len = 0;

  private->flags = BYTE_ARRAY_FLAGS_RESIZABLE | BYTE_ARRAY_FLAGS_OWNS_DATA;
  JS_SetPrivate(cx, obj, private);

  return JS_TRUE;
}


JSBool byte_array_copy(JSContext *cx, JSObject *obj, uintN argc,
		       jsval *argv, jsval *rval)
{
  JSObject *srcarr = NULL, *dstarr=NULL;
  byte_array_private_data *srcprv=NULL, *dstprv=NULL;
  jsint srcpos=0, dstpos=0, len=0;

  *rval = BOOLEAN_TO_JSVAL(JS_FALSE);

  if (argc != 5) return JS_TRUE;
  
  if (! (JSVAL_IS_OBJECT(argv[0]) && JSVAL_IS_OBJECT(argv[2])) )
    return JS_TRUE;

  if (! (JSVAL_IS_INT(argv[1]) 
	 && JSVAL_IS_INT(argv[3]) 
	 && JSVAL_IS_INT(argv[4])))
    return JS_TRUE;
  
  srcarr=JSVAL_TO_OBJECT(argv[0]);
  dstarr=JSVAL_TO_OBJECT(argv[2]);

#ifdef JS_THREADSAFE
  if ( JS_GetClass(cx, srcarr) != &byte_array_class
#else
  if ( JS_GetClass(srcarr) != &byte_array_class
#endif
       ||
#ifdef JS_THREADSAFE
       JS_GetClass(cx, dstarr) != &byte_array_class )
#else
       JS_GetClass(dstarr) != &byte_array_class )
#endif
    return JS_TRUE;

  srcprv = (byte_array_private_data*) JS_GetPrivate(cx, srcarr);
  dstprv = (byte_array_private_data*) JS_GetPrivate(cx, dstarr);

  if (!srcprv && !dstprv) return JS_TRUE;

  srcpos=JSVAL_TO_INT(argv[1]);
  dstpos=JSVAL_TO_INT(argv[3]);
  len=JSVAL_TO_INT(argv[4]);

  if ( len > 0 
       && srcpos > -1 
       && dstpos > -1
       && srcpos < srcprv->len
       && (srcpos+len-1) < srcprv->len 
       && (dstpos+len-1) < dstprv->capacity ) {
    memcpy( dstprv->data + dstpos,
	    srcprv->data + srcpos,
	    len );
    if ((dstpos+len-1) >= dstprv->len)
      dstprv->len = dstpos+len;
    *rval = BOOLEAN_TO_JSVAL(JS_TRUE);
  }

  return JS_TRUE;
}


JSBool byte_array_append(JSContext *cx, JSObject *obj, uintN argc,
				jsval *argv, jsval *rval)
{
  JSObject *rhsArray = NULL;
  byte_array_private_data *private = NULL, *rhsPrivate = NULL;

  if (argc < 1) return JS_TRUE;
  
  // If the RHS wasn't an object, should report an error here.
  if (JSVAL_IS_OBJECT(argv[0])) {
    rhsArray = JSVAL_TO_OBJECT(argv[0]);

#ifdef JS_THREADSAFE
    if (JS_GetClass(cx, rhsArray) == &byte_array_class) {
#else
    if (JS_GetClass(rhsArray) == &byte_array_class) {
#endif
      rhsPrivate = (byte_array_private_data*) JS_GetPrivate(cx, rhsArray);
      private = JS_GetPrivate(cx, obj);

      // Should report an error here
      if (!private) return JS_TRUE;
      if (!rhsPrivate) return JS_TRUE;
      
      if ((rhsPrivate->len + private->len)
	  > private->capacity) {
	char *reallocd  = JS_realloc(cx, private->data,
				     rhsPrivate->capacity + private->capacity);
	if (reallocd) {
	  private->data = reallocd;
	  
	  memcpy( (private->data + private->len),
		  rhsPrivate->data,
		  rhsPrivate->len );

	  private->len += rhsPrivate->len;
	  private->capacity += rhsPrivate->capacity;
	} else {
	  private->data = NULL;
	  private->len = 0;
	  private->capacity = 0;
	  return JS_FALSE;
	}
      }
    }
  }

  return JS_TRUE;
}


JSBool byte_array_toString(JSContext *cx, JSObject *obj, uintN argc,
				  jsval *argv, jsval *rval)
{
  byte_array_private_data *private = NULL;
  *rval = JSVAL_VOID;

  private = JS_GetPrivate(cx, obj);
  if (!private) return JS_TRUE;

  *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, private->data, private->len));

  return JS_TRUE;
}


JSBool
byte_array_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  jsint index = 0;
  byte_array_private_data *private = NULL;

  /*
   * Only resolve indexed properties of the form bar[0], bar[1], etc.
   */
  if (!JSVAL_IS_INT(id)) {
    return JS_TRUE;
  }
  index = JSVAL_TO_INT(id);
  if (index < -2)
    return JS_TRUE;

  private = (byte_array_private_data*) JS_GetPrivate(cx, obj);
  if (!private) return JS_FALSE;

  switch (index) {
  case byte_array_prop_len_id:
    *vp = INT_TO_JSVAL(private->len);
    return JS_TRUE;
  case byte_array_prop_capacity_id:
    *vp = INT_TO_JSVAL(private->capacity);
    return JS_TRUE;
  default:
    break;
  }

  if (index < private->len && index < private->capacity)
    *vp = INT_TO_JSVAL(private->data[index]);

  return JS_TRUE;
}


JSBool
byte_array_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  jsint index = 0;
  unsigned char byte=0;
  byte_array_private_data *private=NULL;

  /*
   * Only accept integer arguments.
   */ 
  if (!JSVAL_IS_INT(*vp)) return JS_TRUE;
  byte = (unsigned char) JSVAL_TO_INT(*vp);

  /*
   * Only set indexed properties of the form bar[0], bar[1], etc.
   */
  if (!JSVAL_IS_INT(id)) {
    return JS_TRUE;
  }
  index = JSVAL_TO_INT(id);
  if (index < 0)
    return JS_TRUE;

  private = (byte_array_private_data*) JS_GetPrivate(cx, obj);
  if (!private) return JS_FALSE;
    
  if (index < private->len && index < private->capacity)
    private->data[index] = byte;

  return JS_TRUE;
}


/**
 * byte_array_resolve
 *
 * Used by SpiderMonkey to lazily evaluate properties defined within ByteArray
 * instances. The purpose of this function is: to allow extension
 * of the array as more data is added to it, without saying that the array
 * must have a fixed length. The other approach would be to pre-define a number
 * of properties up to a fixed length and define them as shared, which is
 * crappy. The way the normal "Array" class works is that its storing all
 * member data in slots, so there's no need to worry about shared properties
 * and private data. Sadly, each slot is a 32-bit entity, which makes it
 * inefficient for storing large quanities of data.
 */
JSBool byte_array_resolve(JSContext *cx, JSObject *obj, jsval id,
			  uintN flags, JSObject **objp)
{
  byte_array_private_data *private=NULL;
  jsint index = 0;

  *objp = NULL;

  /*
   * Only resolve indexed properties of the form bar[0], bar[1], etc.
   */
  if (!JSVAL_IS_INT(id)) {
    return JS_TRUE;
  }
  index = JSVAL_TO_INT(id);
  if (index < 0)
    return JS_TRUE;

  private = (byte_array_private_data*) JS_GetPrivate(cx, obj);
  if (!private) return JS_FALSE;

  /*
   * We've attempted to resolve the property as an lval.
   * bar[0] = 8;
   */
  if (flags & JSRESOLVE_ASSIGNING) {
    /*
     * Don't support sparse arrays.
     */
    if (index > private->len) 
      return JS_TRUE;

    // Make sure the array is resizable
    if ( !(private->flags & BYTE_ARRAY_FLAGS_RESIZABLE))
      return JS_TRUE;

    /*
     * Not enough capacity, so expand the underlying array
     */
    if (! private->data) {
      private->capacity = DEFAULT_CAPACITY;
      private->data = JS_malloc(cx, private->capacity);
    } else if (index == private->len && private->len == private->capacity) {
      private->capacity+=32;
      private->data = JS_realloc(cx, private->data, private->capacity);
    }
    
    // Define the indexed property
    JS_DefineProperty(cx, obj, (char *) index, INT_TO_JSVAL(index), NULL, NULL,
		      JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_INDEX);

    // Increase the used length
    private->len++;
    *objp = obj;

  } else if (flags & JSRESOLVE_QUALIFIED) {
    /*
     * The byte array is indexed as an rval, foo = bar[0],
     * it has been initialized with some data, possibly, by its
     * constructor, the properties still need to be lazily evaluated.
     */

    if (index >= private->len)
      return JS_TRUE;

    JS_DefineProperty(cx, obj, (char *) index, INT_TO_JSVAL(index), NULL, NULL,
		      JSPROP_PERMANENT | JSPROP_SHARED | JSPROP_INDEX);
    *objp = obj;
  }

  return JS_TRUE;
}

JSBool byte_array_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
  byte_array_private_data *private = NULL;
  private = (byte_array_private_data*) JS_GetPrivate(cx, obj);
  *vp=JSVAL_VOID;
  if (!private) return JS_FALSE;

  switch(type) {
  case JSTYPE_NUMBER:
    *vp = INT_TO_JSVAL(private->len);
    return JS_TRUE;
  case JSTYPE_BOOLEAN:
    *vp = BOOLEAN_TO_JSVAL((private->len > 0));
    return JS_TRUE;
  case JSTYPE_OBJECT:
    *vp = OBJECT_TO_JSVAL(obj);
    return JS_TRUE;
  case JSTYPE_STRING:
  default:
    *vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, private->data, private->len));
    return JS_TRUE;
  }
}


/*
 * byte_array_finalize
 *
 * Finalizer called by SpiderMonkey when a byte array instance
 * is garbage collected. The internal buffer is only freed if this
 * ByteArray instance owned the data.
 */
JS_PUBLIC_API(void) byte_array_finalize(JSContext *cx, JSObject *obj)
{
  byte_array_private_data *private = NULL;
  fprintf(stderr, "obj::%p\n", obj);fflush(stderr);
  private = (byte_array_private_data*) JS_GetPrivate(cx, obj);
  
  if (private) {
    if ((private->flags & BYTE_ARRAY_FLAGS_OWNS_DATA) && private->data)
      ;
      //      JS_free(cx, private->data);
      
      //    JS_free(cx, private);
  }
}
