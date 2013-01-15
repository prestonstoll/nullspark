// nsMesh.h - Defines the functionality of a mesh
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
	nsOctreeNode *					Root;

	nsTriangle **					Triangles;
	unsigned int					TriangleCount;

	unsigned int					TopBuildLevel;
	unsigned int					SubBuildLevel;
	unsigned int					TopGridSize;
	unsigned int					SubGridSize;

	mat4							WorldMatrix;

	void *							TextureData;

	nsAABB							MeshAABB;
	nsAABB							GetRootNodeAABB();
	
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