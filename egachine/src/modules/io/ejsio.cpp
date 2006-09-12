#include "ejsio.h"
#include <unistd.h>
#include <iostream>
#include <stdint.h>

static JSObject *ejsByteSourceProto = NULL;

//! byte source
struct ejsByteSource
{
  void* object;
  ejsGetFunc get;
  ejsAvailableFunc available;
  ejsDeleteFunc deleteObject;
};

static
void
ejsByteSource_finalize(JSContext *cx, JSObject *obj);

static
JSClass ejsByteSource_class = {
  "ByteSource",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  ejsByteSource_finalize,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

#define GET_BYTESOURCE_OBJ ejsByteSource* bs=NULL;	\
  if (JS_GET_CLASS(cx, obj) != &ejsByteSource_class)	\
    EJS_THROW_ERROR(cx,obj,"incompatible object type");	\
  bs=(ejsByteSource *)JS_GetPrivate(cx,obj);		\
  if (!bs)						\
    EJS_THROW_ERROR(cx,obj,"no valid stream object")

static
JSBool
ejsByteSource_get(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
{
  EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
  GET_BYTESOURCE_OBJ;
  uint32 toread;
  if (!JS_ValueToECMAUint32(cx,argv[0],&toread)) return JS_FALSE;
  if (toread+1==0) --toread;

  char* buf=(char *)JS_malloc(cx,toread+1);
  if (!buf) return JS_FALSE;
  size_t got=bs->get(bs->object,buf,toread);
  assert(got<=toread);
  
  if (got<toread) {
    buf=(char *)JS_realloc(cx,buf,got+1);
    if (!buf) return JS_FALSE;
  }
  buf[got]='\0';
  JSString *r=JS_NewString(cx, buf, got);
  if (!r) {
    JS_free(cx,buf);
    return JS_FALSE;
  }
  *rval=STRING_TO_JSVAL(r);
  return JS_TRUE;
}

static
JSBool
ejsByteSource_available(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
{
  GET_BYTESOURCE_OBJ;
  return JS_NewNumberValue(cx,bs->available(bs->object),rval);
}
  

static
JSBool
ejsByteSource_close(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
{
  GET_BYTESOURCE_OBJ;
  *rval=(bs->deleteObject(bs->object)==0) ? JSVAL_TRUE : JSVAL_FALSE;
  JS_free(cx,bs);
  return JS_SetPrivate(cx,obj,NULL);
}

#undef GET_BYTESOURCE_OBJ

#define FUNC(name, args) { #name,ejsByteSource_##name,args,0,0}

static JSFunctionSpec ejsByteSource_methods[] = {
  FUNC(get,1),
  FUNC(available,0),
  FUNC(close,0),
  EJS_END_FUNCTIONSPEC
};

#undef FUNC

static
JSBool
ejsByteSource_cons(JSContext *, JSObject *, uintN, jsval *, jsval *)
{
  return JS_TRUE;
}

static
void
ejsByteSource_finalize(JSContext *cx, JSObject *obj)
{
  EJS_CHECK(JS_GET_CLASS(cx, obj) == &ejsByteSource_class);
  ejsByteSource* bs=(ejsByteSource *)JS_GetPrivate(cx,obj);
  if (!bs) return;
  bs->deleteObject(bs->object);
  JS_free(cx,bs);
}

static
size_t
ejsByteSource_defaultAvailable(void*)
{
  return 0;
}

static
int
ejsByteSource_defaultDeleteFunc(void*)
{
  return 0;
}

ejsByteSource*
ejsNewByteSource(JSContext *cx,
		 void* object,
		 ejsGetFunc get,
		 ejsAvailableFunc available,
		 ejsDeleteFunc deleteObject)
{
  ejsByteSource* bs=(ejsByteSource*)JS_malloc(cx,sizeof(ejsByteSource));
  if (!bs) return NULL;
  assert(get);
  bs->object=object;
  bs->get=get;
  bs->available=available ? available : ejsByteSource_defaultAvailable;
  bs->deleteObject=deleteObject ? deleteObject : ejsByteSource_defaultDeleteFunc;
  return bs;
}

JSBool
ejsDefineByteSource(JSContext *cx, JSObject *obj, const char* name, uintN flags, ejsByteSource* bs)
{
  assert(bs);
  JSObject *jsbs;
  assert(ejsByteSourceProto);
  if (!(jsbs=JS_DefineObject(cx, obj, name, &ejsByteSource_class, ejsByteSourceProto, flags)))
    return JS_FALSE;
  return JS_SetPrivate(cx,jsbs,(void *)bs);
}

static
size_t
ejsByteSource_fdReadFull(void* obj, void* buf, size_t count)
{
  int fd=ejs_ptrToInt(obj);
  size_t total=0;
  while (total<count) {
    size_t got=read(fd,(uint8_t*)buf+total,std::min(count-total,(size_t)SSIZE_MAX));
    if (got>0)
      total+=got;
    else if (((got==-1)&&((errno!=EAGAIN)&&(errno!=EINTR)))||(got==0))
      break;
  }
  return total;
}

static
int
ejsByteSource_fdClose(void* obj)
{
  int fd=ejs_ptrToInt(obj);
  return close(fd);
}

JSBool
ejsDefineByteSourceFromFD(JSContext *cx, JSObject *obj, const char* name, uintN flags, int fd, bool close)
{
  // create ByteSource object for fd
  ejsByteSource* bs=ejsNewByteSource(cx,
				     ejs_intToPtr(fd),
				     ejsByteSource_fdReadFull,
				     NULL,
				     close ? ejsByteSource_fdClose : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSource(cx,obj,name,flags,bs);
}

static
size_t
ejsByteSource_fread(void* obj, void* buf, size_t count)
{
  FILE* file=(FILE*)obj;
  size_t total=0;
  while (total<count) {
    size_t got=fread((uint8_t*)buf+total,1,count-total,file);
    if (got>0)
      total+=got;
    else if (feof(file)||ferror(file))
      break;
  }
  return total;
}

//! obtain bytes available in input buffer of FILE* stream
static
size_t
ejsByteSource_favailable(void* obj)
{
#warning using ugly hack to obtain bytes available in input buffer of FILE* stream

  FILE* f=(FILE*)obj;
#if defined(__GLIBC__) && defined(_STDIO_USES_IOSTREAM)
  // todo: wide character mode?!
  if (f->_IO_read_ptr < f->_IO_read_end)
    return f->_IO_read_end - f->_IO_read_ptr;
#else
#warning using non std _cnt field
  if (f->_cnt>0) return f->_cnt;
#endif
  // todo: if this FILE* has an associated fd use ejsByteSource_fdAvailable in
  // case the input buffer is empty
  return 0;
}

static
int
ejsByteSource_fclose(void* obj)
{
  return fclose((FILE*)obj);
}

JSBool
ejsDefineByteSourceFromFILE(JSContext *cx, JSObject *obj, const char* name, uintN flags, FILE* file, bool close)
{
  // create ByteSource object for fd
  ejsByteSource* bs=ejsNewByteSource(cx,
				     (void*)file,
				     ejsByteSource_fread,
				     ejsByteSource_favailable,
				     close ? ejsByteSource_fclose : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSource(cx,obj,name,flags,bs);
}

#ifdef __cplusplus
static
size_t
ejsByteSource_cppStreamRead(void* obj, void* buf, size_t count)
{
  assert(sizeof(char)==sizeof(uint8_t));
  std::streambuf* sb=static_cast<std::streambuf*>(obj);
  size_t total=0;
  while (total<count) {
    size_t got=sb->sgetn((char*)buf+total,std::min(count-total,static_cast<size_t>(std::numeric_limits<std::streamsize>::max())));
    if (got>0)
      total+=got;
    else
      break;
  }
  return total;
}

static
size_t
ejsByteSource_cppStreamAvailable(void* obj)
{
  return static_cast<std::streambuf*>(obj)->in_avail();
}

static
int
ejsByteSource_cppStreamDelete(void* obj)
{
  delete static_cast<std::streambuf*>(obj);
  return 0;
}

JSBool
ejsDefineByteSourceFromCppStream(JSContext *cx, JSObject *obj, const char* name, uintN flags,
				 std::streambuf* stream, bool del)
{
  // create ByteSource object for fd
  ejsByteSource* bs=ejsNewByteSource(cx,
				     (void*)stream,
				     ejsByteSource_cppStreamRead,
				     ejsByteSource_cppStreamAvailable,
				     del ? ejsByteSource_cppStreamDelete : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSource(cx,obj,name,flags,bs);
}
#endif

static JSObject *ejsByteSinkProto = NULL;

//! byte sink
struct ejsByteSink
{
  void* object;
  ejsPutFunc put;
  ejsFlushFunc flush;
  ejsDeleteFunc deleteObject;
};

static
void
ejsByteSink_finalize(JSContext *cx, JSObject *obj);

static
JSClass ejsByteSink_class = {
  "ByteSink",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  ejsByteSink_finalize,
  JSCLASS_NO_OPTIONAL_MEMBERS
};

#define GET_BYTESINK_OBJ ejsByteSink* bs=NULL;		\
  if (JS_GET_CLASS(cx, obj) != &ejsByteSink_class)	\
    EJS_THROW_ERROR(cx,obj,"incompatible object type");	\
  bs=(ejsByteSink *)JS_GetPrivate(cx,obj);		\
  if (!bs)						\
    EJS_THROW_ERROR(cx,obj,"no valid stream object")

static
JSBool
ejsByteSink_put(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
{
  EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
  GET_BYTESINK_OBJ;

  // todo: root string!
  JSString *strtype=JS_ValueToString(cx, argv[0]);
  if (!strtype) return JS_FALSE;
  // todo: we loose unicode information here
  char* ctype=JS_GetStringBytes(strtype);
  if (!ctype) return JS_FALSE;
  size_t w=bs->put(bs->object,ctype,JS_GetStringLength(strtype));
  return JS_NewNumberValue(cx,w,rval);
}

static
JSBool
ejsByteSink_flush(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
{
  GET_BYTESINK_OBJ;
  *rval=(bs->flush(bs->object)==0) ? JSVAL_TRUE : JSVAL_FALSE;
  return JS_TRUE;
}

static
JSBool
ejsByteSink_close(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
{
  GET_BYTESINK_OBJ;
  *rval=(bs->deleteObject(bs->object)==0) ? JSVAL_TRUE : JSVAL_FALSE;
  JS_free(cx,bs);
  return JS_SetPrivate(cx,obj,NULL);
}

#undef GET_BYTESINK_OBJ

#define FUNC(name, args) { #name,ejsByteSink_##name,args,0,0}

static JSFunctionSpec ejsByteSink_methods[] = {
  FUNC(put,1),
  FUNC(close,0),
  EJS_END_FUNCTIONSPEC
};

#undef FUNC

static
JSBool
ejsByteSink_cons(JSContext *, JSObject *, uintN, jsval *, jsval *)
{
  return JS_TRUE;
}

static
void
ejsByteSink_finalize(JSContext *cx, JSObject *obj)
{
  EJS_CHECK(JS_GET_CLASS(cx, obj) == &ejsByteSink_class);
  ejsByteSink* bs=(ejsByteSink *)JS_GetPrivate(cx,obj);
  if (!bs) return;
  bs->deleteObject(bs->object);
  JS_free(cx,bs);
}

static
size_t
ejsByteSink_defaultAvailable(void*)
{
  return 0;
}

static
int
ejsByteSink_defaultFlushFunc(void*)
{
  return 0;
}

static
int
ejsByteSink_defaultDeleteFunc(void*)
{
  return 0;
}

ejsByteSink*
ejsNewByteSink(JSContext *cx,
		 void* object,
		 ejsPutFunc put,
		 ejsFlushFunc flush,
		 ejsDeleteFunc deleteObject)
{
  ejsByteSink* bs=(ejsByteSink*)JS_malloc(cx,sizeof(ejsByteSink));
  if (!bs) return NULL;
  assert(put);
  bs->object=object;
  bs->put=put;
  bs->flush=flush ? flush : ejsByteSink_defaultFlushFunc;
  bs->deleteObject=deleteObject ? deleteObject : ejsByteSink_defaultDeleteFunc;
  return bs;
}

JSBool
ejsDefineByteSink(JSContext *cx, JSObject *obj, const char* name, uintN flags, ejsByteSink* bs)
{
  assert(bs);
  JSObject *jsbs;
  assert(ejsByteSinkProto);
  if (!(jsbs=JS_DefineObject(cx, obj, name, &ejsByteSink_class, ejsByteSinkProto, flags)))
    return JS_FALSE;
  return JS_SetPrivate(cx,jsbs,(void *)bs);
}

static
size_t
ejsByteSink_fdWriteFull(void* obj, const void* buf, size_t count)
{
  int fd=ejs_ptrToInt(obj);
  size_t total=0;
  while (total<count) {
    size_t written=write(fd,(uint8_t*)buf+total,std::min(count-total,(size_t)SSIZE_MAX));
    if (written>0)
      total+=written;
    else if (((written==-1)&&((errno!=EAGAIN)&&(errno!=EINTR)))||(written==0))
      break;
  }
  return total;
}

static
int
ejsByteSink_fdClose(void* obj)
{
  int fd=ejs_ptrToInt(obj);
  return close(fd);
}

JSBool
ejsDefineByteSinkFromFD(JSContext *cx, JSObject *obj, const char* name, uintN flags, int fd, bool close)
{
  // create ByteSink object for fd
  ejsByteSink* bs=ejsNewByteSink(cx,
				 ejs_intToPtr(fd),
				 ejsByteSink_fdWriteFull,
				 NULL,
				 close ? ejsByteSink_fdClose : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSink(cx,obj,name,flags,bs);
}

static
size_t
ejsByteSink_fwrite(void* obj, const void* buf, size_t count)
{
  FILE* file=(FILE*)obj;
  size_t total=0;
  while (total<count) {
    size_t written=fwrite((uint8_t*)buf+total,1,count-total,file);
    if (written>0)
      total+=written;
    else
      break;
  }
  return total;
}

static
int
ejsByteSink_fflush(void* obj)
{
  return fclose((FILE*)obj);
}

static
int
ejsByteSink_fclose(void* obj)
{
  return fclose((FILE*)obj);
}

JSBool
ejsDefineByteSinkFromFILE(JSContext *cx, JSObject *obj, const char* name, uintN flags, FILE* file, bool close)
{
  // create ByteSink object for fd
  ejsByteSink* bs=ejsNewByteSink(cx,
				 (void*)file,
				 ejsByteSink_fwrite,
				 ejsByteSink_fflush,
				 close ? ejsByteSink_fclose : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSink(cx,obj,name,flags,bs);
}

#ifdef __cplusplus
static
size_t
ejsByteSink_cppStreamWrite(void* obj, const void* buf, size_t count)
{
  assert(sizeof(char)==sizeof(uint8_t));
  std::streambuf* sb=static_cast<std::streambuf*>(obj);
  size_t total=0;
  while (total<count) {
    size_t written=sb->sputn((char*)buf+total,std::min(count-total,static_cast<size_t>(std::numeric_limits<std::streamsize>::max())));
    if (written>0)
      total+=written;
    else
      break;
  }
  return total;
}

static
int
ejsByteSink_cppStreamFlush(void* obj)
{
  return static_cast<std::streambuf*>(obj)->pubsync();
}

static
int
ejsByteSink_cppStreamDelete(void* obj)
{
  delete static_cast<std::streambuf*>(obj);
  return 0;
}

JSBool
ejsDefineByteSinkFromCppStream(JSContext *cx, JSObject *obj, const char* name, uintN flags,
				 std::streambuf* stream, bool del)
{
  // create ByteSink object for fd
  ejsByteSink* bs=ejsNewByteSink(cx,
				 (void*)stream,
				 ejsByteSink_cppStreamWrite,
				 ejsByteSink_cppStreamFlush,
				 del ? ejsByteSink_cppStreamDelete : NULL);
  if (!bs) return JS_FALSE;
  return ejsDefineByteSink(cx,obj,name,flags,bs);
}
#endif

JSBool
ejsio_LTX_onLoad(JSContext *cx, JSObject *module)
{
  ejsByteSourceProto = JS_InitClass(cx, module,
				    NULL,
				    &ejsByteSource_class,
				    ejsByteSource_cons, 0,
				    NULL, ejsByteSource_methods,
				    NULL, NULL);
  if (!ejsByteSourceProto) return JS_FALSE;

  // create stdin byte source
  if (!ejsDefineByteSourceFromFD(cx,module,"stdin",JSPROP_ENUMERATE,STDIN_FILENO,false))
    return JS_FALSE;
  if (!ejsDefineByteSourceFromFILE(cx,module,"fstdin",JSPROP_ENUMERATE,stdin,false))
    return JS_FALSE;
#ifdef __cplusplus
  if (!ejsDefineByteSourceFromCppStream(cx,module,"cppstdin",JSPROP_ENUMERATE,std::cin.rdbuf(),false))
    return JS_FALSE;
#endif

  ejsByteSinkProto = JS_InitClass(cx, module,
				    NULL,
				    &ejsByteSink_class,
				    ejsByteSink_cons, 0,
				    NULL, ejsByteSink_methods,
				    NULL, NULL);
  if (!ejsByteSinkProto) return JS_FALSE;

  // create stdout and stderr byte sink
  if (!ejsDefineByteSinkFromFD(cx,module,"stdout",JSPROP_ENUMERATE,STDOUT_FILENO,false))
    return JS_FALSE;
  if (!ejsDefineByteSinkFromFD(cx,module,"stderr",JSPROP_ENUMERATE,STDERR_FILENO,false))
    return JS_FALSE;
  if (!ejsDefineByteSinkFromFILE(cx,module,"fstdout",JSPROP_ENUMERATE,stdout,false))
    return JS_FALSE;
  if (!ejsDefineByteSinkFromFILE(cx,module,"fstderr",JSPROP_ENUMERATE,stderr,false))
    return JS_FALSE;
#ifdef __cplusplus
  if (!ejsDefineByteSinkFromCppStream(cx,module,"cppstdout",JSPROP_ENUMERATE,std::cout.rdbuf(),false))
    return JS_FALSE;
  if (!ejsDefineByteSinkFromCppStream(cx,module,"cppstderr",JSPROP_ENUMERATE,std::cerr.rdbuf(),false))
    return JS_FALSE;
#endif

  return JS_TRUE;
}
