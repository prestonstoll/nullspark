// nullspark.cpp - Defines the interface for NullSpark
#include "nullspark.h"
#include "nsUtil.h"
#include "nsResource.h"
#include "nsMesh.h"
#include <map>
#include <boost/thread/thread.hpp>

namespace
{
	std::map<unsigned int, nsResource*>			ResourceMap;
	unsigned int								ResourceCounter;

	// Output Format
	NS_PIXEL_LAYOUT								PixelLayout;
	unsigned int								OutputWidth;
	unsigned int								OutputHeight;
	void *										OutputBuffer;

	// Threads
	boost::thread *								RenderThread;
}

// nsInit - Initialize NullSpark
NS_DLL_EXPORT NS_RETURNCODE	nsInit( void )
{
	ResourceCounter = 0;
	return NS_OK;
}

// nsLoadTriangleData - Loads triangle data into the system as a mesh
NS_DLL_EXPORT int nsLoadTriangleData( nsTriangle ** pTriangleData, const unsigned int pTriangleCount )
{
	unsigned int ResourceID = ResourceCounter++;
	nsMesh * newMesh = new nsMesh();
	ResourceMap.insert( std::pair<int,nsResource*>(ResourceID,newMesh) );

	if(newMesh->BuildFromTriangles( pTriangleData, pTriangleCount ) == NS_OK) {
		return ResourceID;
	}
	else {
		return NS_FAIL;
	}
}

// nsLoadOctreeFile - Loads octree data into the system as a mesh
NS_DLL_EXPORT int nsLoadOctreeData( const char * pOctreeData )
{
	return 0;
}

// nsDrawMesh - Queue the drawing of the mesh represented by ResourceID with the orientation of pTransform
NS_DLL_EXPORT NS_RETURNCODE	nsDrawMesh( const int ResourceID, const float * pTransform )
{
	return NS_OK;
}

// nsRender - Render the queued meshes to the output buffer
NS_DLL_EXPORT NS_RETURNCODE nsRender( void )
{
	return NS_OK;
}

// nsSetOutputFormat - Set the format of the output buffer
NS_DLL_EXPORT NS_RETURNCODE	nsSetOutputFormat( const NS_PIXEL_LAYOUT pPixelLayout, const unsigned int pWidth, const unsigned int pHeight )
{
	return NS_OK;
}

// nsOutputBuffer - Set the output buffer
NS_DLL_EXPORT NS_RETURNCODE nsSetOutputBuffer( void * pOutputBuffer )
{
	return NS_OK;
}

// nsShutdown - Shutdown NullSpark
NS_DLL_EXPORT NS_RETURNCODE nsShutdown( void  )
{
	return NS_OK;
}