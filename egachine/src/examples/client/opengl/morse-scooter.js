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
if (!EGachine.checkVersion(0,0,5)) throw new Error("at least version 0.0.5 required");
if (!this.gl) throw new Error("This game needs OpenGL");


players=[];

// http://javascript.internet.com/equivalents/morse-code.html
// EGachine
// var morse=".--..--.-.......-..";
// sos
var morse="...---...";
pads=[];

var morseText="Please morse:"+morse;


Input.handleInput=function(i){
  if ((i.dev!=undefined)&&(i.dev>=0)&&(i.dev<2)) {
    if (pads[i.dev]) {
      var c=-1;
      for (a in i) {
	if (i[a]!=pads[i.dev][a]) {
	  handlePress(c*2+i.dev,i[a]);
	}
	++c;
      }
    }
    pads[i.dev]=i;
  }
}

//! handle player button press/release
function handlePress(id,press){
  var stamp=Timer.getTimeStamp();
  if (players[id]) {
    var dt=stamp-players[id].stamp;
    if (!press) {
      // button released
      var got='?';
      if ((dt>60000)&&(dt<350000)) got='.';
      else if ((dt>350000)&&(dt<1000000)) got='-';
      //      print(id+": expect:"+morse[players[id].expect]+" got:"+got);
      if (got==(morse[players[id].expect])) {
	players[id].speed+=0.002;
	scooters[id].headColor=[0,1,0];
      }else{
	players[id].speed-=0.02;
	if (got=='?')
	  scooters[id].headColor=[1,0.5,0];
	else
	  scooters[id].headColor=[1,0,0];
      }
      players[id].speed=Math.max(0.5,players[id].speed);
      players[id].speed=Math.min(1.5,players[id].speed);
      players[id].expect++;
      if (players[id].expect>=morse.length) players[id].expect=0;
    }else{
      // button pressed
      if (dt>1000000) {
	//	print ("restart ("+id+")");
	players[id].expect=0;
      }
    }
  }else{
    // new player
    players[id]={expect:0,speed:1};
  }
  players[id].lastPressed=press;
  players[id].stamp=stamp;
}

function drawCircle(r,l)
{
  gl.PushAttrib(GL_CURRENT_BIT);
  gl.Color3f(0,0,0);
  gl.Begin(GL_TRIANGLE_FAN);
  gl.Vertex2f(0,0);
  var i=0;
  for (i=0;i<=2*Math.PI;i+=2*Math.PI/l)
    gl.Vertex2f(r*Math.sin(i),r*-Math.cos(i));
  gl.End();
  gl.PopAttrib();
  gl.Begin(GL_LINE_LOOP);
  var i=0;
  for (i=0;i<2*Math.PI;i+=2*Math.PI/l)
    gl.Vertex2f(r*Math.sin(i),r*-Math.cos(i));
  gl.End();
}

function drawWheel()
{
  drawCircle(0.09,20);
}

function drawEmptyScooter()
{
  gl.PushMatrix();
  gl.Translatef(-0.4,0,0);
  drawWheel();
  gl.Translatef(0.4,0,0);
  drawWheel();
  gl.PopMatrix();
  gl.Begin(GL_LINE_STRIP);
  gl.Vertex2f(-0.4,0);
  gl.Vertex2f(0,0);
  gl.Vertex2f(0,0.5);
  gl.End();
  gl.PushMatrix();
  gl.Translatef(0,0.5,0);
  drawCircle(0.02,7);
  gl.PopMatrix();
}

function drawStreet(tracks)
{
  var i,z;
  gl.Begin(GL_LINES);
  for (i=-10;i<15;++i) {
    gl.Vertex3f(i,ground,-2);
    gl.Vertex3f(i,ground,minz-2);

    gl.Vertex3f(i,ground,minz-2);
    gl.Vertex3f(i,ground+1,minz-2);

    gl.Vertex3f(i,ground+1,minz-2);
    gl.Vertex3f(i,ground+1+0.2,minz-2+0.2);


    gl.Vertex3f(i,ground+1+0.2+0.2,minz-2+0.2);
    gl.Vertex3f(i,ground+1+0.2+0.4,minz-2+0.2);

    if (i%2) {
      gl.Vertex3f(i,ground+1+0.2+0.4,minz-2+0.2);
      gl.Vertex3f(i+1,ground+1+0.2+0.4,minz-2+0.2);
    }else{
      gl.Vertex3f(i,ground+1+0.2+0.2,minz-2+0.2);
      gl.Vertex3f(i+1,ground+1+0.2+0.2,minz-2+0.2);
    }
  }
  gl.Vertex3f(-10,ground,minz-2);
  gl.Vertex3f(+15,ground,minz-2);

  gl.Vertex3f(-10,ground+1,minz-2);
  gl.Vertex3f(+15,ground+1,minz-2);

  gl.Vertex3f(-10,ground+1+0.2,minz-2+0.2);
  gl.Vertex3f(+15,ground+1+0.2,minz-2+0.2);

  gl.End();
  for (z=0;z<tracks;++z) {
    gl.PushMatrix();
    gl.Translatef(0,ground,-z-2);
    gl.Rotatef(-90,1,0,0);
    gl.Scalef(3,3,1);
    gl.CallList(head);
    gl.Scalef(0.1,0.1,1);
    gl.Rotatef(-90,0,0,1);
    gl.Translatef(0,-0.5,0);
    Video.drawText(z+1,true);
    gl.PopMatrix();
  }
}

