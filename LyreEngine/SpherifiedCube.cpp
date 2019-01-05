#include "stdafx.h"

#include "SpherifiedCube.h"

#include "SpherifiedPlane.h"
#include "TerrainMap.h"

using namespace std;
using namespace DirectX;

namespace {
	const XMFLOAT4X4 CUBE_FACE_ROTATIONS[] = {
		{ // up
			0.f, 0.f, -1.f, 0.f,
			-1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // right
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // front
			-1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // down
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // back
			0.f, 1.f, 0.f, 0.f,
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // left
			0.f, 0.f, 1.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			-1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		}
		// (if we look from inside!)
	};
}

SpherifiedCube::SpherifiedCube(float radius, unsigned seed)
	: m_radius(radius), m_mapLoader(seed) {
	buildCube();
}

SpherifiedCube::~SpherifiedCube() {}

void SpherifiedCube::buildCube() {
	// up, right, front, down, back, left
	for (int i = 0; i < 6; i++) {
		m_cube[i] = make_unique<SpherifiedPlane>(this, i);
	}

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

	m_mapLoader.start();

	//terrain
	TerrainMap::Description terrainDesc;
	for (int i = 0; i < 6; i++) {
		terrainDesc.octave = 0.5f;
		terrainDesc.amplitude = 0.5f;
		terrainDesc.shift = sqrtf(3.f);
		terrainDesc.currentOctaveDepth = 3;
		m_cube[i]->m_pTerrainMap = make_unique<TerrainMap>(m_cube[i].get(), terrainDesc, &m_mapLoader);
	}

	auto pPlane = m_cube[4].get();
	this_thread::sleep_for(chrono::milliseconds(2000));
	for (int i = 0; i < 4; i++) {
		pPlane->tryDivide();
		pPlane = pPlane->m_children[i % 4].get();
		this_thread::sleep_for(chrono::milliseconds(1500 + i * 50));
	}
	applyTopology();
}

void SpherifiedCube::applyTopology() {
	planes.clear();
	terrain.clear();
	DWORD nextIndexVar = 0;
	for (const auto& plane : m_cube) {
		plane->loadPlane(planes, nextIndexVar);
	}
	indices.clear();
	for (const auto& plane : m_cube) {
		plane->loadIndicesAndTerrain(indices, terrain);
	}
}

float SpherifiedCube::getRadius() const {
	return m_radius;
}

const XMFLOAT4X4& SpherifiedCube::getFaceRotation(int faceIdx) {
	return CUBE_FACE_ROTATIONS[faceIdx];
}

