#ifndef EJS_MACROS_H
#define EJS_MACROS_H

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
   \brief Some error handling and log message macros
   \author Jens Thiele
*/

#include <iostream>

/*
  \def EJS_STR(s)
  \brief EJS_STR converts the macro argument to a string constant
*/
#define EJS_STR(s) #s

/*
  \def EJS_STR(s)
  \brief EJS_STR converts the macro argument to a string constant
  \note this works also if the argument is a macro
*/
#define EJS_XSTR(s) EJS_STR(s)

#define EJS_ERRORSTR "errno="<<jg_errno<<":"<<strerror(jg_errno)
#define EJS_FUNCTIONNAME __PRETTY_FUNCTION__
#define EJS_HERE __FILE__ ":" << __LINE__ << ":" << EJS_FUNCTIONNAME
#define EJS_MSG(ejsLevelP, ejsMsgP, printerrno) do{int jg_errno=errno;errno=0;std::cerr << ejsLevelP << EJS_HERE << ":\n    " << ejsMsgP << "\n";if(printerrno&&jg_errno) std::cerr << "    (" << EJS_ERRORSTR << ")\n";}while(0)
#define EJS_WARN(msg) EJS_MSG("WARNING: ",msg, 1)
#define EJS_INFO(msg) EJS_MSG("INFO: ",msg, 0)
#define EJS_ERROR(msg) EJS_MSG("ERROR: ",msg, 1)


/*
  \def EJS_CHECK(expr)
  \brief EJS_CHECK is like assert but takes also effect when NDEBUG is defined
  
  it seems there is no assert which is not disabled by a \#define NDEBUG 
*/
#define EJS_CHECK(expr) (static_cast<void> ((expr) ? 0 : ejs_fatal(__FILE__, __LINE__, EJS_FUNCTIONNAME, "assertion failed: " EJS_STR(expr))))

/*!
  \def EJS_FATAL(msg)
  \brief fatal error - abort with a short message
*/
#define EJS_FATAL(msg) do{ejs_fatal(__FILE__, __LINE__, EJS_FUNCTIONNAME, msg);}while(0)

inline int ejs_fatal(const char *file,int line,const char *func, const char *msg) 
{
  int jg_errno=errno;
  errno=0;
  std::cerr << "FATAL: " << file << ":" << line << ":" << func << ": " << msg << "(" << EJS_ERRORSTR <<")\n";
  std::terminate();
  return 0;
}

#endif
