#ifndef __BYTE_ARRAY_H__
#define __BYTE_ARRAY_H__

/*
 * js_InitByteArrayClass
 *
 * Initialize the byte array class, and make it available globally
 */
extern JSObject* js_InitByteArrayClass(JSContext *cx, 
				       JSObject *parentObj);

/*
 * js_NewByteArray
 *
 * Creates and returns a new byte array instance of 
 * capacity len. The data passed to this function will
 * be owned by the newly constructed ByteArray and finalized
 * when its destructors kick in.
 */
extern JSObject* js_NewByteArray(JSContext *cx,
				 unsigned char *data, jsuint len, 
				 JSBool copyData);


extern JSBool js_GetByteArrayLength(JSContext *cx,
				    JSObject *obj,
				    jsint *len);


extern JSBool js_GetByteArrayBytes(JSContext *cx,
				   JSObject *obj,
				   unsigned char** data);

#endif
