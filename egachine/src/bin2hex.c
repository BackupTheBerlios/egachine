#include <stdio.h>

int main( int argc, const char * argv [] )
{
  const char * varname;
  int i = 0;
  int c;
  int id = 0;

  printf( "var x = \n'");

  while ( ( c = getchar( ) ) != EOF )
    {
      if ( i != 0 && i % 10 == 0 )
	printf( "'\n+'" );
      printf( "\\x%02lx", c & 0xFFl );
      i++;
    }
  printf( "';\nprint(x.toSource());");
  return 0;
}
