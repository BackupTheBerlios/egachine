static
JSBool
EJS_FUNC(appendChild)
(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
  GET_NTHIS;
  EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
  JSObject* jschild=NULL;
  if ((!JSVAL_IS_OBJECT(argv[0]))||(!(jschild=JSVAL_TO_OBJECT(argv[0]))))
    EJS_THROW_ERROR(cx,obj,"object as arg 0 required");
  
  // todo: exceptions
  *rval=OBJECT_TO_JSVAL(obj);
  
  dom::Element* element=NULL;
  if (ejselement_class(cx, jschild)&&ejselement_GetNative(cx,jschild,element)) {
    nthis->appendChild(element);
    return JS_TRUE;
  }
  dom::Text* text=NULL;
  if (ejstext_class(cx, jschild)&&ejstext_GetNative(cx,jschild,text)) {
    nthis->appendChild(text);
    return JS_TRUE;
  }
  EJS_THROW_ERROR(cx,obj,"not yet supported");
}

static
JSBool
EJS_FUNC(setNodeValue)
(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
{
  GET_NTHIS;
  EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
  JSString *strtype=JS_ValueToString(cx, argv[0]);
  if (!strtype) return JS_FALSE;
  unicode::String* value=unicode::String::createStringUtf16(JS_GetStringChars(strtype));
  try{
    nthis->setNodeValue(value);
  }catch(const dom::DOMException &e){
    EJS_THROW_ERROR(cx, obj, e.what());
  }
  return JS_TRUE;
}
