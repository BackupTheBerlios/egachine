// gerneral purpose client - which connects to egaserver

if (!EGachine.client) throw "This file must be run by egachine";
if (!EGachine.checkVersion(0,0,7)) throw "at least version 0.0.7 required";

var host=(argv.length>=2) ? argv[1] : "localhost";
var port=(argv.length>=3) ? argv[2] : 47000;

// input field
function InputField(text,maxlen)
{
  this.maxlen=maxlen;
  this.done=false;
  this.text=text;
  // register member function as callback => bind member function
  Input.handleChar=function(obj){return function(c){obj.handleChar(c);}}(this);
  Input.charMode(true);
}
InputField.prototype.handleChar=function(c){
  //  print(c.charCodeAt(0));
  switch(c) {
  case "\u0000":
    break;
  case "\u0008":
    // delete
    this.text=this.text.substring(0,this.text.length-1);
    break;
  case "\u000D":
    // enter
    this.done=true;
    break;
  default:
    if (this.text.length<this.maxlen)
      this.text=this.text+c;
  }
}

function connect(host,port) {
  // TODO: we assume 1.333:1, restore settings
  // BUG: not all characters are displayed by current video
  // backend !!!
  // this could be a security problem!!
  // example: connect to sicher.de but really connect to
  // öööösicher.de

  var sx=1+1/3,sy=1;
  var i;
  var welcome="    Welcome to EGachine "+EGachine.version.string+"    ";
  var constr="Connect to";
  var s=welcome+"\n\n"+constr;
  Video.setViewportCoords({left:0,right:sx,bottom:0,top:sy});
  var maxlen=Math.max(welcome.length,constr.length);
  var inputField=new InputField(host+":"+port,maxlen);

  i=0.01;
  var start=Timer.getTimeStamp();
  var last=start;
  var dt;
  var blink=true;
  var blinkStamp=last;
  while (!inputField.done) {
    now=Timer.getTimeStamp();
    dt=(now-last)/1000000.0;
    last=now;

    if (i<sx/maxlen) i+=dt/50;
    Video.clear();
    Video.pushMatrix();
    Video.translate(sx/2,3/4*sy);
    Video.scale(i,i*1.3);
    Video.drawText(s,true);
    Video.translate(0,-3);

    Video.pushMatrix();
    Video.translate(0,0.5);
    Video.pushColor();
    Video.drawQuad(inputField.text.length+2+0.1,1+0.1);
    Video.setColor(0.3,0.3,0.3);
    Video.drawQuad(inputField.text.length+2,1);
    Video.popColor();
    Video.popMatrix();

    Video.drawText(inputField.text,true);
    
    if (now-blinkStamp>300000) {
      blinkStamp=now;
      blink=!blink;
    }

    if (blink) {
      Video.pushMatrix();
      Video.translate(inputField.text.length/2,0);
      Video.drawText("_");
      Video.popMatrix();
    }

    Video.translate(0,-1);
    Video.drawText("\n\n\npress escape to quit",true);
    Video.popMatrix();
    Video.swapBuffers();
    Timer.uSleep(10000);
    Input.poll();
  }
  Input.charMode(false);
  var hp=inputField.text.split(":",2);
  host=hp[0];
  port=hp[1];

  // TODO: in the moment we can't handle errors
  // if we could we could print an error message
  // Note: egachine allows only to try once
  // to establish an outgoing connection for security reasons
  // (perhaps we could allow 2 or 3 trials)
  stream=Net.connect(host,port);
}

function readMsg()
{
  var len=Number("0x"+stream.recv(6));
  var msg=stream.recv(len);
  eval(msg);
}

connect(host,port);
while (true) readMsg();
