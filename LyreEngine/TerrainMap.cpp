#include "stdafx.h"

#include "TerrainMap.h"
#include "MapLoader.h"
#include "SpherifiedPlane.h"
#include "SpherifiedCube.h"

using namespace DirectX;
using namespace std;

namespace {
	inline float simplifyAndSolveFraction(__int32 numerator, __int32 denominator) {
		return static_cast<float>(static_cast<double>(numerator) / denominator);
	}
}

TerrainMap::TerrainMap(const TerrainMap& base, unsigned regionIdx)
	: m_pPlane(base.m_pPlane->m_children[regionIdx].get()) {

	shared_lock<shared_mutex> baseLocker(base.m_membersLock);

	if (base.m_desc.currentOctaveDepth > 0)
		throw std::runtime_error("Base terrain map is not ready yet.");

	m_desc = base.m_desc;
	m_pMapLoader = base.m_pMapLoader;

	//need to generate one more octave
	m_desc.currentOctaveDepth++;

	if (base.m_heightMapScaledRegions[regionIdx].empty()) {
		base.produceScaledRegions();
	}
	m_heightMap = std::move(base.m_heightMapScaledRegions[regionIdx]);

	baseLocker.unlock(); // avoid holding two locks (pushToQueue() locks MapLoader)
	m_pMapLoader->pushToQueue(this);
}

TerrainMap::TerrainMap(SpherifiedPlane* plane, const Description& desc, MapLoader* pMapLoader)
	: m_heightMap(HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION, XMFLOAT4 { 0.f, 0.f, 0.f, 0.f }),
	m_desc(desc),
	m_pPlane(plane),
	m_pMapLoader(pMapLoader) {

	m_edgesBackup.fill(HeightMapType(HEIGHTMAP_RESOLUTION, XMFLOAT4 { 0.f, 0.f, 0.f, 0.f }));

	m_pMapLoader->pushToQueue(this);
}

void TerrainMap::nextOctave() {
	m_desc.currentOctaveDepth--;
	m_desc.amplitude /= 2.f;
	m_desc.octave *= 2.f;
}

// Map a cube to a sphere ~ map a square to a circle (x,y -> x',y').
// Constant x line maps to an ellipse curve in the circle (-1 <= x,y,x',y' <= 1).
// Ellipse eq. then should be (x'/x)^2 + (y'/b)^2 = 1.
// When x = 1, x',y' is on the circle and = (x/sqrt(2), sqrt(1-x*x/2)).
// Paste that into ellipse eq. to find b = sqrt(2-x*x).
// Paste that to "const x line" ellipse eq. to find x' = f(y', x).
// Do the same for "const y line" ellipse eq. The system of these 2 eq. produces:
// x' = x*sqrt(1-y*y/2); y' = y*sqrt(1-x*x/2).
// Return to a sphere: x'^2 + y'^2 + z'^2 = 1. Solving that for a sphere produces:
// x' = x * sqrt(1 - y*y/2 - z*z/2 + y*y*z*z/3); eq. for y' and z' are symmetrical to this.

XMFLOAT3 TerrainMap::sampleSphere(__int32 xIndex, __int32 yIndex) const {
	// convert index to global
	xIndex += m_pPlane->m_planeIndex.x * (HEIGHTMAP_RESOLUTION - 1);
	yIndex += m_pPlane->m_planeIndex.y * (HEIGHTMAP_RESOLUTION - 1);

	// count global resolution
	int maxIndex = ((HEIGHTMAP_RESOLUTION - 1) << m_pPlane->m_level); // = resolution - 1

	XMFLOAT3 result {
		simplifyAndSolveFraction(xIndex, maxIndex) * 2.f - 1.f,
		simplifyAndSolveFraction(yIndex, maxIndex) * 2.f - 1.f,
		// let z be 1 (front face), than rotate it to become needed face
		1.f
	};

	float x2 = result.x*result.x;
	float y2 = result.y*result.y;
	//float z2 = 1.f*1.f = 1.f

	result.x *= sqrtf(1.f - (y2 + 1.f) / 2.f + y2 / 3.f);
	result.y *= sqrtf(1.f - (x2 + 1.f) / 2.f + x2 / 3.f);
	result.z = sqrtf(1.f - (x2 + y2) / 2.f + x2*y2 / 3.f);

	XMStoreFloat3(&result, XMVector3Transform(
		XMLoadFloat3(&result),
		XMLoadFloat4x4(&SpherifiedCube::getFaceRotation(m_pPlane->m_faceIdx))));
	return result;
}

