// Copyright (C) 2002-2006 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CTerrainTriangleSelector.h"
#include "CTerrainSceneNode.h"
#include "os.h"
#include <stdio.h>
#include <memory.h>

namespace irr
{
namespace scene
{


//! constructor
CTerrainTriangleSelector::CTerrainTriangleSelector ( ITerrainSceneNode* node, s32 LOD )
: SceneNode ( node )
{
	#ifdef _DEBUG
	setDebugName ("CTerrainTriangleSelector");
	#endif

	setTriangleData ( node, LOD );
}

//! destructor
CTerrainTriangleSelector::~CTerrainTriangleSelector()
{
	TrianglePatches.TrianglePatchArray.clear ( );
}

//! Clears and sets triangle data
void CTerrainTriangleSelector::setTriangleData(ITerrainSceneNode* node, s32 LOD)
{
	s32 x, z, indexCount, i;
	core::triangle3df tri;
	core::array<u32> indices;
	CTerrainSceneNode* terrainNode = (CTerrainSceneNode*)node;

	// Get pointer to the GeoMipMaps vertices
	video::S3DVertex2TCoords* vertices = (video::S3DVertex2TCoords*)terrainNode->RenderBuffer.getVertices();

	// Clear current data
	TrianglePatches.TotalTriangles = 0;
	TrianglePatches.NumPatches = 0;

	int count = terrainNode->TerrainData.PatchCount * terrainNode->TerrainData.PatchCount;
	TrianglePatches.TrianglePatchArray.reallocate(count);
	for (int o=0; o<count; ++o)
		TrianglePatches.TrianglePatchArray.push_back(SGeoMipMapTrianglePatch());

	s32 tIndex;

	for( x = 0; x < terrainNode->TerrainData.PatchCount; x++ )
	{
		for( z = 0; z < terrainNode->TerrainData.PatchCount; z++ )
		{
			tIndex = x * terrainNode->TerrainData.PatchCount + z;
			TrianglePatches.TrianglePatchArray[tIndex].NumTriangles = 0;
			TrianglePatches.TrianglePatchArray[tIndex].Box = terrainNode->getBoundingBox( x, z );
			indexCount = terrainNode->getIndicesForPatch( indices, x, z, LOD );

			for( i = 0; i < indexCount; i += 3 )
			{
				tri.pointA = vertices[indices[i+0]].Pos;
				tri.pointB = vertices[indices[i+1]].Pos;
				tri.pointC = vertices[indices[i+2]].Pos;
				TrianglePatches.TrianglePatchArray[tIndex].Triangles.push_back( tri );
				TrianglePatches.TrianglePatchArray[tIndex].NumTriangles++;
			}

			TrianglePatches.TotalTriangles += TrianglePatches.TrianglePatchArray[tIndex].NumTriangles;
			TrianglePatches.NumPatches++;
		}
	}
}

//! Gets all triangles.
void CTerrainTriangleSelector::getTriangles ( core::triangle3df* triangles, s32 arraySize,
	s32& outTriangleCount, const core::matrix4* transform )
{
	s32 count = TrianglePatches.TotalTriangles;

	if (count > arraySize)
		count = arraySize;

	core::matrix4 mat;

	if (transform)
		mat = (*transform);

	s32 tIndex = 0;
	s32 i, j;

	for (i=0; i<TrianglePatches.NumPatches; ++i)
		if (tIndex + TrianglePatches.TrianglePatchArray[i].NumTriangles <= count)
			for (j=0; j<TrianglePatches.TrianglePatchArray[i].NumTriangles; ++j)
			{
				triangles[tIndex] = TrianglePatches.TrianglePatchArray[i].Triangles[j];

				mat.transformVect(triangles[tIndex].pointA);
				mat.transformVect(triangles[tIndex].pointB);
				mat.transformVect(triangles[tIndex].pointC);

				++tIndex;
			}

	outTriangleCount = tIndex;
}

//! Gets all triangles which lie within a specific bounding box.
void CTerrainTriangleSelector::getTriangles ( core::triangle3df* triangles, s32 arraySize,
	s32& outTriangleCount, const core::aabbox3d<f32>& box,
	const core::matrix4* transform)
{
	s32 count = TrianglePatches.TotalTriangles;

	if (count > arraySize)
		count = arraySize;

	core::matrix4 mat;

	if (transform)
		mat = (*transform);

	s32 tIndex = 0;
	s32 i, j;

	for (i=0; i<TrianglePatches.NumPatches; ++i)
		if (tIndex + TrianglePatches.TrianglePatchArray[i].NumTriangles <= count &&
			TrianglePatches.TrianglePatchArray[i].Box.intersectsWithBox(box))
			for (j=0; j<TrianglePatches.TrianglePatchArray[i].NumTriangles; ++j)
			{
				triangles[tIndex] = TrianglePatches.TrianglePatchArray[i].Triangles[j];

				mat.transformVect(triangles[tIndex].pointA);
				mat.transformVect(triangles[tIndex].pointB);
				mat.transformVect(triangles[tIndex].pointC);

				++tIndex;
			}

	outTriangleCount = tIndex;
}

//! Gets all triangles which have or may have contact with a 3d line.
void CTerrainTriangleSelector::getTriangles(core::triangle3df* triangles, s32 arraySize,
	s32& outTriangleCount, const core::line3d<f32>& line,
	const core::matrix4* transform)
{
	s32 count = TrianglePatches.TotalTriangles;

	if (count > arraySize)
		count = arraySize;

	core::matrix4 mat;

	if (transform)
		mat = (*transform);

	s32 tIndex = 0;
	s32 i, j;

	for (i=0; i<TrianglePatches.NumPatches; ++i)
	{
		if (tIndex + TrianglePatches.TrianglePatchArray[i].NumTriangles <= count
            && TrianglePatches.TrianglePatchArray[i].Box.intersectsWithLine(line))
		{
			for (j=0; j<TrianglePatches.TrianglePatchArray[i].NumTriangles; ++j)
			{
				triangles[tIndex] = TrianglePatches.TrianglePatchArray[i].Triangles[j];

				mat.transformVect(triangles[tIndex].pointA);
				mat.transformVect(triangles[tIndex].pointB);
				mat.transformVect(triangles[tIndex].pointC);

				++tIndex;
			}
		}
	}

	outTriangleCount = tIndex;
}

//! Returns amount of all available triangles in this selector
s32 CTerrainTriangleSelector::getTriangleCount() const
{
	return TrianglePatches.TotalTriangles;
}

} // end namespace scene
} // end namespace irr