headRadius=0.08;
var head=gl.GenLists(1);
gl.NewList(head, GL_COMPILE);
drawCircle(headRadius,15);
gl.EndList();

var emptyScooter=gl.GenLists(1);
gl.NewList(emptyScooter, GL_COMPILE);
drawEmptyScooter();
gl.EndList();

function Scooter(pos) {
  this.pos=pos;
  this.leg=[0,0];
  this.l=0;
  this.headColor=[1,1,1];
}

Scooter.prototype.paint=function()
{
  var head=0.9-Math.abs(this.leg[0]/10);
  var neck=head-headRadius;
  var body=neck-0.3;
  var knee=body-0.2;
  var bottom=0;
  var tl=0.2;
  var bl=0.2;

  gl.PushMatrix();
  gl.Translatef(this.pos.x,this.pos.y,this.pos.z);

  // head
  gl.PushMatrix();
  gl.PushAttrib(GL_CURRENT_BIT);
  gl.Color3fv(this.headColor);
  gl.Translatef(0,head,0);
  gl.CallList(head);
  gl.PopAttrib();
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
  gl.Vertex3f(maxx/2,0.58,0.08);
  gl.Vertex3f(maxx/2,0.58,0.08);
  gl.Vertex2f(maxx,0.6);

  gl.Vertex2f(0,neck);
  gl.Vertex3f(maxx/2,0.58,-0.08);
  gl.Vertex3f(maxx/2,0.58,-0.08);
  gl.Vertex2f(maxx,0.6);

  gl.End();
  gl.PushMatrix();
  gl.Translatef(maxx,0.09,0);
  gl.CallList(emptyScooter);
  gl.PopMatrix();
  gl.PopMatrix();
}
Scooter.prototype.drive=function(dx)
{
  var i;
  for (i=0;i<2;++i)
    this.leg[i]+=((this.l==i) ? 3*dx : -3*dx);
  if (this.leg[0]>1) this.l=1;
  else if (this.leg[0]<-1) this.l=0;
  this.pos.x+=dx;
}

var sx=1+1/3;
var sy=1;
//Video.setViewportCoordinates({left:0,right:sx,bottom:0,top:sy});

gl.MatrixMode(GL_PROJECTION);
gl.LoadIdentity();
glu.Perspective( 45.0, sx, 0.1, 100.0 );
gl.MatrixMode(GL_MODELVIEW);
gl.LoadIdentity();
gl.Enable(GL_LINE_SMOOTH);
gl.LineWidth(2);

var ground=-0.8;
var scooters=[];
var i;
for (i=0;i<5;++i)
  scooters[i]=new Scooter({x:-1,y:ground,z:-i-2});
var minz=scooters[scooters.length-1].pos.z;


var street=gl.GenLists(1);
gl.NewList(street, GL_COMPILE);
drawStreet(scooters.length);
gl.EndList();



var start,last;
start=last=Timer.getTimeStamp();

frames=0;
fpsText="FPS:";
fpsStamp=start;

while (true) {
  now=Timer.getTimeStamp();
  dt=(now-last)/1000000.0;

  last=now;

  Input.poll();

  gl.Clear(GL_COLOR_BUFFER_BIT);


  // camera movement
  minx=Infinity;
  maxx=-Infinity;
  for (i=0;i<scooters.length;++i) {
    minx=Math.min(scooters[i].pos.x,minx);
    maxx=Math.max(scooters[i].pos.x,maxx);
  }
  gl.LoadIdentity();
  gl.Translatef(-minx-1,0,0);


  // "street"
  gl.PushMatrix();
  gl.Translatef(Math.round(minx/10)*10,0,0);
  gl.CallList(street);
  gl.PopMatrix();

  // paint back to front => we need no depth buffer
  for (i=scooters.length-1;i>=0;--i) {
    scooters[i].paint();
    var ps=1;
    if (players[i]) {
      ps=players[i].speed;
      players[i].speed+=0.1*dt*(1-players[i].speed);
    }
    scooters[i].drive(dt*ps);
  }

  gl.MatrixMode(GL_PROJECTION);
  gl.PushMatrix();
  gl.LoadIdentity();
  gl.Ortho(0,1,0,1,-100,100);
  gl.MatrixMode(GL_MODELVIEW);
  gl.PushMatrix();
  gl.LoadIdentity();
  gl.Translatef(0,1-1/20,0);
  gl.Scalef(1/40,1/30,1);
  Video.drawText(fpsText);
  gl.Translatef(40-morseText.length,0,0);
  Video.drawText(morseText);
  gl.PopMatrix();
  gl.MatrixMode(GL_PROJECTION);
  gl.PopMatrix();
  gl.MatrixMode(GL_MODELVIEW);

  if (now-fpsStamp>5000000) {
    fpsText="FPS: "+Math.round(1000000/((now-fpsStamp)/frames));
    fpsStamp=now;
    frames=0;
  }
  ++frames;
  Video.swapBuffers();
}
