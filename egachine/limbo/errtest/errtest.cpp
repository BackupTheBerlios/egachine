#include <iostream>
#include <jsapi.h>

static JSBool
print(JSContext *cx, JSObject *, uintN argc, jsval *argv, jsval *)
{
  if(argc!=1) {
    JS_ReportError(cx,"Wrong number of arguments");
    return JS_FALSE;
  }
  JSString *s=JS_ValueToString(cx, argv[0]);
  if (!s) return JS_FALSE;
  // root the string
  argv[0]=STRING_TO_JSVAL(s);
  char* ctype=JS_GetStringBytes(s);
  if (!ctype) return JS_FALSE;
  unsigned len=JS_GetStringLength(s);
  std::cout.rdbuf()->sputn(ctype,len);
  std::cout << std::endl;
  return JS_TRUE;
}

static
void printError(JSContext *, const char *message, JSErrorReport *report) {
  std::cerr << "JSERROR: "<< (report->filename ? report->filename : "NULL") << ":" << report->lineno << ":\n"
	    << "    " << message << "\n";
  if (report->linebuf) {
    std::string line(report->linebuf);
    if (line[line.length()-1]=='\n') line=line.substr(0,line.length()-1);
    std::cerr << "    \"" << line << "\"\n";
    if (report->tokenptr) {
      int where=report->tokenptr - report->linebuf;
      if ((where>=0)&&(where<80)) {
	std::string ws(where+4,' ');
	std::cerr << ws << "^\n";
      }
    }
  }
  std::cerr << "    Flags:";
  if (JSREPORT_IS_WARNING(report->flags)) std::cerr << " WARNING";
  if (JSREPORT_IS_EXCEPTION(report->flags)) std::cerr << " EXCEPTION";
  if (JSREPORT_IS_STRICT(report->flags)) std::cerr << " STRICT";
  std::cerr << " (Error number: " << report->errorNumber << ")\n";
}

int
main()
{
  JSRuntime *rt;
  JSContext *cx;
  JSObject  *glob;
  if (!(rt = JS_NewRuntime(8L * 1024L * 1024L))) return EXIT_FAILURE;
  if (!(cx = JS_NewContext(rt, 2<<13))) return EXIT_FAILURE;
  if (!(glob = JS_NewObject(cx, NULL, NULL, NULL))) return EXIT_FAILURE;
  if (!JS_InitStandardClasses(cx, glob)) return EXIT_FAILURE;
  JS_SetErrorReporter(cx, printError);
  JS_DefineFunction(cx, glob, "print", print, 2, 0);
  std::string script;
  char c;
  while (std::cin.get(c)) script+=c;
  jsval rval;
  return JS_EvaluateScript(cx, glob, script.c_str(), script.length(), "stdin", 1, &rval)
    ? EXIT_SUCCESS : EXIT_FAILURE;
}
