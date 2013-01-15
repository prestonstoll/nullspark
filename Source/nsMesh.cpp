// nsMesh.cpp - Provides the functionality of building and using a mesh

#include "nsMesh.h"
#include "nsUtil.h"
#include <vector>

nsMesh::nsMesh( void )
{
	TopBuildLevel = DEFAULT_TOPBUILD_LEVEL;
	SubBuildLevel = DEFAULT_SUBBUILD_LEVEL;

	TopGridSize = 1 << (TopBuildLevel + 1);
	SubGridSize = 1 << (SubBuildLevel + 1);
}

int	nsMesh::Destroy( void )
{
	return 0;
}

int	nsMesh::BuildFromTriangles( nsTriangle ** pTriangleData, const unsigned int pTriangleCount )
{
	Root = (nsOctreeNode *) malloc(sizeof(nsOctreeNode));
	Root->type = OCTREE_NODE;
	MeshAABB = ComputeAABB(pTriangleData, pTriangleCount);
	Root->AABB = GetRootNodeAABB();

	Triangles = pTriangleData;
	TriangleCount = pTriangleCount;

	unsigned int triCount = 0;
	unsigned int * triangles;

	nsAABB childAABB;

	for(unsigned int c = 0; c < 8; c++)
	{
		childAABB = ChildAABB(&(Root->AABB), c);
		triangles = GetAllTrianglesInAABB(childAABB,&triCount);
		if(triCount > 0)
		{
			Root->Children[c] = (nsOctreeNodeBase *) malloc(sizeof(nsOctreeNode));
			Root->Children[c]->type = OCTREE_NODE;
			((nsOctreeNode *)Root->Children[c])->AABB = childAABB;

			// Build Node
			BuildNode((nsOctreeNode *)Root->Children[c], &childAABB, triangles, triCount, TopBuildLevel-1);

			// free the triangles
			free( triangles );
		}
		else
		{
			Root->Children[c] = NULL;
		}
	}

	WriteToFile("C:\\testOutput.nsoctree");
	return 0;
}

int nsMesh::BuildNode(nsOctreeNode * node, nsAABB * nAABB, unsigned int * TriangleData, unsigned int TriangleDataCount, unsigned int level)
{
	unsigned int triCount = 0;
	unsigned int * triangles;
	nsAABB childAABB;
	if(level != 0)
	{
		// Node children
		for(unsigned int c = 0; c < 8; c++)
		{
			childAABB = ChildAABB(nAABB, c);
			triangles = GetTrianglesInAABB(childAABB, &triCount, TriangleData, TriangleDataCount );
			if(triCount > 0)
			{
				node->Children[c] = (nsOctreeNodeBase *) malloc(sizeof(nsOctreeNode));
				node->Children[c]->type = OCTREE_NODE;
				((nsOctreeNode *)node->Children[c])->AABB = childAABB;

				// Build Node
				BuildNode((nsOctreeNode *)node->Children[c], &childAABB, triangles, triCount, level-1);

				// free the triangles
				free( triangles );
			}
			else
			{
				node->Children[c] = NULL;
			}
		}
	}
	else
	{
		// Leaf children
		for(unsigned int c = 0; c < 8; c++)
		{
			childAABB = ChildAABB(nAABB, c);
			triangles = GetTrianglesInAABB(childAABB, &triCount, TriangleData, TriangleDataCount );
			if(triCount > 0)
			{
				node->Children[c] = (nsOctreeNodeBase *) malloc(sizeof(nsOctreeNode));
				node->Children[c]->type = OCTREE_SUPER_LEAF;
				((nsOctreeNode *)node->Children[c])->AABB = childAABB;

				// Build Node
				BuildSubNode((nsOctreeNode *)node->Children[c], &childAABB, triangles, triCount, SubBuildLevel);

				// free the triangles (not saving this triangle data)
				free( triangles );
			}
			else
			{
				node->Children[c] = NULL;
			}
		}
	}

	return 0;
}

