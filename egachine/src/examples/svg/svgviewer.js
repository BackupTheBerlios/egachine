var svgl=ejs.ModuleLoader.get("svgl");

if (argv.length<2) {
  stderr.write("Usage: "+argv[0]+" FILE\n");
  ejs.exit(false);
};

svgl.viewFile(argv[1]);
