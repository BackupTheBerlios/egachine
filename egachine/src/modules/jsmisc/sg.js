(function(sg){
  var cons;

  // hack currently needed for the distributed scene graph
  if (typeof adjCons == "undefined")
    cons=function(func){return func;};
  else
    cons=adjCons;

  // vector object
  sg.V2D=cons(function(x,y){
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
  sg.Degrees=cons(function (deg){
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
  sg.Node=cons(function() {});
  sg.Node.prototype.paint=function(time){
    var i;
    if (this.children)
      for (i=0;i<this.children;++i)
	this[i].paint(time);
  };
  sg.Node.prototype.add=function(n){
    if (!this.children) this.children=0;
    this[this.children++]=n;
    return this;
  };

  // derived object Rotate
  sg.Rotate=cons(function (degrees) {
			     this.degrees=degrees;
			   });
  sg.Rotate.prototype=new sg.Node();
  sg.Rotate.prototype.paint=function(time){
    sg.Video.pushMatrix();
    sg.Video.rotate(this.degrees.value);
    //  Node.prototype.paint.call(this,time);
    Node.prototype.paint.call(this,time);
    sg.Video.popMatrix();
  };

  // derived object Texture
  sg.Texture=cons(function(resname){
			      this.resname=resname;
			    });
  sg.Texture.prototype=new sg.Node();
  sg.Texture.prototype.paint=function(time){
    sg.Video.drawTexture(sg.Video.getTextureID(this.resname));
    //  Node.prototype.paint.call(this,time);
    sg.Node.prototype.paint.call(this,time);
  };

  // derived object Scale
  sg.Scale=cons(function(v) {
			    this.v=v;
			  });
  sg.Scale.prototype=new sg.Node();
  sg.Scale.prototype.paint=function(time){
    sg.Video.pushMatrix();
    sg.Video.scale(this.v.x,this.v.y);
    sg.Node.prototype.paint.call(this,time);
    sg.Video.popMatrix();
  };

  // derived object Translate
  sg.Translate=cons(function(v) {
				this.v=v;
			      });
  sg.Translate.prototype=new sg.Node();
  sg.Translate.prototype.paint=function(time){
    sg.Video.pushMatrix();
    sg.Video.translate(this.v.x,this.v.y);
    sg.Node.prototype.paint.call(this,time);
    sg.Video.popMatrix();
  };

  // derived object Sprite
  sg.Sprite=cons(function(resname,size,pos,degrees) {
			     this.size=size;
			     this.pos=pos;
			     this.degrees=degrees;
			     this.resname=resname;
			   });
  sg.Sprite.prototype=new sg.Node();
  sg.Sprite.prototype.paint=function(time){
    sg.Video.pushMatrix();
    sg.Video.translate(this.pos.x,this.pos.y);
    if (this.degrees) sg.Video.rotate(this.degrees.value);
    sg.Video.pushMatrix();
    sg.Video.scale(this.size.x,this.size.y);
    sg.Video.drawTexture(sg.Video.getTextureID(this.resname));
    sg.Video.popMatrix();
    sg.Node.prototype.paint.call(this,time);
    sg.Video.popMatrix();
  };

  // derived object Color
  sg.Color=cons(function(r,g,b,a) {
			    if (r.length)
			      this.c=r;
			    else
			      this.c=[r,g,b,a];
			  });
  sg.Color.prototype=new sg.Node();
  sg.Color.prototype.paint=function(time){
    sg.Video.pushColor();
    sg.Video.setColor4v(this.c);
    sg.Node.prototype.paint.call(this,time);
    sg.Video.popColor();
  };

  // derived object Text
  sg.Text=cons(function(text,hcenter,vcenter) {
			   this.text=text;
			   this.hcenter=hcenter;
			   this.vcenter=vcenter;
			 });
  sg.Text.prototype=new sg.Node();
  sg.Text.prototype.paint=function(time){
    sg.Video.drawText(this.text,this.hcenter,this.vcenter);
    sg.Node.prototype.paint.call(this,time);
  };

  // derived object Quad
  sg.Quad=cons(function(size,pos,degrees) {
			   this.size=size;
			   this.pos=pos;
			   this.degrees=degrees;
			 });
  sg.Quad.prototype=new sg.Node();
  sg.Quad.prototype.paint=function(time){
    sg.Video.pushMatrix();
    if (this.pos) sg.Video.translate(this.pos.x,this.pos.y);
    if (this.degrees) sg.Video.rotate(this.degrees.value);
    sg.Video.drawQuad(this.size.x,this.size.y);
    sg.Node.prototype.paint.call(this,time);
    sg.Video.popMatrix();
  };

 })(this);
