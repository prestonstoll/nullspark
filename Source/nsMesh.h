// nsMesh.h - Defines the functionality of a mesh. This class stores the octree data (in Root) and contains the functionality to load and display a voxel mesh.
#ifndef NSMESH_H__
#define NSMESH_H__

#include "nsResource.h"
#include "nsUtil.h"

#include <fstream>

#define DEFAULT_TOPBUILD_LEVEL 4
#define DEFAULT_SUBBUILD_LEVEL 4

struct nsOctreeNode;

class nsMesh : public nsResource
{
private:
	// The root node of the octree
	nsOctreeNode *					Root;

	// The original triangle data that created this mesh
	// (NULL if loaded from .nsoctree file)
	nsTriangle **					Triangles;
	unsigned int					TriangleCount;

	unsigned int					TopBuildLevel;
	unsigned int					SubBuildLevel;
	unsigned int					TopGridSize;
	unsigned int					SubGridSize;

	mat4							WorldMatrix;

	void *							TextureData;

	// AABB of the triangle data
	nsAABB							MeshAABB;
	// AABB of the root node (a cube centered at the mesh center position)
	nsAABB							GetRootNodeAABB();
	
	// helper functions for the loader
	unsigned int *					GetTrianglesInAABB(nsAABB& pAABB, unsigned int * triCount, unsigned int * TriangleData, unsigned int TriangleDataCount);
	unsigned int *					GetAllTrianglesInAABB(nsAABB& pAABB, unsigned int * triCount);
	vec3							SampleTexture(unsigned int * TriangleData, unsigned int TriangleDataIndex, nsAABB& nodeAABB, nsOctreeLeafNode * node);
	void							WriteNodeToFile(nsOctreeNodeBase * mNode, std::ofstream * out);
protected:
public:
									nsMesh( void );
	int								Destroy( void );
	NS_RETURNCODE					BuildFromTriangles( nsTriangle ** pTriangleData, const unsigned int pTriangleCount );
	int								BuildNode(nsOctreeNode * node, nsAABB * nAABB, unsigned int * TriangleData, unsigned int TriangleDataCount, unsigned int level);
	int								BuildSubNode(nsOctreeNode * node, nsAABB * nAABB, unsigned int * TriangleData, unsigned int TriangleDataCount, unsigned int level);
	void							WriteToFile(char * fileName);
};

#endif