file=argv[0];

function test(func){
  try{
    func(file);
    throw new Error("failed");
  }catch(error){
    // todo: this is bad
    if (error.message.indexOf("Security")<0)
      throw new Error(error);
  }
}

if (typeof load == 'function') test(load);
test(ejs.load);
test(ejs.ModuleLoader.load);
test(ejs.ModuleLoader.loadUntrusted);

