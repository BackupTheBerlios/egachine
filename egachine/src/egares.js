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
   \file egares.js
   \brief resource utility hack
   \author Jens Thiele
*/

//! wrap a string to fit into 80 columns
function wrapString(str)
{
  var result="";
  var s=0,e,cols=80;
  while ((e=s+cols-1)<=str.length) {
    result+=str.substring(s,e)+"\\\n";
    s+=cols-1;
  }
  result+=str.substring(s,e);
  return result;
}

function egares(resource) {
  var resname=argv[0];
  var res=new Resource(resname,resource);
  var stdout="EGachine.addResource(\n";

  res=res.toSource();

  // this is buggy since we may use "\" only in a string to wrap wrong lines
  // but at least we detect errors (below)
  stdout+=wrapString(res)+");";
  // check for errors (decoding should result in the resource again)
  eval(stdout);
  var dec=EGachine.getResource(resname).decode();
  if (dec!=resource)
    throw new Error("egares is buggy and something went wrong");

  // puh ;-) good luck
  print(stdout);
}
