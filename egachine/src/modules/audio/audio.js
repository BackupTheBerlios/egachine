(function(Audio){
  // module configuration options
  var getResource=ejs.config.Audio.getResource;

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsaudio.la");
  if (!fname) throw new Error("Could not find module: 'ejsaudio.la'");
  ejs.ModuleLoader.loadNative.call(Audio,"ejsaudio",fname.substring(0,fname.lastIndexOf(".")));

  Audio._playMusic=Audio.playMusic;
  Audio.playMusic=function(resname) {
    var res=getResource(resname);
    var dec=res.decode();
    Audio._playMusic(dec);
  };

  Audio.samples={};

  Audio._loadSample=Audio.loadSample;
  Audio._playSample=Audio.playSample;

  Audio.loadSample=function(resname) {
    var res=getResource(resname);
    var dec=res.decode();
    var sid=Audio._loadSample(dec);
    Audio.samples[resname]=sid;
    return sid;
  };

  Audio.playSample=function(resname,repeat) {
    var sid=Audio.samples[resname];
    if (sid==undefined)
      sid=Audio.loadSample(resname);
    if (repeat==undefined)
      repeat=0;
    return Audio._playSample(sid,repeat);
  };
 })(this);


