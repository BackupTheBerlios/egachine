function assert(f){if (!f()) throw f.toSource();};
function print(x) {x=x||"";Stream.stdout.write(x+"\n");};

function crossprod(a,b){
  assert(function(){return ((a.length==3)&&(a.length==b.length));});
  return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];
};

function map(f,v) {
  var i,r=[];
  for (i=0;i<v.length;++i) r[i]=f(v[i]);
  return r;
};

function map2(f,a,b) {
  var i,r=[];
  assert(function(){return (a.length==b.length);});
  for (i=0;i<a.length;++i) r[i]=f(a[i],b[i]);
  return r;
};

function sub(a,b) {return a-b;};

function vsub(a,b) {
  assert(function(){return (a.length==b.length);});
  return map2(sub,a,b);
}

function dotprod(a,b){
  var i,r=0;
  assert(function(){return (a.length==b.length);});
  for (i=0;i<a.length;++i) r+=a[i]*b[i];
  return r;
}

function normal(a,b,c) {
  assert(function(){return (a.length==b.length);});
  assert(function(){return (a.length==c.length);});
  return crossprod(vsub(b,a),vsub(c,a));
}

function loadTexture(fname) {
  // return Video.createTexture(ejs.ModuleLoader.get("File").read(fname).readAll());
  return Video.getTextureID(fname);
};

//! v is point cloud
function delaunay(v) {
  // point cloud (3d)
  var s=[];
  // triangles
  var t=[];
  // new triangle
  var i,j,k,n,invert,l,f,hull;
  // map to Paraboloid
  for (i=0;i<v.length;++i) {
    s.push([v[i][0],v[i][1],v[i][0]*v[i][0]+v[i][1]*v[i][1]]);
  }
  // convex hull (without faces showing up)
  n=s.length;
  for (i=0;i<n;++i){
    for (j=i+1;j<n;++j){
      if (j==i) continue;
      for (k=j+1;k<n;++k){
        if ((k==j)||(k==i)) continue;
        // dreieck ijk
        // normalen vektor
        v=normal(s[i],s[j],s[k]);
        invert=false;
	hull=true;
        l=0;
        while ((l==i)||(l==j)||(l==k)) ++l;
        if (l<n) {
          invert=(dotprod(vsub(s[l],s[i]),v)>0);
          if (invert) v=map(function(x){return -x;},v);// flip face
	  if (v[2]>0) continue;
	  ++l;
	  for (;l<n;++l) {
	    if ((l==i)||(l==j)||(l==k)) continue;
	    if (dotprod(vsub(s[l],s[i]),v)>0) {
	      hull=false;
	      break;
	    }
	  }
	  if (hull) {
	    if (!invert)
	      t.push([i,j,k]);
	    else
	      t.push([k,j,i]);
	  }
	}
      }
    }
  }

  /*
  // output for geomview (http://www.geomview.org/)
  print("OFF");
  f=t.length;
  print(""+s.length+" "+f+" "+(f*3)+"\n");
  // todo: z=0 (but it is nice)
  for (i=0;i<n;++i) print(""+s[i][0]+" "+s[i][1]+" "+s[i][2]);
  print();
  for (i=0;i<f;++i) print("3 "+t[i][0]+" "+t[i][1]+" "+t[i][2]);
  print();
  */
  return t;
}

/*
function Triangulation(s) {
  var i,j,t=delaunay(s);
  this.l=gl.GenLists(1);
  gl.NewList(this.l, GL_COMPILE);
  gl.Begin(GL_TRIANGLES);
  for (i=0;i<t.length;++i) {
    gl.Color4f(rnd(0.3,1),rnd(0.3,1),rnd(0.3,1),1);
    for (j=0;j<3;++j)
      gl.Vertex2f(s[t[i][j]][0],s[t[i][j]][1]);
  }
  gl.End();
  gl.EndList();
};

Triangulation.prototype.paint=function(time){
  gl.Disable(GL_TEXTURE_2D);
  gl.CallList(this.l);
  sg.Node.prototype.paint.call(this,time);
};
*/

function pointCloud() {
  var i,s=[];
  var l=-0.5,r=0.5;
  for (i=0;i<7;i++)
    s.push([rnd(l+0.1,r-0.1),rnd(l+0.1,r-0.1)]);
  s.push([l,l]);
  s.push([l,r]);
  s.push([r,l]);
  s.push([r,r]);
  return s;
};

function Morpher(fname) {
  var i;
  var s=pointCloud();
  var t=delaunay(s);
  
  this.tid=loadTexture(fname);
  this.paintTriangle=[];
  for (i=0;i<t.length;++i) {
    this.paintTriangle[i]=(function(t,dr){
			     return function(time){
			       var x,y,j,st=Math.sin(time/2000000)+1;
			       gl.Rotatef(dr*st,0,0,1);
			       gl.Begin(GL_TRIANGLES);
			       for (j=0;j<3;++j) {
				 x=s[t[j]][0];
				 y=s[t[j]][1];
				 gl.TexCoord2f(x+0.5,1-(y+0.5));
				 gl.Vertex2f(x,y);
			       }
			       gl.End();
			     };
			   })(t[i],rnd(-90,90));
  };
};

Morpher.prototype.paint=function(time){
  var s=this.s;
  var t=this.t;
  var i,j,x,y;
  gl.Enable(GL_TEXTURE_2D);
  gl.BindTexture(GL_TEXTURE_2D,this.tid);
  for (i=0;i<this.paintTriangle.length;++i) {
    gl.PushMatrix();
    this.paintTriangle[i](time);
    gl.PopMatrix();
  }
};

function rnd(min,max) {
  return Math.random()*(max-min)+min;
};

function init() {
  var viewport=Video.getViewport();
  var sx=viewport[2];
  var sy=viewport[3];
  // array - with current state of input devices
  var joypad=[];
  var pos=new sg.V2D(sx/2,sy/2);

  /*
  var i,j,s=pointCloud();

  s=[];
  for (i=l;i<=r;i+=(r-l)/2)
    for (j=l;j<=r;j+=(r-l)/2)
      s.push([i,j]);
  */

  EGachine.sceneGraph=new sg.Node()
    .add(new sg.Translate(pos)
	 .add(new sg.Scale(new sg.V2D(sx,sy))
	      //	      .add(new Triangulation(s))
	      .add(new Morpher("wheel.jpg"))
	      ));

  Input.addDevListener(function(i){
			 joypad[i.dev]={x:i.x, y:i.y, buttons:i.buttons};
		       });

  EGachine.step(function(dt){
		  // move quad
		  var joy=joypad[0];
		  if (joy) {
		    pos.x+=joy.x*sx/2*dt;
		    pos.y+=joy.y*sy/2*dt;
		  }
		});
};

EGachine.addResource(
({name:"wheel.jpg", size:25509, z:true, data:"\
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\
RH//ZCgUDwg==\
"}));

init();
