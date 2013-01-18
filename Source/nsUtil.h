// nsUtil.h - basic math utilities for NullSpark

#ifndef NSUTIL_H__
#define NSUTIL_H__

#include <glm/glm.hpp>
using namespace glm;

enum						{ OCTREE_NODE, OCTREE_SUPER_LEAF, OCTREE_LEAF, OCTREE_STUB };
enum	NS_RETURNCODE		{ NS_FAIL = -1, NS_OK, NS_FALSE, NS_RETURNCODE_COUNT };
enum	NS_PIXEL_LAYOUT		{ NS_PIXEL_RGBA_8, NS_PIXEL_LAYOUT_COUNT };

struct nsVertex
{
	vec3 point;
	vec3 normal;
	float u, v;
};

struct nsAABB
{
	vec3 min, max;
};

struct nsTriangle
{
	nsVertex ** vertices;
	vec3 * normal;
	nsAABB * pAABB;
};

struct nsRay
{
	vec3 dir;
	vec3 origin;
};

struct nsOctreeNodeBase
{
	char type;
};

struct nsPlane
{
	vec3 normal;
	float dir;
};

struct nsOctreeNode : public nsOctreeNodeBase
{
	nsAABB				AABB;
	nsOctreeNodeBase *	Children[8];
};

struct nsOctreeLeafNode : public nsOctreeNodeBase
{
	vec3 AvgNormal;	// The average triangle normal
	vec3 AvgColor;	// The average albedo color
};

static nsAABB ComputeAABB( nsTriangle ** pTriangleData, const unsigned int pTriangleCount )
{
	nsAABB ReturnAABB;
	float MinProjection[3], MaxProjection[3], CurrentProjection[3];
	vec3 CurrentPoint;
	const nsTriangle * CurrentTriangle;
	vec3 Direction[3];
	Direction[0].x = 1.0f;
	Direction[1].y = 1.0f;
	Direction[2].z = 1.0f;

	MinProjection[0] = FLT_MAX;
	MinProjection[1] = FLT_MAX;
	MinProjection[2] = FLT_MAX;
	MaxProjection[0] = -FLT_MAX;
	MaxProjection[1] = -FLT_MAX;
	MaxProjection[2] = -FLT_MAX;

	ReturnAABB.min.x = FLT_MAX;
	ReturnAABB.min.y = FLT_MAX;
	ReturnAABB.min.z = FLT_MAX;
	ReturnAABB.max.x = -FLT_MAX;
	ReturnAABB.max.y = -FLT_MAX;
	ReturnAABB.max.z = -FLT_MAX;

	for(unsigned int i = 0; i < pTriangleCount; i++)
	{
		CurrentTriangle = pTriangleData[i];
		for(unsigned int v = 0; v < 3; v++)
		{
			CurrentPoint = CurrentTriangle->vertices[v]->point;
			for(unsigned int a = 0; a < 3; a++)
			{
				CurrentProjection[a] = dot(CurrentPoint, Direction[a]);

				if(CurrentProjection[a] < MinProjection[a])
				{
					MinProjection[a] = CurrentProjection[a];
					ReturnAABB.min[a] = CurrentPoint[a];
				}

				if(CurrentProjection[a] > MaxProjection[a])
				{
					MaxProjection[a] = CurrentProjection[a];
					ReturnAABB.max[a] = CurrentPoint[a];
				}
			}
		}
	}

	return ReturnAABB;
}

static nsAABB ChildAABB(nsAABB * nAABB, unsigned int c)
{
	nsAABB ret;
	float width = nAABB->max.x - nAABB->min.x;
	float step = width * 0.25f;

	vec3 origin, offset, newOrigin;

	origin.x = (nAABB->max.x + nAABB->min.x) * 0.5f;
	origin.y = (nAABB->max.y + nAABB->min.y) * 0.5f;
	origin.z = (nAABB->max.z + nAABB->min.z) * 0.5f;

	offset.x = ((c & 1) ? step : -step);
	offset.y = ((c & 2) ? step : -step);
	offset.z = ((c & 4) ? step : -step);

	newOrigin = origin + offset;

	ret.max.x = newOrigin.x + step;
	ret.max.y = newOrigin.y + step;
	ret.max.z = newOrigin.z + step;
	ret.min.x = newOrigin.x - step;
	ret.min.y = newOrigin.y - step;
	ret.min.z = newOrigin.z - step;

	return ret;
}

static bool IntersectAABB(nsAABB& a, nsAABB& b)
{
	if(a.max[0] < b.min[0] || a.min[0] > b.max[0]) return false;
	if(a.max[1] < b.min[1] || a.min[1] > b.max[1]) return false;
	if(a.max[2] < b.min[2] || a.min[2] > b.max[2]) return false;
	return true;
}

/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3]);

static bool IntersectSegmentTriangleEx(vec3& p, vec3& q, vec3& a, vec3& b, vec3& c, vec3* r, float& w, float& v, float& u)
{
	vec3 pq = q - p;
	vec3 pa = a - p;
	vec3 pb = b - p;
	vec3 pc = c - p;

	vec3 m = cross(pq, p);
	float s = dot(m, c - b);
	float t = dot(m, a - c);
	u = dot(pq, cross(c, b)) + s;
	if(u < 0.0f) return false;
	v = dot(pq, cross(a, c)) + t;
	if(v < 0.0f) return false;
	w = dot(pq, cross(b, a)) - s - t;
	if(w < 0.0f) return false;

	float denom = 1.0f / (u + v + w);
	u *= denom;
	v *= denom;
	w *= denom;

	*r = a * u  + b * v + c * w;
	return true;
}

static nsPlane ComputePlane(vec3& o, vec3& n)
{
	nsPlane p;
	p.normal = n;
	p.dir = dot(p.normal, o);
	return p;
}

static bool IntersectSegmentPlane(vec3& a, vec3& b, nsPlane& pl, float& t, vec3& q)
{
	vec3 pq = b - a;
	t = (pl.dir - dot(pl.normal, a)) / dot(pl.normal, pq);

	if(t >= 0.0f && t<= 1.0f)
	{
		q = a + pq * t;
		return true;
	}

	return false;
}

#endif