int nsMesh::BuildSubNode(nsOctreeNode * node, nsAABB * nAABB, unsigned int * TriangleData, unsigned int TriangleDataCount, unsigned int level)
{
	unsigned int triCount = 0;
	unsigned int * triangles;
	nsAABB childAABB;
	if(level != 0)
	{
		// Node children
		for(unsigned int c = 0; c < 8; c++)
		{
			childAABB = ChildAABB(nAABB, c);
			triangles = GetTrianglesInAABB(childAABB, &triCount, TriangleData, TriangleDataCount );
			if(triCount > 0)
			{
				node->Children[c] = (nsOctreeNodeBase *) malloc(sizeof(nsOctreeNode));
				node->Children[c]->type = OCTREE_NODE;
				((nsOctreeNode *)node->Children[c])->AABB = childAABB;

				// Build Node
				BuildSubNode((nsOctreeNode *)node->Children[c], &childAABB, triangles, triCount, level-1);

				// free the triangles
				free( triangles );
			}
			else
			{
				node->Children[c] = NULL;
			}
		}
	}
	else
	{
		// Leaf children
		for(unsigned int c = 0; c < 8; c++)
		{
			childAABB = ChildAABB(nAABB, c);
			triangles = GetTrianglesInAABB(childAABB, &triCount, TriangleData, TriangleDataCount );
			if(triCount > 0)
			{
				node->Children[c] = (nsOctreeNodeBase *) malloc(sizeof(nsOctreeLeafNode));
				node->Children[c]->type = OCTREE_LEAF;

				// Build Leaf
				((nsOctreeLeafNode *)node->Children[c])->AvgNormal.x = 0.0f;
				((nsOctreeLeafNode *)node->Children[c])->AvgNormal.y = 0.0f;
				((nsOctreeLeafNode *)node->Children[c])->AvgNormal.z = 0.0f;

				float ratio = 1.0f / triCount;
				for(unsigned int t = 0; t < triCount; t++)
				{
					((nsOctreeLeafNode *)node->Children[c])->AvgNormal = ((nsOctreeLeafNode *)node->Children[c])->AvgNormal + *Triangles[triangles[t]]->normal * ratio;
				}
				normalize(((nsOctreeLeafNode *)node->Children[c])->AvgNormal);

				
				((nsOctreeLeafNode *)node->Children[c])->AvgColor.x = 0.0f;
				((nsOctreeLeafNode *)node->Children[c])->AvgColor.y = 0.0f;
				((nsOctreeLeafNode *)node->Children[c])->AvgColor.z = 0.0f;

				for(unsigned int t = 0; t < triCount; t++)
				{
					((nsOctreeLeafNode *)node->Children[c])->AvgColor= ((nsOctreeLeafNode *)node->Children[c])->AvgColor + SampleTexture(triangles, t, childAABB,((nsOctreeLeafNode *)node->Children[c])) * ratio;
				}

				//((OctreeLeafNode *)node->Children[c])->AvgColor = SampleTexture(VMC, triangles, 0, childAABB,((OctreeLeafNode *)node->Children[c]));

				// free the triangles (not saving this triangle data)
				free( triangles );
			}
			else
			{
				node->Children[c] = NULL;
			}
		}
	}

	return 0;
}

