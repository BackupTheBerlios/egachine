// todo
// this is buggy but funny
// mini synthie/tracker

wavheader="\
\x52\x49\x46\x46\x24\x02\x00\x00\x57\x41\x56\x45\x66\x6d\x74\x20\x10\
\x00\x00\x00\x01\x00\x01\x00\x44\xac\x00\x00\x44\xac\x00\x00\x01\x00\
\x08\x00\x64\x61\x74\x61\x00\x02\x00\x00";

sos="c c c aaaa aaaa aaaa c c c     ";
ref="cdefggaaaagaaaagffffeeaaaac";
ref2="cdefggaaacccagaaaaccc  gffffeeaaaac   e";
loop="c c c aaa aaa aaa c c c    ";
music=sos+sos+sos+ref+ref+ref+ref+loop+sos+sos+sos
+ref2+ref2+sos+loop+sos+sos+ref+ref+ref+loop+ref2+ref2+sos+loop;
cols=[[1,0,0,1],[0,1,0,1],[0,0,1,1],[1,1,1,1],[1,0.5,0,1],[0,0.5,1,1]];

function createSample(name,freq,volume) {
  var wav=wavheader;
  m=Math.floor(4*freq/(44100/512));
  for (i=0;i<512;++i)
    wav+=String.fromCharCode(Math.sin(m*Math.PI*i/512)*127*volume+127);
  EGachine.addResource(({name:name, size:wav.length, data:Base64.encode(wav)}));
}

function playSample(name,sec) {
  rep=Math.floor(sec*44100/512);
  Audio.playSample(name,rep);
}

volume=0.5;
createSample("a",440,volume);
createSample("g",396,volume);
createSample("f",352,volume);
createSample("e",330,volume);
createSample("d",297,volume);
createSample("c",264,volume);

Video.setViewportCoords({left:0,right:4/3,bottom:0,top:1});
Video.translate(4/3/2,0.5);
Video.rotate(10);

// (single track) tracker
var note=0;
while (true) {
  var pton=music.charAt((note-1)>=0 ? (note-1) : (music.length-1));
  var ton=music.charAt(note);
  if ((ton!=" ")&&(ton!=pton)) {
    var c=1,nnote=note;
    while(music.charAt((nnote=(nnote+1<music.length ? nnote+1 : 0)))==ton) ++c;
    playSample(ton,0.1*c);
    //    Video.setClearColor(cols[note%cols.length]);
    //    Video.clear();
    for (i=1;i<6;++i) {
      Video.setColor4v(cols[(note+i)%cols.length]);
      Video.drawRectangle(0.8-i/8,0.8-i/8);
      Video.rotate(i/2);
    }
    Video.swapBuffers();
  }
  Timer.uSleep(100000);
  if (++note>=music.length) note=0;
  Input.poll();
}
