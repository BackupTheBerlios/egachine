#!/usr/bin/ejs
// minimalistic interactive shell example
if (typeof ejs == "undefined") throw "Example must be run by ejs";
var Stream=ejs.ModuleLoader.get("Stream");
var util=ejs.ModuleLoader.get("util");
util.ieval.call(this,Stream.stdin,Stream.stdout,Stream.stderr);
