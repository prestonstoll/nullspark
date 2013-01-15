// nullspark.h - The interface definition for NullSpark

#ifndef NULLSPARK_H__
#define NULLSPARK_H__

#include "nsUtil.h"

struct nsTriangle;

#define NS_DLL_EXPORT extern "C" __declspec(dllexport)

// nsInit - Initialize NullSpark
NS_DLL_EXPORT NS_RETURNCODE					nsInit( void );

// nsLoadTriangleData - Loads triangle data into the system as a mesh
NS_DLL_EXPORT int							nsLoadTriangleData( nsTriangle ** pTriangleData, const unsigned int pTriangleCount );

// nsLoadOctreeFile - Loads octree data into the system as a mesh
NS_DLL_EXPORT int							nsLoadOctreeData( const char * pOctreeData );

// nsDrawMesh - Queue the drawing of the mesh represented by ResourceID with the world orientation of pTransform
NS_DLL_EXPORT NS_RETURNCODE					nsDrawMesh( const int ResourceID, const float * pTransform );

// nsRender - Render the queued meshes to the output buffer
NS_DLL_EXPORT NS_RETURNCODE					nsRender( void );

// nsSetOutputFormat - Set the format of the output buffer
NS_DLL_EXPORT NS_RETURNCODE					nsSetOutputFormat( NS_PIXEL_LAYOUT pPixelLayout, unsigned int pWidth, unsigned int pHeight );

// nsOutputBuffer - Set the output buffer
NS_DLL_EXPORT NS_RETURNCODE					nsSetOutputBuffer( void * pOutputBuffer );

// nsShutdown - Shutdown NullSpark
NS_DLL_EXPORT NS_RETURNCODE					nsShutdown( void  );

#endif