x=function(){var y={a:5,b:7};y.c={y:y.a,g:y.b};return y;}();
print(x.toSource());
