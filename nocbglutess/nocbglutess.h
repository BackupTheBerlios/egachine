#ifndef NOCBGLUTESS_H
#define NOCBGLUTESS_H

/*!
  \brief API for glu tesselation without callbacks
  \file nocbglutess.h
  \author Jens Thiele
*/

/*!
  \mainpage 

  \section problem The Problem:
  It is quite difficult to provide language bindings for the opengl utility library (glu) tesselation object because of extensive usage of C callbacks.

  \section solution The Solution:
  Provide procedures not using C callbacks based on glu tesselation to simplify the task:
  - \ref nocb_gluTessPolygon and \ref nocb_gluTessPolygonContours

  \section todos Todo:
  - you don't get the edgeFlag at the moment
  (typically the caller should know the contour anyway?)
  - implement in C completely (for now only the API should be usable in C)
  - why is the error callback not called? mesa glu bug?
  - maybe provide a simpler API for simple polygons (no self-intersection)

  \section example Example:
  A example program:
  \include main.c
  Running the resulting executable produces this output:
  \verbinclude test.out

  \section references References:
  - The opengl utility library (glu): http://www.opengl.org/documentation/specs/
  - Polygon: http://en.wikipedia.org/wiki/Polygon
  - Computational Geometry: http://en.wikipedia.org/wiki/Computational_geometry
  - Polygon triangulation: http://en.wikipedia.org/wiki/Polygon_triangulation
*/

#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

  /*!
    \brief information about newly created vertices (self-intersection)
    \sa man gluTessCallback
  */
  struct nocb_GluCombineInfo
  {
    /*! \brief position of new vertex (intersection point) */
    GLdouble pos[3];
    /*! \brief source vertices (pointing into input data)
      \bug in error cases some might be invalid (bug in libglu implementation of mesa?)
    */
    unsigned src[4];
    /*! \brief weights of four input vertices */
    GLfloat weight[4];
  };

  /*! \brief result type */
  struct nocb_GluTessRes {
    /*!
      \brief number of created vertices
      \sa man gluTessCallback (GLU_TESS_COMBINE)
    */
    unsigned newVerts;
    /*! \brief info about the new vertices */
    struct nocb_GluCombineInfo* newVertsInfo;
    /*! 
      \brief index of first created vertex (if any)
      \note only provided for convenience (index is equal to the total number of input vertices)
    */
    unsigned firstNewVertex;

    /*! \brief number of geometries */
    unsigned geoms;
    /*!
      \brief geometry mode for each geometry
      \sa man glBegin 
    */
    GLenum* mode;
    /*! \brief number of vertices for each geometry */
    unsigned* vertices;

    /*!
      \brief vertex indices of all geometries stored consecutively
      \note indices refer to input data and maybe to the newly created vertices
    */
    unsigned* data;

    /*!
      \brief size of data array
      \note only provided for convenience (could be calculated from geoms and vertices)
    */
    unsigned dataSize;
  };

  /*! \brief free result object */
  void
  nocb_gluTessRes_delete(struct nocb_GluTessRes* res);

  /*!
    \brief tesselate polygon having more than one contour

    The polygon may:
    - be complex having self intersections
    - have more than one contour (have holes)
    
    \param contours number of contours
    \param contourVertices number of vertices for each contour
    \param vertices all vertices of all contours stored consecutively

    \return result object that must be deleted by calling \ref nocb_gluTessRes_delete
  */
  struct nocb_GluTessRes*
  nocb_gluTessPolygonContours(unsigned contours, const unsigned* contourVertices,
				 const GLdouble* vertices);

  /*! \brief tesselate polygon

  The polygon may:
  - be complex having self intersections

  \param vertices number of polygon vertices
  \param data vertices of the polygon (x1,y1,z1, x2,y2,z2, ...)

  \return result object that must be deleted by calling \ref nocb_gluTessRes_delete
  */
  struct nocb_GluTessRes*
  nocb_gluTessPolygon(unsigned vertices, const GLdouble* data);

#ifdef __cplusplus
}
#endif

#endif
