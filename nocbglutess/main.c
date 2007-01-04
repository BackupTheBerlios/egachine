/*! simple test of nocbglutess */

#include "nocbglutess.h"
#include <stdio.h>
#include "GL/glu.h"
#include <assert.h>

void
printv(const GLdouble v[3])
{
  if (v)
    printf("(%0.0f %0.0f %0.0f)", v[0], v[1], v[2]);
  else
    printf("(NULL)");
}

unsigned
printg(unsigned contours, const unsigned* contourVertices,
	  const GLdouble* vertices)
{
  unsigned i,j,total=0;
  for (i=0;i<contours;++i) {
    for (j=0;j<contourVertices[i];++j,++total) {
      printf(" %d=", total);
      printv(&vertices[total*3]);
    }
    printf("|");
  }
  return total;
}

/* example */
void
tessPolygonContours(unsigned contours, const unsigned* contourVertices,
		    const GLdouble* vertices)
{
  unsigned i,j;
  struct nocb_GluTessRes* res=nocb_gluTessPolygonContours(contours, contourVertices, vertices);

  /* print result */

  /* info about new vertices */
  unsigned* data=res->data;
  for (i=0;i<res->newVerts;++i) {
    printf("\nNew Vertex: ");
    printv(res->newVertsInfo[i].pos);
    printf(" from ");
    for (j=0;j<4;++j) {
      printf("+");
      printf("%d",res->newVertsInfo[i].src[j]);
      printf("*%0.2f", res->newVertsInfo[i].weight[j]);
    }
  }

  /* this would be the code to draw the result */
  for (i=0;i<res->geoms;++i) {
    printf("\nglBegin(%d): vertex indices:", res->mode[i]);
    for (j=0;j<res->vertices[i];++j) {
      assert(*data<res->firstNewVertex+res->newVerts);
      printf(" %d",*data);
      if (*data<res->firstNewVertex)
	/* to lookup the vertex:
	   printv(&vertices[*data*3]); */
	;
      else{
	/* this is a newly created vertex
	   (you probably would interpolate some attributes here */
	printf("[NEW:");
	/* to lookup the vertex: */
	printv(res->newVertsInfo[*data-res->firstNewVertex].pos);
	printf("]");
      }
      data++;
    }
  }
  printf("\n");

  nocb_gluTessRes_delete(res);
  res=0;
  printf("%s\n", gluErrorString(glGetError()));
}

void
tessPolygon(unsigned vertices, const GLdouble* data)
{
  tessPolygonContours(1, &vertices, data);
}

int
main(int argc, char** argv)
{
  GLdouble quadAndTriangle[]={0,0,0,
			      0,3,0,
			      3,3,0,
			      3,0,0,
			      1,1,0,
			      1,2,0,
			      2,2,0,
			      2,1,0};
  const GLdouble* quad=quadAndTriangle;
  const GLdouble* triangle=quadAndTriangle+3*4;
  unsigned numVert[2]={4,3};

  printf("triangle:");
  tessPolygon(3, triangle);

  printf("\nquad:");
  tessPolygon(4, quad);

  printf("\nquadAndTriangle:");
  tessPolygonContours(2, numVert, quadAndTriangle);

  GLdouble butterfly[]={0,0,0,
			2,2,0,
			2,0,0,
			0,2,0};
  printf("\nbutterfly:");
  tessPolygon(4, butterfly);

  GLdouble bug[]={0,0,0,
		  2,2,0,
		  2,0,0,
		  0,2,2};
  printf("\nbug in mesa libglu?: (null pointer and error callback is not called)\n");
  tessPolygon(4, bug);
  
  return 0;
}
