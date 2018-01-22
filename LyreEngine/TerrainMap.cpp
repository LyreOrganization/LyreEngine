#include "stdafx.h"

#include "TerrainMap.h"
#include "MapLoader.h"

using namespace DirectX;
using namespace std;

TerrainMap::TerrainMap(const TerrainMap& base, unsigned regionIdx)
	: m_heightMap(HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION),
	m_state(State::NoMap), m_desc(base.m_desc), m_pMapLoader(base.m_pMapLoader) {

	if (base.m_desc.currentOctaveDepth > 0)
		throw std::runtime_error("Base terrain map is not ready yet.");

	if (++(m_desc.currentOctaveDepth) > 0) {
		//need to generate one more octave
		nextOctave();
	}

	switch (regionIdx % 4) {
	case 0:
		m_desc.quad[1] = base.sampleSphere(0.5f, 0.f);
		m_desc.quad[2] = base.sampleSphere(0.5f, 0.5f);
		m_desc.quad[3] = base.sampleSphere(0.f, 0.5f);
		break;
	case 1:
		m_desc.quad[0] = base.sampleSphere(0.5f, 0.f);
		m_desc.quad[2] = base.sampleSphere(1.f, 0.5f);
		m_desc.quad[3] = base.sampleSphere(0.5f, 0.5f);
		break;
	case 2:
		m_desc.quad[0] = base.sampleSphere(0.5f, 0.5f);
		m_desc.quad[1] = base.sampleSphere(1.f, 0.5f);
		m_desc.quad[3] = base.sampleSphere(0.5f, 1.f);
		break;
	case 3:
		m_desc.quad[0] = base.sampleSphere(0.f, 0.5f);
		m_desc.quad[1] = base.sampleSphere(0.5f, 0.5f);
		m_desc.quad[2] = base.sampleSphere(0.5f, 1.f);
		break;
	}

	int firstTexelJ = (regionIdx == 0 || regionIdx == 3 ? 0 : HEIGHTMAP_RESOLUTION / 2);
	int firstTexelI = (regionIdx == 0 || regionIdx == 1 ? 0 : HEIGHTMAP_RESOLUTION / 2);
	//row-wise
	for (int i = 0; i <= HEIGHTMAP_RESOLUTION / 2; i++) {
		for (int j = 0; j < HEIGHTMAP_RESOLUTION / 2; j++) {
			m_heightMap[i * 2 * HEIGHTMAP_RESOLUTION + j * 2] =
				base.m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + j];
			XMStoreFloat4(&m_heightMap[i * 2 * HEIGHTMAP_RESOLUTION + j * 2 + 1],
				(XMLoadFloat4(&m_heightMap[i * 2 * HEIGHTMAP_RESOLUTION + j * 2]) +
				 XMLoadFloat4(&base.m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + j + 1])) / 2.f);
		}
		//right edge texel
		m_heightMap[i * 2 * HEIGHTMAP_RESOLUTION + HEIGHTMAP_RESOLUTION - 1] =
			base.m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + HEIGHTMAP_RESOLUTION / 2];
	}
	//column-wise
	for (int i = 0; i < HEIGHTMAP_RESOLUTION / 2; i++) {
		for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
			XMStoreFloat4(&m_heightMap[(i * 2 + 1) * HEIGHTMAP_RESOLUTION + j],
				(XMLoadFloat4(&m_heightMap[i * 2 * HEIGHTMAP_RESOLUTION + j]) +
				 XMLoadFloat4(&m_heightMap[(i * 2 + 2) * HEIGHTMAP_RESOLUTION + j])) / 2.f);
		}
	}
}

TerrainMap::TerrainMap(const Description & desc, MapLoader* pMapLoader)
	: m_heightMap(HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION, { 0, 0, 0, 0 }),
	m_state(State::NoMap), m_desc(desc), m_pMapLoader(pMapLoader) {}

void TerrainMap::nextOctave() {
	m_desc.amplitude /= 2.f;
	m_desc.octave *= 2.f;
	m_desc.shift *= sqrt(3); //sqrt(3) has no meaning, just random, replace it later
}

XMFLOAT3 TerrainMap::sampleSphere(float u, float v) const {
	XMFLOAT3 result;
	float radius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_desc.quad[0])));
	XMStoreFloat3(&result, XMVector3ClampLength(
		(XMLoadFloat3(&m_desc.quad[1])*u +
		 XMLoadFloat3(&m_desc.quad[0])*(1 - u))*(1-v) +
		 (XMLoadFloat3(&m_desc.quad[2])*u +
		  XMLoadFloat3(&m_desc.quad[3])*(1 - u))*v,
		radius, radius));
	return result;
}

void TerrainMap::loadTerrain(std::vector<XMFLOAT4>& terrain) {
	unique_lock<mutex> locker(m_membersLock);

	if (m_state == State::NoMap) {
		m_pMapLoader->pushToQueue(this);
	}
	else if (m_state == State::DeleteMark) {
		m_state = State::MapReady;
	}

	// TODO: seamless LOD
	terrain.insert(terrain.end(), m_heightMap.begin(), m_heightMap.end());
}

TerrainMap::State TerrainMap::getState() const {
	unique_lock<mutex> locker(m_membersLock);
	return m_state;
}
