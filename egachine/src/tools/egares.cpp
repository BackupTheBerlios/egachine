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
  \file tools/egares.cpp
  \brief resource tool
  \author Jens Thiele
*/

/*
  tool to convert files to resources
  egachine/egaserver are not allowed to open files
  => everything must be in one file or received from the server
  (which in turn must read it from one file)
*/

#include <iostream>
#include <string>
#include <fstream>

#include "common/error.h"
#include "common/ecmascript.h"
#include "common/jszlib.h"

//! the program
/*!
  \note we use a struct/class to make sure everything is always deinititalized (by the destructor)
*/
struct EGares
{
  int
  init() 
  {
    if (!ECMAScript::init()) {
      JGACHINE_ERROR("could not inititialize interpreter");
      ECMAScript::deinit();
      return EXIT_FAILURE;
    }

    // now register our objects/functions
    if (!(JSZlib::init())) {
      JGACHINE_ERROR("could not register functions/objects with interpreter");
      ECMAScript::deinit();
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

  ~EGares()
  {
    JSZlib::deinit();
    ECMAScript::deinit();
  }

  //! copy stream to JSString
  /*!
    \note the result is not rooted! be careful not to run the GC
  */
  JSString*
  streamToString(std::streambuf* buf)
  {
    // copy streambuf to javascript string
    // hmm quite a hack
    const unsigned blkSize=1024;
    unsigned destLen=0;
    char* dest=NULL;
    unsigned got;
  
    do{
      dest=(char *)JS_realloc(ECMAScript::cx,dest,destLen+blkSize);
      if (!dest) return NULL;
      got=buf->sgetn(dest+destLen,blkSize);
      destLen+=got;
    }while(got==blkSize); // todo this might abort to early?
    dest=(char *)JS_realloc(ECMAScript::cx,dest,destLen);
    if (!dest) return NULL;
  
    return JS_NewString(ECMAScript::cx, dest, destLen);
  }

  int
  main(int argc, char** argv)
  {
    // load libs
    ECMAScript::parseLib("common.js");
    ECMAScript::parseLib("egares.js");

    // copy version information to the interpreter
    ECMAScript::setVersion("EGachine.version");

    ECMAScript::copyargv(argc,argv);

    // Attention: we do not root s
    JSString* s=NULL;
    if (argc<3)
      s=streamToString(std::cin.rdbuf());
    else {
      std::ifstream in(argv[2]);
      if (!in.good()) {
	JGACHINE_ERROR("Could not open file: \""<<argv[1]<<"\"");
	return EXIT_FAILURE;
      }
      s=streamToString(in.rdbuf());
    }

    JGACHINE_CHECK(s);
    jsval param[1];
    param[0]=STRING_TO_JSVAL(s);
    jsval rval;
    if (!JS_CallFunctionName(ECMAScript::cx, ECMAScript::glob, "egares", 1, param, &rval)) {
      JGACHINE_ERROR("error while calling egares. TODO: why don't we get more info?");
      ECMAScript::handleExceptions();
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
};

void
printUsage()
{
  std::cerr << "Usage: egares [-h|--help] RESNAME [FILE] [OPTION]...\n";
}

int
main(int argc, char** argv)
{
  if (argc<2) {
    printUsage();
    return EXIT_FAILURE;
  }
  std::string resname(argv[1]);

  if (argc==2) {
    if ((resname=="-h")||(resname=="--help")) {
      printUsage();
      return EXIT_SUCCESS;
    }
  }
  
  EGares egares;
  if (egares.init()!=EXIT_SUCCESS) return EXIT_FAILURE;
  return egares.main(argc,argv);
}