vec3 nsMesh::SampleTexture(unsigned int * TriangleData, unsigned int TriangleDataIndex, nsAABB& nodeAABB, nsOctreeLeafNode * node)
{
	float w, u, v;
	vec3 textureColor;
	vec3 r, uvw;
	nsRay ray;

	
	unsigned int TriangleIndex = TriangleData[TriangleDataIndex];

	textureColor.x = 0.0f;
	textureColor.y = 0.0f;
	textureColor.z = 0.0f;

	ray.origin.x = (nodeAABB.min.x + nodeAABB.max.x) * 0.5f;
	ray.origin.y = (nodeAABB.min.y + nodeAABB.max.y) * 0.5f;
	ray.origin.z = (nodeAABB.min.z + nodeAABB.max.z) * 0.5f;

	//ray.d.x = 0.0f;
	//ray.d.y = 0.0f;
	//ray.d.z = -1.0f;

	ray.dir = *(Triangles[TriangleIndex]->normal) * -1.0f;
	ray.origin = ray.origin - ray.dir * (nodeAABB.max.z - nodeAABB.min.z);

	bool hit = IntersectSegmentTriangleEx(
		ray.origin, ray.origin + ray.dir * 99999.0f, 
		Triangles[TriangleIndex]->vertices[0]->point,
		Triangles[TriangleIndex]->vertices[1]->point,
		Triangles[TriangleIndex]->vertices[2]->point,
		&r, w, u, v);

	if(!hit)
		IntersectSegmentTriangleEx(
		ray.origin, ray.origin + ray.dir * 99999.0f, 
		Triangles[TriangleIndex]->vertices[2]->point,
		Triangles[TriangleIndex]->vertices[1]->point,
		Triangles[TriangleIndex]->vertices[0]->point,
		&r, w, u, v);

	int a,b,c;

	//if(!hit)
	//	return textureColor;

	uvw[0] = (Triangles[TriangleIndex]->vertices[0]->point - r).length();
	uvw[1] = (Triangles[TriangleIndex]->vertices[1]->point - r).length();
	uvw[2] = (Triangles[TriangleIndex]->vertices[2]->point - r).length();

	if(uvw[0] > uvw[1] && uvw[0] > uvw[2])
	{
		a = 0;
		b = 1;
		c = 2;
	}
	else if(uvw[1] > uvw[0] && uvw[1] > uvw[2])
	{
		a = 1;
		b = 0;
		c = 2;
	}
	else
	{
		a = 2;
		b = 0;
		c = 1;
	}

	vec3 aPoint = Triangles[TriangleIndex]->vertices[a]->point;
	vec3 bPoint = Triangles[TriangleIndex]->vertices[b]->point;
	vec3 cPoint = Triangles[TriangleIndex]->vertices[c]->point;

	float aU = Triangles[TriangleIndex]->vertices[a]->u;
	float bU = Triangles[TriangleIndex]->vertices[b]->u;
	float cU = Triangles[TriangleIndex]->vertices[c]->u;

	float aV = Triangles[TriangleIndex]->vertices[a]->v;
	float bV = Triangles[TriangleIndex]->vertices[b]->v;
	float cV = Triangles[TriangleIndex]->vertices[c]->v;

	nsRay iRay;
	iRay.origin = aPoint;
	iRay.dir = normalize( r - aPoint );

	nsPlane triPlane = ComputePlane(cPoint, cross(cPoint - bPoint, *Triangles[TriangleIndex]->normal));
	nsPlane triPlaneInv = ComputePlane(cPoint, cross(cPoint - bPoint, *Triangles[TriangleIndex]->normal * -1.0f));
	vec3 q;
	float qt;

	if(!IntersectSegmentPlane(iRay.origin, iRay.origin + iRay.dir * 999999.0f, triPlane, qt, q))
		IntersectSegmentPlane(iRay.origin, iRay.origin + iRay.dir * 999999.0f, triPlaneInv, qt, q);

	float distanceBQ = (bPoint - q).length();
	float distanceBC = (bPoint - cPoint).length();
	
	float distanceQI = (q - r).length();
	float distanceQA = (q - aPoint).length();

	float UQ = bU + ((cU - bU) * (distanceBQ / distanceBC));
	u = UQ + ((aU - UQ) * (distanceQI / distanceQA));

	float VQ = bV + ((cV - bV) * (distanceBQ / distanceBC));
	v = VQ + ((aV - VQ) * (distanceQI / distanceQA));

	int col = std::max(0.0f, std::min(u, 1.0f)) * 256;
	int row = std::max(0.0f, std::min(v, 1.0f)) * 256;

	if(TextureData != NULL)
	{
		const unsigned char * pixel = (const unsigned char *)TextureData + 256 * row * 3 + 3 * col;
		textureColor.x = pixel[0] / 255.0;
		textureColor.y = pixel[1] / 255.0;
		textureColor.z = pixel[2] / 255.0;
	}
	else
	{
		textureColor.x = u;
		textureColor.y = v;
		textureColor.z = 0.0f;
	}
	return textureColor;
}

