#include <GL/gl.h>
#include <GL/glu.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <vector>
#include <map>
#include "nocbglutess.h"

// on windows you might have to define this
#ifndef CALLBACK
#define CALLBACK
#endif

using namespace std;

struct nocb_GluCombineInfoPtr;

//! result using c++ (later converted to C)
/*!
  \note i was to lazy to write this completely in C
*/
struct CppRes
{
  //! new vertices
  vector<nocb_GluCombineInfoPtr*> first;
  //! tesselation result
  vector<pair<GLenum, vector<const GLdouble*> > > second;
  //! error reported by callback (todo: doesn't work?)
  GLenum error;
};

extern "C" {

  /*!
    \brief information about newly created vertices (self-intersection) using pointers
    \sa man gluTessCallback
  */
  struct nocb_GluCombineInfoPtr
  {
    /*! \brief position of new vertex (intersection point) */
    GLdouble pos[3];
    /*! \brief source vertices (pointing into input data)
      \bug in error cases some might be NULL (bug in libglu implementation of mesa?)
    */
    const GLdouble* src[4];
    /*! \brief weights of four input vertices */
    GLfloat weight[4];
  };

  /*! \brief result type using pointers */
  struct nocb_GluTessResPtr {
    /*!
      \brief number of created vertices
      \sa man gluTessCallback (GLU_TESS_COMBINE)
    */
    unsigned newVerts;
    /*! \brief info about the new vertices */
    struct nocb_GluCombineInfoPtr** newVertsInfo;

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
      \brief vertices of all geometries stored consecutively
      \note pointers point to input data and maybe to the newly created vertices
    */
    const GLdouble** data;

    /*!
      \brief size of data array
      \note only provided for convenience (could be calculated from geoms and vertices)
    */
    unsigned dataSize;
  };

  static
  struct nocb_GluTessResPtr*
  nocb_gluTessResPtr_new()
  {
    struct nocb_GluTessResPtr* res=(struct nocb_GluTessResPtr*)malloc(sizeof(struct nocb_GluTessResPtr));
    res->newVerts=0;
    res->newVertsInfo=NULL;
    res->geoms=0;
    res->mode=NULL;
    res->vertices=NULL;
    res->dataSize=0;
    res->data=NULL;
    return res;
  }

  static
  struct nocb_GluTessRes*
  nocb_gluTessRes_new()
  {
    struct nocb_GluTessRes* res=(struct nocb_GluTessRes*)malloc(sizeof(struct nocb_GluTessRes));
    res->newVerts=0;
    res->newVertsInfo=NULL;
    res->geoms=0;
    res->mode=NULL;
    res->vertices=NULL;
    res->dataSize=0;
    res->data=NULL;
    return res;
  }

  void
  nocb_gluTessResPtr_delete(struct nocb_GluTessResPtr* res) 
  {
#define FREE(x) do{				\
      if (x) {					\
	free(x);				\
	x=NULL;					\
      }						\
    }while(0)
    
    assert(res);
    if (res->newVertsInfo) {
      for (unsigned i=0;i<res->newVerts;++i)
	free(res->newVertsInfo[i]);
      free(res->newVertsInfo);
      res->newVertsInfo=NULL;
    }
    FREE(res->mode);
    FREE(res->vertices);
    FREE(res->data);
    free(res);
#undef FREE
  }
  
  static
  void CALLBACK
  myBeginData(GLenum which, void* polygon_data)
  {
    CppRes* cppRes=reinterpret_cast<CppRes*>(polygon_data);
    cppRes->second.push_back(pair<GLenum, vector<const GLdouble*> >(which,vector<const GLdouble*>()));
  }

  static
  void CALLBACK
  myEndData(void* /* polygon_data */)
  {
    //    CppRes* cppRes=reinterpret_cast<CppRes*>(polygon_data);
  }
  
  static
  void CALLBACK
  myVertexData(void* vertex_data, void* polygon_data)
  {
    CppRes* cppRes=reinterpret_cast<CppRes*>(polygon_data);
    assert(!cppRes->second.empty());
    cppRes->second.back().second.push_back(reinterpret_cast<GLdouble*>(vertex_data));
  }

  static
  void CALLBACK
  myCombineData(GLdouble coords[3], void* vertex_data[4],
		GLfloat weight[4], void** outData, void* polygon_data)
  {
    CppRes* cppRes=reinterpret_cast<CppRes*>(polygon_data);
    struct nocb_GluCombineInfoPtr* nv=(struct nocb_GluCombineInfoPtr*)malloc(sizeof(struct nocb_GluCombineInfoPtr));
    for (unsigned i=0;i<3;++i)
      nv->pos[i]=coords[i];
    assert(vertex_data);
    for (unsigned i=0;i<4;++i) {
      nv->src[i]=reinterpret_cast<const GLdouble*>(vertex_data[i]);
      //      assert(nv->src[i]);
    }
    for (unsigned i=0;i<4;++i)
      nv->weight[i]=weight[i];
    GLdouble** out=reinterpret_cast<GLdouble**>(outData);
    *out=nv->pos;
    cppRes->first.push_back(nv);
  }

  static
  void CALLBACK
  myError(GLenum error)
  {
    printf("error: %i\n", error);
  }
  
  static
  void CALLBACK
  myErrorData(GLenum error, void* polygon_data)
  {
    printf("error: %i\n", error);
    CppRes* cppRes=reinterpret_cast<CppRes*>(polygon_data);
    for (unsigned i=0;i<cppRes->first.size();++i)
      free(cppRes->first[i]);
    cppRes->first.clear();
    cppRes->second.clear();
    cppRes->error=error;
  }
}

