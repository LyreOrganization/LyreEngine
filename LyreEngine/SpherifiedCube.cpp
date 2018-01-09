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
	float cubeCoords = m_radius/sqrt(3.);
	m_vertices.assign({
		// front
		{ { -cubeCoords, -cubeCoords,  cubeCoords } },
		{ { cubeCoords, -cubeCoords,  cubeCoords } },
		{ { cubeCoords,  cubeCoords,  cubeCoords } },
		{ { -cubeCoords,  cubeCoords,  cubeCoords } },
		// back
		{ { -cubeCoords, -cubeCoords, -cubeCoords } },
		{ { cubeCoords, -cubeCoords, -cubeCoords } },
		{ { cubeCoords,  cubeCoords, -cubeCoords } },
		{ { -cubeCoords,  cubeCoords, -cubeCoords } },
	});

	m_cube = {
		make_unique<SpherifiedPlane>(this, DWORD4{ 2, 6, 7, 3 }),	// up
		make_unique<SpherifiedPlane>(this, DWORD4{ 2, 1, 5, 6 }),	// right
		make_unique<SpherifiedPlane>(this, DWORD4{ 2, 3, 0, 1 }),	// front

		make_unique<SpherifiedPlane>(this, DWORD4{ 4, 5, 1, 0 }),	// down	
		make_unique<SpherifiedPlane>(this, DWORD4{ 4, 7, 6, 5 }),	// back
		make_unique<SpherifiedPlane>(this, DWORD4{ 4, 0, 3, 7 }),	// left
	};

	// neighbours
	for (int i = 0; i < 3; i++)
	{
		m_cube[i]->m_neighbours[0] = m_cube[(i + 1) % 3].get();
		m_cube[i]->m_neighbours[3] = m_cube[(i + 2) % 3].get();

		m_cube[i + 3]->m_neighbours[0] = m_cube[(i + 1) % 3 + 3].get();
		m_cube[i + 3]->m_neighbours[3] = m_cube[(i + 2) % 3 + 3].get();

		m_cube[i]->m_neighbours[1] = m_cube[5 - (i + 1) % 3].get();
		m_cube[5 - (i + 1) % 3]->m_neighbours[1] = m_cube[i].get();

		m_cube[i]->m_neighbours[2] = m_cube[5 - i].get();
		m_cube[5 - i]->m_neighbours[2] = m_cube[i].get();
	}

	divide(2);
}

void SpherifiedCube::divide(unsigned depth)
{
	for (const auto& plane : m_cube)
	{
		plane->divide(depth);
	}
}

DWORD SpherifiedCube::createHalf(DWORD point1, DWORD point2)
{
	DWORD newInd = m_vertices.size();
	m_vertices.push_back(Vertex());
	XMStoreFloat3(&(m_vertices[newInd].position), XMVector3Normalize({
		(m_vertices[point1].position.x + m_vertices[point2].position.x) / 2.f,
		(m_vertices[point1].position.y + m_vertices[point2].position.y) / 2.f,
		(m_vertices[point1].position.z + m_vertices[point2].position.z) / 2.f,
		0
	}) * m_radius);
	return newInd;
}

vector<DWORD> SpherifiedCube::getIndicesBuffer()
{
	vector<DWORD> indices;
	for (const auto& plane : m_cube)
	{
		vector<DWORD> planeIndices = plane->getIndicesBuffer();
		indices.insert(indices.end(), planeIndices.begin(), planeIndices.end());
	}
	return indices;
}

vector<SpherifiedCube::Vertex> SpherifiedCube::getVertices() const 
{ 
	return m_vertices; 
}

/*
void SpherifiedCube::rebuildCPIndicesBuffer()
{
	m_indices.clear();
	m_indices.reserve(m_planesCount * 6);
	for (int i = 0; i < ICOS_PLANES; ++i)
	{
		buildTrianCPIndsR(m_icosahedron[i].get());
	}
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
