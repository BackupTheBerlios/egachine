viewport=Video.getViewport();
width=viewport[2];
height=viewport[3];
while(true) {
  if (!Input.waitEvent())
    stderr.write("Warning: error while waiting for event");
  events=Input.getEvents();
  for (i=0;i<events.length;++i) {
    e=events[i];
    switch(e.type) {
    case Input.QUIT:
      ejs.exit(true);
    case Input.KEYDOWN:
      if (e.sym==Input.KEY_ESCAPE) ejs.exit(true);
      if ( (e.sym==Input.KEY_RETURN) && (e.mod & Input.KMOD_ALT))
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
	Video.swapBuffers();
      };
      break;
    default:
      stdout.write(e.toSource()+"\n");
    };
  };
};
