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
   \file common/error.h
   \brief some error handling macros
   \author Jens Thiele
*/

#ifndef JGACHINE_ERROR_H
#define JGACHINE_ERROR_H

#include <iostream>
#include <stdexcept>

/*
  \def JGACHINE_STR(s)
  \brief JGACHINE_STR converts the macro argument to a string constant
*/
#define JGACHINE_STR(s) #s

/*
  \def JGACHINE_STR(s)
  \brief JGACHINE_STR converts the macro argument to a string constant
  \note this works also if the argument is a macro
*/
#define JGACHINE_XSTR(s) JGACHINE_STR(s)

#define JGACHINE_ERRORSTR "errno="<<jg_errno<<":"<<strerror(jg_errno)
#define JGACHINE_FUNCTIONNAME __PRETTY_FUNCTION__
#define JGACHINE_HERE __FILE__ ":" << __LINE__ << ":" << JGACHINE_FUNCTIONNAME
#define JGACHINE_MSG(jgachineLevelP, jgachineMsgP, printerrno) do{int jg_errno=errno;errno=0;std::cerr << jgachineLevelP << JGACHINE_HERE << ":\n    " << jgachineMsgP << "\n";if(printerrno&&jg_errno) std::cerr << "    (" << JGACHINE_ERRORSTR << ")\n";}while(0)
#define JGACHINE_WARN(msg) JGACHINE_MSG("WARNING: ",msg, 1)
#define JGACHINE_INFO(msg) JGACHINE_MSG("INFO: ",msg, 0)
#define JGACHINE_ERROR(msg) JGACHINE_MSG("ERROR: ",msg, 1)


/*
  \def JGACHINE_CHECK(expr)
  \brief JGACHINE_CHECK is like assert but takes also effect when NDEBUG is defined
  
  it seems there is no assert which is not disabled by a \#define NDEBUG 
*/
#define JGACHINE_CHECK(expr) (static_cast<void> ((expr) ? 0 : jgachine_fatal(__FILE__, __LINE__, JGACHINE_FUNCTIONNAME, "assertion failed: " JGACHINE_STR(expr))))

/*!
  \def JGACHINE_FATAL(msg)
  \brief fatal error - abort with a short message
*/
#define JGACHINE_FATAL(msg) do{jgachine_fatal(__FILE__, __LINE__, JGACHINE_FUNCTIONNAME, msg);}while(0)

inline int jgachine_fatal(const char *file,int line,const char *func, const char *msg) 
{
  int jg_errno=errno;
  errno=0;
  std::cerr << "FATAL: " << file << ":" << line << ":" << func << ": " << msg << "(" << JGACHINE_ERRORSTR <<")\n";
  std::terminate();
  return 0;
}

#define JGACHINE_THROW(msg) throw new std::runtime_error(msg)

#endif