nsAABB nsMesh::GetRootNodeAABB()
{
	float maxLen = -FLT_MAX;
	nsAABB rootAABB;

	for(unsigned int i = 0; i < 3; i++)
	{
		if(MeshAABB.max[i] - MeshAABB.min[i] > maxLen)
		{
			maxLen = MeshAABB.max[i] - MeshAABB.min[i];
		}
	}

	float width = maxLen;
	float halfWidth = width * 0.5f;

	vec3 newOrigin;
	newOrigin.x = (MeshAABB.max.x + MeshAABB.min.x) * 0.5f;
	newOrigin.y = (MeshAABB.max.y + MeshAABB.min.y) * 0.5f;
	newOrigin.z = (MeshAABB.max.z + MeshAABB.min.z) * 0.5f;

	rootAABB.min.x = newOrigin.x - halfWidth;
	rootAABB.min.y = newOrigin.y - halfWidth;
	rootAABB.min.z = newOrigin.z - halfWidth;
	rootAABB.max.x = newOrigin.x + halfWidth;
	rootAABB.max.y = newOrigin.y + halfWidth;
	rootAABB.max.z = newOrigin.z + halfWidth;

	return rootAABB;
}

unsigned int * nsMesh::GetTrianglesInAABB(nsAABB& pAABB, unsigned int * triCount, unsigned int * TriangleData, unsigned int TriangleDataCount)
{
	std::vector<unsigned int> TriangleIndices;
	float center[3];
	float size[3];
	float points[3][3];

	size[0] = (pAABB.max.x - pAABB.min.x);
	size[1] = (pAABB.max.y - pAABB.min.y);
	size[2] = (pAABB.max.z - pAABB.min.z);

	center[0] = (pAABB.max.x + pAABB.min.x) * 0.5f;
	center[1] = (pAABB.max.y + pAABB.min.y) * 0.5f;
	center[2] = (pAABB.max.z + pAABB.min.z) * 0.5f;

	unsigned int triIndex;
	for(unsigned int t = 0; t < TriangleDataCount; t++)
	{
		triIndex = TriangleData[t];
		if(IntersectAABB(*Triangles[triIndex]->pAABB, pAABB) == true)
		{
			points[0][0] = Triangles[triIndex]->vertices[0]->point.x;
			points[0][1] = Triangles[triIndex]->vertices[0]->point.y;
			points[0][2] = Triangles[triIndex]->vertices[0]->point.z;
			
			points[1][0] = Triangles[triIndex]->vertices[1]->point.x;
			points[1][1] = Triangles[triIndex]->vertices[1]->point.y;
			points[1][2] = Triangles[triIndex]->vertices[1]->point.z;
			
			points[2][0] = Triangles[triIndex]->vertices[2]->point.x;
			points[2][1] = Triangles[triIndex]->vertices[2]->point.y;
			points[2][2] = Triangles[triIndex]->vertices[2]->point.z;

			if(triBoxOverlap(center, size, points) == 1)
			{
				TriangleIndices.push_back(triIndex);
			}
		}
	}

	unsigned int * ret;
	if(TriangleIndices.size() > 0)
	{
		ret = (unsigned int *) malloc(sizeof(unsigned int) * TriangleIndices.size());
		memcpy(ret, &TriangleIndices[0], sizeof(unsigned int) * TriangleIndices.size());
		*triCount = (unsigned int)TriangleIndices.size();
		return ret;
	}
	else
	{
		*triCount = 0;
		return (unsigned int *)0;
	}
}

