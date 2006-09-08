#ifndef EJS_IO_H
#define EJS_IO_H

#include <limits.h>
#ifndef SSIZE_MAX
#warning SSIZE_MAX not defined in limits.h - i try to take a safe but possibly slow guess
#define SSIZE_MAX SHRT_MAX
#endif

#include "ejsmodule.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*!
    function to be called if bytesource isn't needed anymore
    \return 0 on sucess
  */
  typedef int (*ejsDeleteFunc)(void* obj);

  //! "embed" int in void*
  /*!
    \note this function exists since "it isn't portable" (non-std) s.a. http://c-faq.com/ptrs/int2ptr.html
    \todo perhaps use a union instead
  */
  inline
  void*
  ejs_intToPtr(int fd)
  {
    assert(sizeof(void*)>=sizeof(int));
    void* ret=(void*)fd;
    assert(fd==(int)ret);
    return ret;
  }

  //! extract int from void*
  /*!
    \note this function exists since "it isn't portable" (non-std) s.a. http://c-faq.com/ptrs/int2ptr.html
    \todo perhaps use a union instead
  */
  inline
  int
  ejs_ptrToInt(void* ptr)
  {
    return (int)ptr;
  }

  /*!
    function that gets count bytes from byte source and puts them into buf
    \return number of bytes got (todo: eof vs. read error vs. xxx?)
  */
  typedef size_t (*ejsGetFunc)(void* obj, void* buf, size_t count);
  /*!
    function returning the 
    \return number of bytes that are "guaranteed" to be available (will not "block")
  */
  typedef size_t (*ejsAvailableFunc)(void* obj);

  //! byte source
  struct ejsByteSource;

  //! create new C ByteSource object
  /*!
    \return NULL on error (js error state is set)
  */
  ejsByteSource*
  ejsNewByteSource(JSContext *cx,
		   void* object,
		   ejsGetFunc get,
		   ejsAvailableFunc available=NULL,
		   ejsDeleteFunc deleteObject=NULL);

  //! create javascript ByteSource object for C ByteSource object
  JSBool
  ejsDefineByteSource(JSContext *cx, JSObject *obj, const char* name, uintN flags, ejsByteSource* bs);

  //! create new C and JS ByteSource object from file descriptor
  JSBool
  ejsDefineByteSourceFromFD(JSContext *cx, JSObject *obj, const char* name, uintN flags, int fd, bool close);

  //! create new C and JS ByteSource object from c stream (FILE*)
  JSBool
  ejsDefineByteSourceFromFILE(JSContext *cx, JSObject *obj, const char* name, uintN flags, FILE* file, bool close);

#ifdef __cplusplus
  //! create new C and JS ByteSource object from c++ stream
  JSBool
  ejsDefineByteSourceFromCppStream(JSContext *cx, JSObject *obj, const char* name, uintN flags,
				   std::streambuf* stream, bool del);
#endif

  /*!
    function that puts count bytes from buf to sink
    \return number of bytes put (todo: eof vs. write error vs. xxx?)
  */
  typedef size_t (*ejsPutFunc)(void* obj, const void* buf, size_t count);

  //! function to flush output buffer (if any)
  typedef int (*ejsFlushFunc)(void* obj);

  //! byte sink
  struct ejsByteSink;

  //! create new C ByteSink object
  /*!
    \return NULL on error (js error state is set)
  */
  ejsByteSink*
  ejsNewByteSink(JSContext *cx,
		 void* object,
		 ejsPutFunc put,
		 ejsFlushFunc flush=NULL,
		 ejsDeleteFunc deleteObject=NULL);

  //! create javascript ByteSink object for C ByteSink object
  JSBool
  ejsDefineByteSink(JSContext *cx, JSObject *obj, const char* name, uintN flags, ejsByteSink* bs);

  //! create new C and JS ByteSink object from file descriptor
  JSBool
  ejsDefineByteSinkFromFD(JSContext *cx, JSObject *obj, const char* name, uintN flags, int fd, bool close);

  //! create new C and JS ByteSink object from c stream (FILE*)
  JSBool
  ejsDefineByteSinkFromFILE(JSContext *cx, JSObject *obj, const char* name, uintN flags, FILE* file, bool close);

#ifdef __cplusplus
  //! create new C and JS ByteSink object from c++ stream
  JSBool
  ejsDefineByteSinkFromCppStream(JSContext *cx, JSObject *obj, const char* name, uintN flags,
				 std::streambuf* stream, bool del);
#endif

  JSBool
  ejsio_LTX_onLoad(JSContext *cx, JSObject *module);

#ifdef __cplusplus
}
#endif

#endif