struct nocb_GluTessResPtr*
nocb_gluTessPolygonContoursPtr(unsigned contours, const unsigned* contourVertices,
			       const GLdouble* vertices)
{
  CppRes cppRes;
  GLUtesselator* tess=gluNewTess();

  typedef void (CALLBACK *cb)(void);
  gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (cb)myBeginData);
  gluTessCallback(tess, GLU_TESS_END_DATA, (cb)myEndData);
  gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (cb)myVertexData);
  gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (cb)myCombineData);
  gluTessCallback(tess, GLU_TESS_ERROR, (cb)myError);
  gluTessCallback(tess, GLU_TESS_ERROR_DATA, (cb)myErrorData);
  
  gluTessBeginPolygon(tess, &cppRes);
  const GLdouble* v=vertices;
  for (unsigned i=0;i<contours;++i) {
    gluTessBeginContour(tess);
    for (unsigned j=0;j<contourVertices[i];++j) {
      gluTessVertex(tess,
		    const_cast<GLdouble*>(v),
		    const_cast<void*>(reinterpret_cast<const void*>(v)));
      v+=3;
    }
    gluTessEndContour(tess);
  }
  gluTessEndPolygon(tess);
  gluDeleteTess(tess);

  struct nocb_GluTessResPtr* res=nocb_gluTessResPtr_new();

  // convert into result suitable for C

  // new vertices

  res->newVerts=cppRes.first.size();
  res->newVertsInfo=(struct nocb_GluCombineInfoPtr**)malloc(res->newVerts*sizeof(struct nocb_GluCombineInfoPtr*));
  for (unsigned i=0;i<cppRes.first.size();++i)
    res->newVertsInfo[i]=cppRes.first[i];

  // geometry

  res->geoms=cppRes.second.size();
  res->mode=(GLenum*)malloc(res->geoms*sizeof(GLenum));
  res->vertices=(unsigned*)malloc(res->geoms*sizeof(unsigned));
  res->dataSize=0;
  for (unsigned i=0;i<cppRes.second.size();++i) {
    res->mode[i]=cppRes.second[i].first;
    res->vertices[i]=cppRes.second[i].second.size();
    res->dataSize+=cppRes.second[i].second.size();
  }
  
  res->data=(const GLdouble**)malloc(res->dataSize*sizeof(GLdouble*));
  unsigned k=0;
  for (unsigned i=0;i<cppRes.second.size();++i)
    for (unsigned j=0;j<cppRes.second[i].second.size();++j)
      res->data[k++]=cppRes.second[i].second[j];
  return res;
}

