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
	const float CUBE_COORDS = m_radius / sqrt(3.f);
	m_vertices.assign({
		// front
		{ { -CUBE_COORDS, -CUBE_COORDS,  CUBE_COORDS } },
		{ { CUBE_COORDS, -CUBE_COORDS,  CUBE_COORDS } },
		{ { CUBE_COORDS,  CUBE_COORDS,  CUBE_COORDS } },
		{ { -CUBE_COORDS,  CUBE_COORDS,  CUBE_COORDS } },
		// back
		{ { -CUBE_COORDS, -CUBE_COORDS, -CUBE_COORDS } },
		{ { CUBE_COORDS, -CUBE_COORDS, -CUBE_COORDS } },
		{ { CUBE_COORDS,  CUBE_COORDS, -CUBE_COORDS } },
		{ { -CUBE_COORDS,  CUBE_COORDS, -CUBE_COORDS } },
	});

	m_cube = {
		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 2, 6, 7, 3 }),	// up
		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 2, 1, 5, 6 }),	// right
		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 2, 3, 0, 1 }),	// front

		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 4, 5, 1, 0 }),	// down	
		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 4, 7, 6, 5 }),	// back
		make_unique<SpherifiedPlane>(this, SpherifiedPlane::DWORD4{ 4, 0, 3, 7 }),	// left
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
	XMFLOAT3 sum {
		m_vertices[point1].position.x + m_vertices[point2].position.x,
		m_vertices[point1].position.y + m_vertices[point2].position.y,
		m_vertices[point1].position.z + m_vertices[point2].position.z
	};
	XMStoreFloat3(&(m_vertices[newInd].position), XMVector3Normalize(XMLoadFloat3(&sum)) * m_radius);
	return newInd;
}

DWORD SpherifiedCube::createMidpoint(const SpherifiedPlane::DWORD4& points) {
	DWORD newInd = static_cast<DWORD>(m_vertices.size());
	m_vertices.push_back(Vertex());
	XMVECTOR sum = XMVectorZero();
	for (auto& index : points) {
		sum += XMLoadFloat3(&m_vertices[index].position);
	}
	XMStoreFloat3(&(m_vertices[newInd].position), XMVector3Normalize(sum) * m_radius);
	return newInd;
}

const vector<SpherifiedCube::Vertex>& SpherifiedCube::vertices() {
	return m_vertices;
}

void SpherifiedCube::distort() {
	for (const auto& plane : m_cube)
		plane->generateTerrain();
}

void SpherifiedCube::applyTopology() {
	indices.clear();
	terrain.clear();
	for (const auto& plane : m_cube) {
		plane->loadTopology(terrain, indices);
	}
}

float SpherifiedCube::getRadius() const {
	return m_radius;
}
