#include "jsnetwork.h"
#include "error.h"
#include "dopeexcept.h"
#include <sstream>

// connection to server
static NetStreamBuf* outgoing=0;
static int connectAttempts=0;

extern "C" {
  ECMA_BEGIN_FUNC(jsnet_connect) 
  {
    if (connectAttempts) ECMA_ERROR("Security: Already tried to establish an outgoing connection");
    ECMA_CHECK_NUM_ARGS(2);
    // we allow only one connection for security reasons
    if (outgoing) return JS_FALSE;
    if (!JSVAL_IS_STRING(argv[0])) ECMA_ERROR("Argument 0 must be a hostname");
    int32 port;
    if (!JS_ValueToInt32(cx,argv[1],&port)) ECMA_ERROR("Argument 1 must be a port number");
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    try {
      ++connectAttempts;
      outgoing=new NetStreamBuf(InternetAddress(ctype,port));
      if (!outgoing) return JS_FALSE;
      if (!JSNetwork::newStreamObject(outgoing,rval)) return JS_FALSE;
    }catch(const SocketError &e){
      JS_ReportError(cx,e.what());
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  ECMA_BEGIN_METHOD(stream_send) 
  {
    ECMA_CHECK_NUM_ARGS(1);
    NetStreamBuf* stream=(NetStreamBuf *)JS_GetPrivate(cx,obj);
    if (!stream) return JS_FALSE;
    if (!JSVAL_IS_STRING(argv[0])) return JS_FALSE;
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    int w=stream->sputn(ctype,strlen(ctype));
    return JS_NewNumberValue(cx,w,rval);
  }

  ECMA_BEGIN_METHOD(stream_recv) 
  {
    ECMA_CHECK_NUM_ARGS(1);
    NetStreamBuf* stream=(NetStreamBuf *)JS_GetPrivate(cx,obj);
    if (!stream) return JS_FALSE;
    if (!JSVAL_IS_INT(argv[0])) return JS_FALSE;
    int toread=JSVAL_TO_INT(argv[0]);
    char* buf=(char *)JS_malloc(cx,toread+1);
    if (!buf) return JS_FALSE;
    int got=stream->sgetn(buf,toread);
    JGACHINE_CHECK(got>=0);
    JGACHINE_CHECK(got<=toread);
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

  ECMA_BEGIN_VOID_METHOD_VOID(stream_sync)
  {
    ECMA_CHECK_NUM_ARGS(0);
    NetStreamBuf* stream=(NetStreamBuf *)JS_GetPrivate(cx,obj);
    if (!stream) return JS_FALSE;
    stream->pubsync();
    return JS_TRUE;
  }

  ECMA_BEGIN_METHOD(stream_select) 
  {
    NetStreamBuf* stream=(NetStreamBuf *)JS_GetPrivate(cx,obj);
    if (!stream) return JS_FALSE;
    JGACHINE_SMARTPTR<Timer::TimeStamp> timeout;
    assert(!timeout.get());
    if ((argc>=1)&&(JSVAL_IS_INT(argv[0])))
      timeout=JGACHINE_SMARTPTR<Timer::TimeStamp>(new Timer::TimeStamp(JSVAL_TO_INT(argv[0])));
    bool r=stream->select(timeout.get());
    *rval=BOOLEAN_TO_JSVAL(r);
    return JS_TRUE;
  }
}

#define JSFUNC(prefix, name, args) { #name,prefix##name,args,0,0}

static JSFunctionSpec stream_methods[] = {
  JSFUNC(stream_, send,1),
  JSFUNC(stream_, recv,1),
  JSFUNC(stream_, sync,0),
  JSFUNC(stream_, select,1),
  ECMA_END_FUNCSPECS
};

static JSFunctionSpec net_static_methods[] = {
  JSFUNC(jsnet_, connect,2),
  ECMA_END_FUNCSPECS
};

#undef JSFUNC

static JSClass stream_class = {
  "Stream",JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
  JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
  ECMA_END_CLASS_SPEC
};

JSBool
JSNetwork::newStreamObject(NetStreamBuf* s, jsval* rval)
{
  JSObject *obj=JS_NewObject(ECMAScript::cx, &stream_class, NULL, NULL);
  if (!obj) return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(obj);
  assert(JSVAL_IS_OBJECT(*rval));
  if (!JS_DefineFunctions(ECMAScript::cx, obj, stream_methods)) return JS_FALSE;
  if (!JS_SetPrivate(ECMAScript::cx,obj,(void *)s)) return JS_FALSE;
  return JS_TRUE;
}

bool
JSNetwork::init()
{
  JSObject *obj = JS_DefineObject(ECMAScript::cx, ECMAScript::glob,
				  "Net", NULL, NULL, JSPROP_ENUMERATE);
  if (!obj) return JS_FALSE;
  return JS_DefineFunctions(ECMAScript::cx, obj, net_static_methods);
}

bool
JSNetwork::deinit()
{
  if (outgoing) {
    delete outgoing;
    outgoing=0;
  }
  return true;
}