struct nocb_GluTessResPtr*
nocb_gluTessPolygonPtr(unsigned vertices, const GLdouble* data)
{
  return nocb_gluTessPolygonContoursPtr(1, &vertices, data);
}

void
nocb_gluTessRes_delete(struct nocb_GluTessRes* res) 
{
#define FREE(x) do{				\
    if (x) {					\
      free(x);					\
      x=NULL;					\
    }						\
  }while(0)
  assert(res);
  FREE(res->newVertsInfo);
  FREE(res->mode);
  FREE(res->vertices);
  FREE(res->data);
  free(res);
#undef FREE
}

struct nocb_GluTessRes*
nocb_gluTessPolygonContours(unsigned contours, const unsigned* contourVertices,
			    const GLdouble* vertices)
{
  struct nocb_GluTessResPtr* resP=nocb_gluTessPolygonContoursPtr(contours, contourVertices,
								 vertices);
  // map pointers to indices
  map<const GLdouble*, unsigned> idx;

  unsigned cIdx=0;
  for (unsigned i=0;i<contours;++i)
    for (unsigned j=0;j<contourVertices[i];++j) {
      assert(idx.find(&vertices[cIdx*3])==idx.end());
      idx[&vertices[cIdx*3]]=cIdx++;
    }

  unsigned inputVertices=cIdx;
  
  for (unsigned i=0;i<resP->newVerts;++i) {
    assert(idx.find(static_cast<const GLdouble*>(resP->newVertsInfo[i]->pos))==idx.end());
    idx[static_cast<const GLdouble*>(resP->newVertsInfo[i]->pos)]=cIdx++;
  }
  
  struct nocb_GluTessRes* res=nocb_gluTessRes_new();
  res->newVerts=resP->newVerts;
  res->newVertsInfo=(struct nocb_GluCombineInfo*)malloc(res->newVerts*sizeof(struct nocb_GluCombineInfo));
  for (unsigned i=0;i<resP->newVerts;++i) {
    for (unsigned j=0;j<3;++j)
      res->newVertsInfo[i].pos[j]=resP->newVertsInfo[i]->pos[j];
    for (unsigned j=0;j<4;++j) {
      res->newVertsInfo[i].weight[j]=resP->newVertsInfo[i]->weight[j];
      // todo: bug in mesa glu might be a null pointer
      // assert(idx.find(static_cast<const GLdouble*>(resP->newVertsInfo[i]->src[j]))!=idx.end());
      res->newVertsInfo[i].src[j]=idx[static_cast<const GLdouble*>(resP->newVertsInfo[i]->src[j])];
    }
  }
  res->firstNewVertex=inputVertices;
  res->geoms=resP->geoms;
  res->mode=(GLenum*)malloc(res->geoms*sizeof(GLenum));
  res->vertices=(unsigned*)malloc(res->geoms*sizeof(unsigned));

  for (unsigned i=0;i<resP->geoms;++i) {
    res->mode[i]=resP->mode[i];    
    res->vertices[i]=resP->vertices[i];
  }
  res->dataSize=resP->dataSize;
  res->data=(unsigned*)malloc(res->dataSize*sizeof(unsigned));
  for (unsigned i=0;i<res->dataSize;++i) {
    assert(idx.find(resP->data[i])!=idx.end());
    res->data[i]=idx[resP->data[i]];
  }
  
  nocb_gluTessResPtr_delete(resP);
  return res;
}

struct nocb_GluTessRes*
nocb_gluTessPolygon(unsigned vertices, const GLdouble* data)
{
  return nocb_gluTessPolygonContours(1, &vertices, data);
}