void TerrainMap::loadTerrain(std::vector<XMFLOAT4>& terrain, const std::array<bool, 4>& trueEdges) const {
	shared_lock<shared_mutex> locker(m_membersLock);

	int terrainStartIdx = static_cast<int>(terrain.size());
	terrain.insert(terrain.end(), m_heightMap.begin(), m_heightMap.end());

	if (!trueEdges[0]) {
		for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++)
			terrain[terrainStartIdx + i] = m_edgesBackup[0][i];
	}
	if (!trueEdges[2]) {
		for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++)
			terrain[terrainStartIdx + (HEIGHTMAP_RESOLUTION - 1) * HEIGHTMAP_RESOLUTION + i] = m_edgesBackup[2][i];
	}
	if (!trueEdges[3]) {
		for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++)
			terrain[terrainStartIdx + i * HEIGHTMAP_RESOLUTION] = m_edgesBackup[3][i];
	}
	if (!trueEdges[1]) {
		for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++)
			terrain[terrainStartIdx + (i + 1) * HEIGHTMAP_RESOLUTION - 1] = m_edgesBackup[1][i];
	}
}

bool TerrainMap::isComplete() const {
	shared_lock<shared_mutex> locker(m_membersLock);
	return m_desc.currentOctaveDepth <= 0;
}

void TerrainMap::produceScaledRegions() const {
	m_heightMapScaledRegions.fill(HeightMapType(HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION));

	for (int regionIdx = 0; regionIdx < 4; regionIdx++) {
		auto& region = m_heightMapScaledRegions[regionIdx];

		int firstTexelJ = (regionIdx == 0 || regionIdx == 3 ? 0 : HEIGHTMAP_RESOLUTION / 2);
		int firstTexelI = (regionIdx == 0 || regionIdx == 1 ? 0 : HEIGHTMAP_RESOLUTION / 2);

		//row-wise
		for (int i = 0; i <= HEIGHTMAP_RESOLUTION / 2; i++) {
			for (int j = 0; j < HEIGHTMAP_RESOLUTION / 2; j++) {
				region[i * 2 * HEIGHTMAP_RESOLUTION + j * 2] =
					m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + j];
				XMStoreFloat4(&region[i * 2 * HEIGHTMAP_RESOLUTION + j * 2 + 1],
					(XMLoadFloat4(&region[i * 2 * HEIGHTMAP_RESOLUTION + j * 2]) +
					 XMLoadFloat4(&m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + j + 1])) / 2.f);
			}
			//right edge texel
			region[i * 2 * HEIGHTMAP_RESOLUTION + HEIGHTMAP_RESOLUTION - 1] =
				m_heightMap[(firstTexelI + i)*HEIGHTMAP_RESOLUTION + firstTexelJ + HEIGHTMAP_RESOLUTION / 2];
		}

		//column-wise
		for (int i = 0; i < HEIGHTMAP_RESOLUTION / 2; i++) {
			for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
				XMStoreFloat4(&region[(i * 2 + 1) * HEIGHTMAP_RESOLUTION + j],
					(XMLoadFloat4(&region[i * 2 * HEIGHTMAP_RESOLUTION + j]) +
					 XMLoadFloat4(&region[(i * 2 + 2) * HEIGHTMAP_RESOLUTION + j])) / 2.f);
			}
		}
	}
}
