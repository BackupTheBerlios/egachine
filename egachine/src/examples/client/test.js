// simple test program using the egachine script library

EGachine.addResource("racket", '\
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\
');

EGachine.addResource("alpha", '\
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\
aQAAAABJRU5ErkJggg==\
');

EGachine.addResource("solid", '\
iVBORw0KGgoAAAANSUhEUgAAACwAAAAhBAMAAABQNFTsAAAAHlBMVEUPBwIekhqiVjYrlxrG\
SgIXH9F6dkdyiWO9RwL4+fgxF8yBAAAAAWJLR0QAiAUdSAAAAAlwSFlzAAALEgAACxIB0t1+\
/AAAAAd0SU1FB9QEBw4KN2XgCm8AAADgSURBVHjabc8xC8IwEAXgDCpdi7gLBcGxdHCvR3V0\
acVNRVucxSFbkQ6Zq1Du35o05pq2eePH4yXHDozCkcKsHN08c/OUO5k5WIhyzJ6oQJQjFiCz\
Xw/YqxTDbsBtGSAesC4D0Apbqg1F7zy/dlz/OS5Cf14Q41nzypeZd9zc2QQgCRX7G2KZL0DQ\
KtVbrgEumhdpwYkbiPVGKpNxw/ifXihOj8S3hMoy3HC97cptXTMGNmfEjbWhVgyHTsYeP4if\
Np+IP6GzjZGbZd3xpFyPzL9TtBjxNbpSh6b7/DK34w/kRAwGc7/F2AAAAABJRU5ErkJggg==\
');

EGachine.addResource("back", '\
iVBORw0KGgoAAAANSUhEUgAAAQAAAAABCAMAAAAGh4BFAAADAFBMVEUCAAAEAAAGAAAIAAAK\
AAAMAAAOAAAQAAASAAAUAAAWAAAYAAAaAAAcAAAeAAAgAAAiAAAkAAAmAAAoAAAqAAAsAAAu\
AAAwAAAyAAA0AAA2AAA4AAA6AAA8AAA+AABAAABCAABEAABGAABIAABKAABMAABOAABQAABS\
AABUAABWAABYAABaAABcAABeAABgAABiAABkAABmAABoAABqAABsAABuAABwAAByAAB0AAB2\
AAB4AAB6AAB8AAB+AACAAACCAACEAACGAACIAACKAACMAACOAACQAACRAACSAACUAACWAACY\
AACaAACcAACeAACgAACiAACkAACmAACoAACqAACsAACuAACwAACyAAC0AAC2AAC4AAC6AAC8\
AAC+AADAAADCAADEAADGAADIAADKAADMAADOAADQAADSAADUAADWAADYAADaAADcAADeAADg\
AADiAADkAADmAADoAADqAADsAADuAADwAADyAAD0AAD2AAD4AAD6AAD8AAD+AAD/AAD/AgD/\
BAD/BgD/CAD/CgD/DAD/DgD/EAD/EgD/FAD/FgD/GAD/GgD/HAD/HgD/IAD/IgD/JAD/JgD/\
KAD/KgD/LAD/LgD/MAD/MgD/NAD/NgD/OAD/OgD/PAD/PgD/QAD/QgD/RAD/RgD/SAD/SgD/\
TAD/TgD/UAD/UgD/VAD/VgD/WAD/WgD/XAD/XgD/YAD/YgD/ZAD/ZgD/aAD/agD/bAD/bQD/\
bgD/cAD/cgD/dAD/dgD/eAD/egD/fAD/fgD/gAD/ggD/hAD/hgD/iAD/igD/jAD/jgD/kAD/\
kgD/lAD/lgD/mAD/mgD/nAD/ngD/oAD/ogD/pAD/pgD/qAD/qgD/rAD/rgD/sAD/sgD/tAD/\
tgD/uAD/ugD/vAD/vgD/wAD/wgD/xAD/xgD/yAD/ygD/zAD/zgD/0AD/0gD/1AD/1gD/2AD/\
2gD/3AD/3gD/4AD/4gD/5AD/5gD/6AD/6gD/7AD/7gD/8AD/8gD/9AD/9gD/+AD/+gD//AA5\
e2/hAAAAAWJLR0QAiAUdSAAAAAlwSFlzAAALEgAACxIB0t1+/AAAAAd0SU1FB9QEBw46LDDD\
9XAAAAEMSURBVHjaAQEB/v4AAAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIj\
JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZ\
WltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P\
kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TF\
xsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7\
/P3+/633f4EuF7rAAAAAAElFTkSuQmCC\
');

// handle input events
handleInput=function(i){
  print("got input:");
  for (var x in i){print(x+"="+i[x]);}
}


function run(graph) {
  var start=Timer.getTimeStamp();
  for (var i=0;i<frames;++i) {
    // get input events
    Input.poll();
    
    // paint some lines
    Video.setColor(1,1,1,1);
    for (var y=768/2-200;y<768/2+200;y+=20) Video.drawLine(0,y,1024,y);
    //    Video.setColor(1,0,0,1);
    
    // paint scenegraph
    graph.paint();

    Video.pushMatrix();
    Video.scale(64,64);
    Video.drawText(i.convertTo(16,8));
    Video.popMatrix();
  
    // move and rotate sprite
    graph.children[1].pos.x+=0.5;
    graph.children[1].degrees.value-=1;

    //  Timer.uSleep(10);
    Video.swapBuffers();
  }
  // calculate how many frames we painted per second
  print("fps:"+(1000000/((Timer.getTimeStamp()-start)/frames)));
}

frames=1000;
spriteSize=new V2D(100,100);

pos=new V2D(0,768/2);
degrees=new Degrees(0);

// build our scenegraph
root=new Node()
  .addNode(new Sprite("back",new V2D(1024,768),new V2D(1024/2,768/2)))
  .addNode((sprite=new Sprite("solid",spriteSize,pos,degrees)))
  
for (var x=-200;x<=200;x+=100)
  for (var y=-200;y<=200;y+=100)
    sprite.addNode(new Sprite("alpha",spriteSize,new V2D(x,y)));

run(root);

// serialize
start=Timer.getTimeStamp();
//print(root.toSource());
var x=serialize(root);
//print(root.toSource());
print("time to serialize:"+(Timer.getTimeStamp()-start));

//print(x);

// deserialize
start=Timer.getTimeStamp();
rr=deserialize(x);
print("time to deserialize:"+(Timer.getTimeStamp()-start));
//print(rr.toSource());
rr.children[1].pos.x=10;
run(rr);

/*
start=Timer.getTimeStamp();
var z=toSource(root);
print("time for toSource(root):"+(Timer.getTimeStamp()-start));


Video.setColor(1,1,1,1);

rr.paint();
Video.swapBuffers();
//Timer.uSleep(4000000);

*/
