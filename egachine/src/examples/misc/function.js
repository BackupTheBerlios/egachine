// DONTINSTALL

// binding member function to function

// first a simple function variable example
function foo(){
  print("foo");
}
// prints foo and foo
function simpleTest(){
  foo();
  var x=foo;
  x();
}
simpleTest();

// now i want to bind a member function to a function variable
// this will not work:
// x=foo.bar; (since "this" will be wrong)

function Foo(bar){
  this.b=bar;
}
Foo.prototype.bar=function()
{
  print (this.b);
}

function test() {
  var foo=new Foo("bar");
  foo.bar(); // print bar

  // bind method - is there a simpler way ????
  var x=function(y){var z=y;return function(){z.bar();}}(foo);

  x(); // print bar
}

test();


function p(s){
  print(s);
}

var x=(function(str){return function(){p(str);};})("hellox");
x();

