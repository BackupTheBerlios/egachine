(function(sg){
  var Video=ejs.ModuleLoader.get("Video");

  // hack currently needed for the distributed scene graph
  if (typeof adjCons == "undefined")
    adjCons=function(func){return func;};

  // vector object
  sg.V2D=adjCons(function(x,y){
			  this.x=x;
			  this.y=y;
			});
  sg.V2D.middle=function(p1,p2) {
    return p1.add(p2).scale(0.5);
  }
  sg.V2D.prototype.clone=function()
    {
      return new sg.V2D(this.x,this.y);
    };
  sg.V2D.prototype.add=function(v)
    {
      return new sg.V2D(this.x+v.x,this.y+v.y);
    };
  sg.V2D.prototype.inc=function(v)
    {
      this.x+=v.x;
      this.y+=v.y;
      return this;
    };
  sg.V2D.prototype.sub=function(v)
    {
      return new sg.V2D(this.x-v.x,this.y-v.y);
    };
  sg.V2D.prototype.dec=function(v)
    {
      this.x-=v.x;
      this.y-=v.y;
      return this;
    };
  sg.V2D.prototype.scale=function(s)
    {
      return new sg.V2D(this.x*s,this.y*s);
    };
  sg.V2D.prototype.rot90=function()
    {
      return new sg.V2D(this.y,-this.x);
    };
  sg.V2D.prototype.length=function()
    {
      return Math.sqrt(this.dot(this));
    };
  sg.V2D.prototype.normalized=function()
    {
      return this.scale(1/this.length());
    };
  sg.V2D.prototype.dot=function(v)
    {
      return this.x*v.x+this.y*v.y;
    };

  // degrees object
  // this is mainly to get reference semantic / wrap primitive type
  sg.Degrees=adjCons(function (deg){
			      this.value=deg;
			    });
  sg.Degrees.prototype.add=function(deg)
    {
      return new sg.Degrees(this.value+deg);
    };
  sg.Degrees.prototype.inc=function(deg)
    {
      this.value+=deg;
      return this;
    };
  sg.Degrees.prototype.sub=function(deg)
    {
      return new sg.Degrees(this.value-deg);
    };
  sg.Degrees.prototype.dec=function(deg)
    {
      this.value-=deg;
      return this;
    };

  // Node object
  // curently prototype object of all nodes in the scenegraph
  // (prototype of the prototypes)
  sg.Node=adjCons(function() {});
  sg.Node.prototype.paint=function(time){
    var i;
    if (this.children)
      for (i=0;i<this.children;++i)
	this[i].paint(time);
  };
  // deprecated
  sg.Node.prototype.step=function(dt){
    var i;
    if (!this.children) return;
    for (i=0;i<this.children;++i)
      this[i].step(dt);
  };
  sg.Node.prototype.add=function(n){
    if (!this.children) this.children=0;
    this[this.children++]=n;
    return this;
  };

  // derived object Rotate
  sg.Rotate=adjCons(function (degrees) {
			     this.degrees=degrees;
			   });
  sg.Rotate.prototype=new sg.Node();
  sg.Rotate.prototype.paint=function(time){
    Video.pushMatrix();
    Video.rotate(this.degrees.value);
    //  Node.prototype.paint.call(this,time);
    Node.prototype.paint.call(this,time);
    Video.popMatrix();
  };

  // derived object Texture
  sg.Texture=adjCons(function(resname){
			      this.resname=resname;
			    });
  sg.Texture.prototype=new sg.Node();
  sg.Texture.prototype.paint=function(time){
    Video.drawTexture(Video.getTextureID(this.resname));
    //  Node.prototype.paint.call(this,time);
    sg.Node.prototype.paint.call(this,time);
  };

  // derived object Scale
  sg.Scale=adjCons(function(v) {
			    this.v=v;
			  });
  sg.Scale.prototype=new sg.Node();
  sg.Scale.prototype.paint=function(time){
    Video.pushMatrix();
    Video.scale(this.v.x,this.v.y);
    sg.Node.prototype.paint.call(this,time);
    Video.popMatrix();
  };

  // derived object Translate
  sg.Translate=adjCons(function(v) {
				this.v=v;
			      });
  sg.Translate.prototype=new sg.Node();
  sg.Translate.prototype.paint=function(time){
    Video.pushMatrix();
    Video.translate(this.v.x,this.v.y);
    sg.Node.prototype.paint.call(this,time);
    Video.popMatrix();
  };

  // derived object Sprite
  sg.Sprite=adjCons(function(resname,size,pos,degrees) {
			     this.size=size;
			     this.pos=pos;
			     this.degrees=degrees;
			     this.resname=resname;
			   });
  sg.Sprite.prototype=new sg.Node();
  sg.Sprite.prototype.paint=function(time){
    Video.pushMatrix();
    Video.translate(this.pos.x,this.pos.y);
    if (this.degrees) Video.rotate(this.degrees.value);
    Video.pushMatrix();
    Video.scale(this.size.x,this.size.y);
    Video.drawTexture(Video.getTextureID(this.resname));
    Video.popMatrix();
    sg.Node.prototype.paint.call(this,time);
    Video.popMatrix();
  };

  // derived object Color
  sg.Color=adjCons(function(r,g,b,a) {
			    if (r.length)
			      this.c=r;
			    else
			      this.c=[r,g,b,a];
			  });
  sg.Color.prototype=new sg.Node();
  sg.Color.prototype.paint=function(time){
    Video.pushColor();
    Video.setColor4v(this.c);
    sg.Node.prototype.paint.call(this,time);
    Video.popColor();
  };

  // derived object Text
  sg.Text=adjCons(function(text,hcenter,vcenter) {
			   this.text=text;
			   this.hcenter=hcenter;
			   this.vcenter=vcenter;
			 });
  sg.Text.prototype=new sg.Node();
  sg.Text.prototype.paint=function(time){
    Video.drawText(this.text,this.hcenter,this.vcenter);
    sg.Node.prototype.paint.call(this,time);
  };

  // derived object Quad
  sg.Quad=adjCons(function(size,pos,degrees) {
			   this.size=size;
			   this.pos=pos;
			   this.degrees=degrees;
			 });
  sg.Quad.prototype=new sg.Node();
  sg.Quad.prototype.paint=function(time){
    Video.pushMatrix();
    if (this.pos) Video.translate(this.pos.x,this.pos.y);
    if (this.degrees) Video.rotate(this.degrees.value);
    Video.drawQuad(this.size.x,this.size.y);
    sg.Node.prototype.paint.call(this,time);
    Video.popMatrix();
  };

  // derived object Mover (deprecated)
  sg.Mover=adjCons(function(speed, rotspeed) {
			    this.speed=speed;
			    this.rotspeed=rotspeed;
			    this.time=0;
			    this.last=0;
			  });
  sg.Mover.prototype=new sg.Node();
  sg.Mover.prototype.step=function(dt){
    var i,ct=this.time+dt;
    if (ct-this.last<2) {
      dontwatch=true;
      this.time=ct;
    }else{
      this.time=ct;
      this.last=this.time;
    }
    if (this.children)
      for (i=0;i<this.children;++i){
	this[i].pos.x+=this.speed.x*dt;
	this[i].pos.y+=this.speed.y*dt;
	if (this.rotspeed) {
	  this[i].degrees.value+=this.rotspeed.value*dt;
	  if (this[i].degrees.value>360) this[i].degrees.value-=360;
	  if (this[i].degrees.value<0) this[i].degrees.value+=360;
	}
      }
    dontwatch=false;
    sg.Node.prototype.step.call(this,dt);
  };
 })(this);
