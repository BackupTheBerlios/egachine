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

// small opengl demo for egachine

if (!EGachine.client) throw new Error("This file must be run by egachine");
EGachine.checkVersion("0.1.2");
var gl=ejs.ModuleLoader.get("gl");

function drawCircle(r)
{
  gl.Begin(GL_LINE_LOOP);
  var i=0;
  for (i=0;i<2*Math.PI;i+=2*Math.PI/10)
    gl.Vertex2f(r*Math.sin(i),r*-Math.cos(i));
  gl.End();
}

function drawBackground()
{
  gl.Begin(GL_LINES);
  var i,min=-1,max=3,c=0;
  for (i=min;i<max;i+=0.1,++c) {
    gl.Color3f((c%3) ? 0.5 : 0.8, ((c+1)%3) ? 0.5 : 0.8, ((c+2)%3) ? 0.5 : 0.8);
    gl.Vertex2f(i,0.2);
    gl.Vertex2f(i,0.4);
    gl.Vertex2f(i-0.02,0.4-0.02);
    gl.Vertex2f(i,0.4);
    gl.Vertex2f(i,0.4);
    gl.Vertex2f(i+0.02,0.4-0.02);
  }
  gl.Color3f(1,1,1); // todo push/pop
  gl.Vertex2f(min,0.2);
  gl.Vertex2f(max,0.2);
  gl.Vertex2f(min,0.36);
  gl.Vertex2f(max,0.36);
  gl.End();
}

function Man() {
  this.leg=[0,0];
  this.l=0;
}

Man.prototype.paint=function()
{
  var head=0.9-Math.abs(this.leg[0]/10);
  var neck=head-0.08;
  var body=neck-0.3;
  var knee=body-0.2;
  var bottom=0;
  var tl=0.2;
  var bl=0.2;

  // head
  gl.PushMatrix();
  gl.Translatef(0,head,0);
  drawCircle(head-neck);
  gl.PopMatrix();

  gl.Begin(GL_LINES);
  // body
  gl.Vertex2f(0,neck);
  gl.Vertex2f(0,body);

  var i;
  var maxx=0;
  for (i=0;i<2;++i) {
    // legs
    var alpha=this.leg[i];
    var kx=Math.sin(alpha)*tl;
    var ky=body-Math.cos(alpha)*tl;
    gl.Vertex2f(0,body);
    gl.Vertex2d(kx,ky);
    gl.Vertex2d(kx,ky);
    var beta;
    if (alpha>0) {
      beta=-alpha/8;
      kx-=Math.abs(Math.sin(beta)*bl);
      ky-=Math.cos(beta)*bl;
    }else{
      beta=2*alpha;
      kx-=Math.abs(Math.sin(beta)*bl);
      ky-=Math.cos(beta)*bl;
    }
    gl.Vertex2d(kx,ky);
    gl.Vertex2d(kx,ky);
    var gamma;
    if (alpha>0)
      gamma=beta+Math.PI/2;
    else
      gamma=beta+0.9*Math.PI/2;
    kx+=Math.sin(gamma)*bl/3;
    ky-=Math.cos(gamma)*bl/3;
    gl.Vertex2d(kx,ky);
    maxx=Math.max(maxx,kx);
  }
  maxx+=0.05;
  gl.Vertex2f(maxx-0.4,0.09);
  gl.Vertex2f(maxx,0.09);
  gl.Vertex2f(maxx,0.6);
  gl.Vertex2f(maxx,0.09);

  // arm
  gl.Vertex2f(0,neck);
  gl.Vertex2f(maxx/2,0.55);
  gl.Vertex2f(maxx/2,0.55);
  gl.Vertex2f(maxx,0.6);

  gl.Vertex2f(0,neck);
  gl.Vertex2f(maxx/2,0.58);
  gl.Vertex2f(maxx/2,0.58);
  gl.Vertex2f(maxx,0.6);

  gl.End();
  gl.PushMatrix();
  gl.Translatef(maxx-0.4,0.09,0);
  drawCircle(0.03);
  gl.Translatef(0.4,0,0);
  drawCircle(0.03);
  gl.PopMatrix();
  gl.PushMatrix();
  gl.Translatef(maxx,0.6,0);
  drawCircle(0.02);
  gl.PopMatrix();

}
Man.prototype.step=function(dt)
{
  var i;
  for (i=0;i<2;++i)
    this.leg[i]+=((this.l==i) ? 3*dt : -3*dt);
  if (this.leg[0]>1) this.l=1;
  else if (this.leg[0]<-1) this.l=0;
}


viewport=Video.getViewport();
gl.Scalef(viewport[3],viewport[3],1);
gl.Enable(GL_LINE_SMOOTH);

var back = gl.GenLists(1);
gl.NewList(back, GL_COMPILE);
drawBackground();
gl.EndList();

man=new Man();

var x=0;
var start=Timer.getTimeStamp();
var last=start;
var dt;
while (true) {
  now=Timer.getTimeStamp();
  dt=(now-last)/1000000.0;
  last=now;

  Input.poll();
  gl.Clear(GL_COLOR_BUFFER_BIT);

  gl.LineWidth(2);

  gl.PushMatrix();
  gl.Translatef(-1.4*x,0.2,0);
  gl.Scalef(0.6,0.6,1);
  gl.CallList(back);
  gl.Translatef(5,0,0);
  gl.CallList(back);
  gl.PopMatrix();

  gl.LineWidth(4);

  gl.PushMatrix();
  gl.Translatef(-2*x,0,0);
  gl.CallList(back);
  gl.Translatef(4,0,0);
  gl.CallList(back);
  gl.PopMatrix();

  x+=0.3*dt;
  if (x>2) x-=2.4;

  gl.LineWidth(5);

  gl.PushMatrix();
  gl.Translatef(x,0,0);
  man.paint();
  gl.PopMatrix();
  man.step(dt);
  Video.swapBuffers();
  //  Timer.uSleep(10000);
}
