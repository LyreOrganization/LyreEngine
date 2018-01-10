#include "stdafx.h"

#include "SpherifiedCube.h"

#include "SpherifiedPlane.h"

using namespace std;
using namespace DirectX;

SpherifiedCube::SpherifiedCube(float radius)
	: m_radius(radius) {

	buildCube();
}

SpherifiedCube::~SpherifiedCube() {}

void SpherifiedCube::buildCube() {
	float cubeCoords = m_radius / sqrt(3.f);
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
	for (int i = 0; i < 3; i++) {
		m_cube[i]->m_neighbours[0] = m_cube[(i + 1) % 3].get();
		m_cube[i]->m_neighbours[3] = m_cube[(i + 2) % 3].get();

		m_cube[i + 3]->m_neighbours[0] = m_cube[(i + 1) % 3 + 3].get();
		m_cube[i + 3]->m_neighbours[3] = m_cube[(i + 2) % 3 + 3].get();

		m_cube[i]->m_neighbours[1] = m_cube[5 - (i + 1) % 3].get();
		m_cube[5 - (i + 1) % 3]->m_neighbours[1] = m_cube[i].get();

		m_cube[i]->m_neighbours[2] = m_cube[5 - i].get();
		m_cube[5 - i]->m_neighbours[2] = m_cube[i].get();
	}
}

void SpherifiedCube::divide(unsigned depth) {
	for (const auto& plane : m_cube) {
		plane->divide(depth);
	}
}

DWORD SpherifiedCube::createHalf(DWORD point1, DWORD point2) {
	DWORD newInd = static_cast<DWORD>(m_vertices.size());
	m_vertices.push_back(Vertex());
	XMStoreFloat3(&(m_vertices[newInd].position), XMVector3Normalize({
		(m_vertices[point1].position.x + m_vertices[point2].position.x) / 2.f,
		(m_vertices[point1].position.y + m_vertices[point2].position.y) / 2.f,
		(m_vertices[point1].position.z + m_vertices[point2].position.z) / 2.f,
		0
	}) * m_radius);
	return newInd;
}

vector<DWORD> SpherifiedCube::getIndicesBuffer() {
	vector<DWORD> indices;
	for (const auto& plane : m_cube) {
		vector<DWORD> planeIndices = plane->getIndicesBuffer();
		indices.insert(indices.end(), planeIndices.begin(), planeIndices.end());
	}
	return indices;
}

vector<SpherifiedCube::Vertex> SpherifiedCube::getVertices() const {
	return m_vertices;
}
