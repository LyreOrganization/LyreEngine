#include "stdafx.h"

#include "SpherifiedCube.h"

#include "SpherifiedPlane.h"
#include "TerrainMap.h"

using namespace std;
using namespace DirectX;

namespace {
	const XMFLOAT4X4 CUBE_FACE_ROTATIONS[] = {
		{ // up
			0.f, 0.f, 1.f, 0.f,
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // right
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // front
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // down
			-1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // back
			0.f, -1.f, 0.f, 0.f,
			-1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		},
		{ // left
			0.f, 0.f, -1.f, 0.f,
			0.f, -1.f, 0.f, 0.f,
			-1.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		}
		// (if we look from inside!)
	};
}

SpherifiedCube::SpherifiedCube(float radius, unsigned seed)
	: m_radius(radius), m_mapLoader(this, seed) {
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
		terrainDesc.octave = 1.f;
		terrainDesc.amplitude = 10.f;
		terrainDesc.shift = sqrtf(3.f);
		m_cube[i]->m_pTerrainMap = make_unique<TerrainMap>(m_cube[i].get(), terrainDesc, &m_mapLoader);
	}
}

void SpherifiedCube::loadTopology() {
	unique_lock<shared_mutex> locker(m_topologyLock); // writer
	for (const auto& plane : m_cube) {
		plane->prepareToLoad();
	}
	m_topology.m_planes.clear();
	m_topology.m_planes.reserve(2048 * 4);
	DWORD nextIndexVar = 0;
	for (const auto& plane : m_cube) {
		plane->loadPlane(m_topology.m_planes, nextIndexVar);
	}
	m_topology.m_indices.clear();
	m_topology.m_indices.reserve(2048 * 5);
	m_topology.m_terrain.clear();
	m_topology.m_terrain.reserve(2048 * 64 * 64);
	for (const auto& plane : m_cube) {
		plane->loadIndicesAndTerrain(m_topology.m_indices, m_topology.m_terrain);
	}
}

SphereTopology* SpherifiedCube::getTopology() {
	if (m_topologyLock.try_lock_shared()) return &m_topology;
	return nullptr;
}

void SpherifiedCube::releseTopology() {
	m_topologyLock.unlock_shared();
}

float SpherifiedCube::getRadius() const {
	return m_radius;
}

const XMFLOAT4X4& SpherifiedCube::getFaceRotation(int faceIdx) {
	return CUBE_FACE_ROTATIONS[faceIdx];
}

SpherifiedPlane* SpherifiedCube::getPlane(const SpherifiedPlane::GPUDesc& desc) {
	SpherifiedPlane* pPlane = nullptr;
	SpherifiedPlane* pPlaneNew = m_cube[desc.faceAndFlags & 0xff].get();
	int childIdx = 0;
	for (int i = desc.level - 1; i >= 0; i--) {
		pPlane = pPlaneNew;
		if ((desc.position.x >> i) % 2) {
			if ((desc.position.y >> i) % 2) childIdx = 2;
			else childIdx = 1;
		}
		else {
			if ((desc.position.y >> i) % 2) childIdx = 3;
			else childIdx = 0;
		}
		pPlaneNew = pPlane->m_children[childIdx].get();
		if (pPlaneNew == nullptr) return pPlane;
	}
	return pPlaneNew;
}

SphereTopology::SphereTopology(SphereTopology&& r)
	: m_planes(move(r.m_planes)),
	m_indices(move(r.m_indices)),
	m_terrain(move(r.m_terrain)) {}

SphereTopology& SphereTopology::operator=(SphereTopology&& r) {
	m_planes = move(r.m_planes);
	m_indices = move(r.m_indices);
	m_terrain = move(r.m_terrain);
	return *this;
}
