viewport=Video.getViewport();
width=viewport[2];
height=viewport[3];
loops=0;
numEvents=0;
maxEvents=0;
quit=false;
Stream=ejs.ModuleLoader.get("Stream");
stderr=Stream.stderr;
stdout=Stream.stdout;
do{
  if (!Input.waitEvent())
    stderr.write("Warning: error while waiting for event");
  events=Input.getEvents();
  maxEvents=Math.max(maxEvents,events.length);
  numEvents+=events.length;
  for (i=0;i<events.length;++i) {
    e=events[i];
    switch(e.type) {
    case Input.QUIT:
      quit=true;
      break;
    case Input.KEYDOWN:
      if (e.sym==Input.KEY_ESCAPE)
	quit=true;
      else if ( (e.sym==Input.KEY_RETURN) && (e.mod & Input.KMOD_ALT))
	Video.toggleFullscreen();
      break;
    case Input.MOUSEMOTION:
      if (i+1==events.length) {
	Video.clear();
	Video.pushMatrix();
	Video.translate(e.x,height-e.y);
	Video.setColor(1,1,1);
	Video.drawQuad(width/20,width/20);
	Video.setColor(0,0,0);
	Video.drawQuad(width/40,width/40);
	Video.popMatrix();

	Video.setColor(1,1,1);
	Video.pushMatrix();
	Video.translate(e.x,0);
	Video.drawLine(0,0,0,height);
	Video.popMatrix();
	Video.pushMatrix();
	Video.translate(0,height-e.y);
	Video.drawLine(0,0,width,0);
	Video.popMatrix();
	Video.swapBuffers();
      };
      break;
    default:
      stdout.write(e.toSource()+"\n");
    };
  };
  ++loops;
}while(!quit);
stdout.write("Events processed:"+numEvents+" (in "+loops+" loops)\n");
stdout.write("Max. events in queue:"+maxEvents+"\n");
stdout.write("Avg. events in queue:"+(numEvents/loops)+"\n");

