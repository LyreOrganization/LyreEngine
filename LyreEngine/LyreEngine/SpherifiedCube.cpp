#include "stdafx.h"

#include "SpherifiedCube.h"

using namespace std;
using namespace DirectX;

SpherifiedCube::SpherifiedCube(float radius)
	: m_radius(radius)
{
	buildCube();
}

void SpherifiedCube::buildCube()
{
	m_vertices.assign({
		// front
		{ { -1.0, -1.0,  1.0 } },
		{ { 1.0, -1.0,  1.0 } },
		{ { 1.0,  1.0,  1.0 } },
		{ { -1.0,  1.0,  1.0 } },
		// back
		{ { -1.0, -1.0, -1.0 } },
		{ { 1.0, -1.0, -1.0 } },
		{ { 1.0,  1.0, -1.0 } },
		{ { -1.0,  1.0, -1.0 } },
	});

	m_cube = {
		make_unique<SpherifiedPlane>(this, DWORD4{ 0, 1, 2, 3 }), // front
		make_unique<SpherifiedPlane>(this, DWORD4{ 1, 5, 6, 2 }), // top
		make_unique<SpherifiedPlane>(this, DWORD4{ 7, 6, 5, 4 }), // back
		make_unique<SpherifiedPlane>(this, DWORD4{ 4, 0, 3, 7 }), // bottom
		make_unique<SpherifiedPlane>(this, DWORD4{ 4, 5, 1, 0 }), // left
		make_unique<SpherifiedPlane>(this, DWORD4{ 3, 2, 6, 7 }) // right
	};
}

void SpherifiedCube::divide(unsigned depth)
{
	for (const auto& plane : m_cube)
	{
		plane->divide(depth);
	}
}

/*
void SpherifiedCube::rebuildVertexIndicesBuffer()
{
	m_indices.clear();
	m_indices.reserve(m_planesCount * 3);
	for (int i = 0; i < ICOS_PLANES; ++i)
	{
		buildTrianVIndsR(m_icosahedron[i].get());
	}
}

void SpherifiedCube::rebuildCPIndicesBuffer()
{
	m_indices.clear();
	m_indices.reserve(m_planesCount * 6);
	for (int i = 0; i < ICOS_PLANES; ++i)
	{
		buildTrianCPIndsR(m_icosahedron[i].get());
	}
}

SpherifiedPoint* SpherifiedCube::createHalf(SpherifiedPoint* point1, SpherifiedPoint* point2)
{
	DWORD newInd = m_pointsCount++;
	m_vertices.push_back(LESimpleVertex());
	XMStoreFloat3(&(m_vertices[newInd].pos), XMVector3Normalize(
	{
		(getVertexByTrianPoint(point1)->pos.x + getVertexByTrianPoint(point2)->pos.x) / 2,
		(getVertexByTrianPoint(point1)->pos.y + getVertexByTrianPoint(point2)->pos.y) / 2,
		(getVertexByTrianPoint(point1)->pos.z + getVertexByTrianPoint(point2)->pos.z) / 2,
		0
	}) * m_radius);
	m_points.push_front(std::make_unique<SpherifiedPoint>(newInd));
	return m_points.front().get();
}

const std::vector<LESimpleVertex>& SpherifiedCube::getVertices()
{
	return m_vertices;
}

void SpherifiedCube::fillAllTriangleVertices(std::vector<LESimpleVertex> &allTriangleVertices)
{
	allTriangleVertices.clear();
	for (int i = 0; i < m_indices.size(); i++)
	{
		allTriangleVertices.push_back(m_vertices[m_indices[i]]);
	}
}

const std::vector<DWORD>& SpherifiedCube::getIndices()
{
	return m_indices;
}

void SpherifiedCube::updatePlanesAmount()
{
	m_planesCount = 0;
	for (int i = 0; i < ICOS_PLANES; ++i)
	{
		m_planesCount += m_icosahedron[i]->getTrianglesAmount();
	}
}

LESimpleVertex * SpherifiedCube::getVertexByIndex(DWORD index)
{
	return &(m_vertices[index]);
}

LESimpleVertex* SpherifiedCube::getVertexByTrianPoint(SpherifiedPoint * point)
{
	return &(m_vertices[point->getIndex()]);
}

int SpherifiedCube::getTrianPlanesAmount()
{
	return m_planesCount;
}

int SpherifiedCube::getTrianPointsAmount()
{
	return m_pointsCount;
}

int SpherifiedCube::getIndicesAmount()
{
	return m_indices.size();
}

void SpherifiedCube::buildTrianVIndsR(SpherifiedPlane* trian)
{
	if (trian->m_divided)
	{
		buildTrianVIndsR(trian->m_children[0].get());
		buildTrianVIndsR(trian->m_children[1].get());
		buildTrianVIndsR(trian->m_children[2].get());
		buildTrianVIndsR(trian->m_children[3].get());
	}
	//separate in two triangles to cover neighbour's half (if it exists)
	else switch (trian->m_validHalf)
	{
	case 0:
		m_indices.push_back(trian->getHalfIndex(0));
		m_indices.push_back(trian->getTrianPointIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(0));
		////
		m_indices.push_back(trian->getHalfIndex(0));
		m_indices.push_back(trian->getTrianPointIndex(0));
		m_indices.push_back(trian->getTrianPointIndex(2));
		break;
	case 1:
		m_indices.push_back(trian->getHalfIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(0));
		////
		m_indices.push_back(trian->getHalfIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(1));
		break;
	case 2:
		m_indices.push_back(trian->getHalfIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(0));
		m_indices.push_back(trian->getTrianPointIndex(2));
		////
		m_indices.push_back(trian->getHalfIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(1));
		break;
	default://no halfs exist
		m_indices.push_back(trian->getTrianPointIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(0));
		break;
	}
}

void SpherifiedCube::buildTrianCPIndsR(SpherifiedPlane * trian)
{
	if (trian->m_divided)
	{
		buildTrianCPIndsR(trian->m_children[0].get());
		buildTrianCPIndsR(trian->m_children[1].get());
		buildTrianCPIndsR(trian->m_children[2].get());
		buildTrianCPIndsR(trian->m_children[3].get());
	}
	//no half separation for control point patches
	else
	{
		m_indices.push_back(trian->getTrianPointIndex(2));
		m_indices.push_back(trian->getTrianPointIndex(1));
		m_indices.push_back(trian->getTrianPointIndex(0));
		////
		if (trian->m_neighbours[2]->m_direction != trian->m_direction)
			m_indices.push_back(trian->m_neighbours[2]->getTrianPointIndex(2));
		else m_indices.push_back(trian->m_neighbours[2]->getTrianPointIndex(1));
		if (trian->m_neighbours[1]->m_direction != trian->m_direction)
			m_indices.push_back(trian->m_neighbours[1]->getTrianPointIndex(1));
		else m_indices.push_back(trian->m_neighbours[1]->getTrianPointIndex(2));
		m_indices.push_back(trian->m_neighbours[0]->getTrianPointIndex(0));
	}
}
*/