unsigned int * nsMesh::GetAllTrianglesInAABB(nsAABB& pAABB, unsigned int * triCount)
{
	std::vector<unsigned int> TriangleIndices;
	float center[3];
	float size[3];
	float points[3][3];

	size[0] = (pAABB.max.x - pAABB.min.x);
	size[1] = (pAABB.max.y - pAABB.min.y);
	size[2] = (pAABB.max.z - pAABB.min.z);

	center[0] = (pAABB.max.x + pAABB.min.x) * 0.5f;
	center[1] = (pAABB.max.y + pAABB.min.y) * 0.5f;
	center[2] = (pAABB.max.z + pAABB.min.z) * 0.5f;

	unsigned int triIndex;
	nsAABB extendedAABB = pAABB;
	for(unsigned int t = 0; t < TriangleCount; t++)
	{
		triIndex = t;
		if(IntersectAABB(*Triangles[triIndex]->pAABB, pAABB) == true)
		{
			points[0][0] = Triangles[triIndex]->vertices[0]->point.x;
			points[0][1] = Triangles[triIndex]->vertices[0]->point.y;
			points[0][2] = Triangles[triIndex]->vertices[0]->point.z;
			
			points[1][0] = Triangles[triIndex]->vertices[1]->point.x;
			points[1][1] = Triangles[triIndex]->vertices[1]->point.y;
			points[1][2] = Triangles[triIndex]->vertices[1]->point.z;
			
			points[2][0] = Triangles[triIndex]->vertices[2]->point.x;
			points[2][1] = Triangles[triIndex]->vertices[2]->point.y;
			points[2][2] = Triangles[triIndex]->vertices[2]->point.z;

			if(triBoxOverlap(center, size, points) == 1)
			{
				TriangleIndices.push_back(triIndex);
			}
		}
	}

	unsigned int * ret;
	if(TriangleIndices.size() > 0)
	{
		ret = (unsigned int *) malloc(sizeof(unsigned int) * TriangleIndices.size());
		memcpy(ret, &TriangleIndices[0], sizeof(unsigned int) * TriangleIndices.size());
		*triCount = (unsigned int)TriangleIndices.size();
		return ret;
	}
	else
	{
		*triCount = 0;
		return (unsigned int *)0;
	}
}

void nsMesh::WriteToFile(char * fileName)
{
	std::ofstream out(fileName, std::ios::out | std::ios::binary);

	if(out.good())
	{
		out.write((char *)&TopBuildLevel, sizeof(int));
		out.write((char *)&SubBuildLevel, sizeof(int));
		out.write((char *)&TopGridSize, sizeof(unsigned int));
		out.write((char *)&SubGridSize, sizeof(unsigned int));
		out.write((char *)&WorldMatrix, sizeof(float) * 16);
		nsAABB rootAABB = GetRootNodeAABB();
		out.write((char *)&rootAABB, sizeof(float) * 6);

		WriteNodeToFile(Root, &out);
	}
}

void nsMesh::WriteNodeToFile(nsOctreeNodeBase * mNode, std::ofstream * out)
{
	if(mNode == NULL)
	{
		char stubType = OCTREE_STUB;
		out->write((char *)&(stubType), sizeof(char));
		return;
	}

	if(mNode->type == OCTREE_NODE || mNode->type == OCTREE_SUPER_LEAF)
	{
		nsOctreeNode * node = (nsOctreeNode *)mNode;
		out->write((char *)&(node->type), sizeof(char));
		out->write((char *)&(node->AABB), sizeof(float) * 6);

		for(unsigned int c = 0; c < 8; c++)
		{
			WriteNodeToFile(node->Children[c], out);
		}
	}
	else if(mNode->type == OCTREE_LEAF)
	{
		nsOctreeLeafNode * node = (nsOctreeLeafNode *)mNode;
		out->write((char *)&(mNode->type), sizeof(char));
		out->write((char *)&node->AvgNormal, sizeof(float) * 3);
		out->write((char *)&node->AvgColor, sizeof(float) * 3);
	}
}