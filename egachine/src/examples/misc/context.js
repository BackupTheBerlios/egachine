/* DONTINSTALL

   scheme:
   guile> (define x ((lambda(text) (lambda() text)) "hello"))
   guile> (source x)
   (lambda () text)
   guile> x
   #<procedure x ()>
   guile> (x)
   "hello"
*/

// first test
print ("First test output");
x=function(text){return function(){print(text);};}("hello world");
x(); // will print hello world as expected
foo={x:1,y:2}
foo.watch("x",x);
foo.x=10; // now will print hello world

// second test
print ("Second test output");
x=function(text){
	return function(){
		print("this:"+this);
		print("this.foomem:"+this.foomem);
		print("text:"+text);
	};
}("hello world");
x();
foo={x:1,foomem:2}
foo.watch("x",x);
foo.x=10;
