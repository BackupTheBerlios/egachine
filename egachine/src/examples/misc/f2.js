var f=function(i){return function(){return i;}}(10)
print(f()); // 10
var g=function(i){return function(){return ++i;}}(10)

print(g()); // 11
print(g()); // 12
print(g()); // 13


