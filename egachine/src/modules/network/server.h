/*
 * Copyright (C) 2004 Jens Thiele <karme@berlios.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*!
   \brief Server Class
   \author Jens Thiele
*/

#ifndef SERVER_H
#define SERVER_H

#include <ejsmodule.h> // EJS_ macros

#include "netstreambufserver.h"
#include "ejsnet.h"

//! exception we throw within our callbacks
struct CallbackError
{};

//! native server object to be wrapped
/*!
  \note the difficulty is that we must call back to javascript
*/
struct Server : public SigC::Object
{
  NetStreamBufServer listener;
  
  //! our context
  JSContext* cx;
  
  //! our wrapper object
  JSObject* obj;
  
  Server(JSContext* _cx, JSObject* _obj, Port port) : listener(port), cx(_cx), obj(_obj)
  {
  }
  
  //! call method of our wrapper object
  JSBool
  callJSMethod(const char* fname, jsuint argc, jsval* argv)
  {
    jsval rval;
    
    EJS_CHECK(obj);
    EJS_CHECK(fname);
    if (argc>0) EJS_CHECK(argv);
    
    return JS_CallFunctionName(cx, obj, fname, argc, argv, &rval);
  }
  
  JSBool
  init()
  {
    try {
      EJS_CHECK(listener.init());
    }catch(const SocketError &e){
      EJS_THROW_ERROR(cx,obj,e.what());
    }
    listener.newConnection.connect(SigC::slot(*this,&Server::handleNewConnection));
    listener.dataAvailable.connect(SigC::slot(*this,&Server::handleDataAvailable));
    return JS_TRUE;
  }
  
  void
  handleNewConnection(NetStreamBufServer::ID id, NetStreamBuf* streamPtr)
  {
    assert(streamPtr);
    // todo those values are not rooted
    jsval args[2];
    if (!JS_NewNumberValue(cx,id,&args[0]))
      throw CallbackError();
    if (!newStreamObject(cx,obj,streamPtr,&args[1]))
      throw CallbackError();
    if (!callJSMethod("handleNewConnection",2,args))
      throw CallbackError();
  }
  
  void
  handleDataAvailable(NetStreamBufServer::ID id, NetStreamBuf* streamPtr)
  {
    assert(streamPtr);
    jsval args[1];
    
    if (!JS_NewNumberValue(cx,id,&args[0]))
      throw CallbackError();
    if (!callJSMethod("handleDataAvailable",1,args))
      throw CallbackError();
  }

};

#endif
