(function(Video){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsvideo.la");
  if (!fname) throw new Error("Could not find module: 'ejsvideo.la'");
  ejs.ModuleLoader.loadNative.call(Video,"ejsvideo",fname.substring(0,fname.lastIndexOf(".")));

  // get gl module
  var gl=ejs.ModuleLoader.get("gl");

  // map resname to texture id
  Video.textures={};
  // color stack
  Video.colors=[];

  //! map resname to texture id
  /*!
    create texture from resource if necessary
  */
  Video.getTextureID=function(resname){
    var tid=Video.textures[resname];
    if (tid) return tid;
    var res=EGachine.getResource(resname);
    var dec=res.decode();
    tid=Video.textures[resname]=Video.createTexture(dec);
    return tid;
  };

  // we expose a subset of opengl to the video api
  // this subset should be easy to implement
  // all javascript code included with egachine only uses this
  // subset (exception: the opengl examples)
  // 
  // grep -n -r "gl\." .|grep "\.js"|cut -f 1- -d ":" -
  // should reflect this and only list egachine.js and examples
  // in the opengl directory

  if (typeof gl.PushMatrix == 'undefined')
    throw new Error("gl not yet completely loaded");
  
  Video.pushMatrix=gl.PushMatrix;
  Video.popMatrix=gl.PopMatrix;
  Video.getColor4v=function(){return gl.GetFloatv(GL_CURRENT_COLOR)};
  Video.setColor=gl.Color3f;
  Video.setColor4=gl.Color4f;
  Video.setColor4v=gl.Color4fv;

  Video.pushColor=function() {
    Video.colors.push(Video.getColor4v());
  };

  Video.popColor=function() {
    Video.setColor4v(Video.colors.pop());
  };

  Video.rotate=function(d){
    gl.Rotatef(d,0,0,1);
  };

  Video.translate=function(x,y)
    {
      gl.Translatef(x,y,0);
    };

  Video.scale=function(x,y)
    {
      gl.Scalef(x,y,0);
    };

  Video.clear=function()
    {
      gl.Clear(GL_COLOR_BUFFER_BIT);
    };

  Video.setClearColor=function(r,g,b,a)
    {
      if (r.length)
	gl.ClearColor(r[0],r[1],r[2],r[3]);
      else
	gl.ClearColor(r,g,b,a);
    };

  Video.drawLine=function(x1,y1,x2,y2)
    {
      gl.Begin(GL_LINES);
      gl.Vertex2f(x1,y1);
      gl.Vertex2f(x2,y2);
      gl.End();
    };

  Video.drawQuad=function(w,h)
    {
      var w2=w/2;
      var h2=h/2;
      gl.Rectf(-w2,-h2,w2,h2);
    };

  Video.setViewportCoords=function(obj)
    {
      if (typeof obj !="object") throw new Error("object required as argument");
      if (obj.near==undefined) obj.near=-100;
      if (obj.far==undefined) obj.far=100;
      gl.MatrixMode(GL_PROJECTION);		
      gl.LoadIdentity();
      gl.Ortho(obj.left,obj.right,obj.bottom,obj.top,obj.near,obj.far);
      gl.MatrixMode(GL_MODELVIEW);
    };

  Video.getViewport=function()
    {
      // todo perhaps our getViewport should unproject the points?
      return gl.GetIntegerv (GL_VIEWPORT);
    };

  Video.setViewport=function(vec)
    {
      // todo perhaps our setViewport should project the points?
      gl.Viewport(v[0],v[1],v[2],v[3]);
    };
 })(this);
