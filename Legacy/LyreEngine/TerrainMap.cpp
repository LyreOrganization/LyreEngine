#include "stdafx.h"

#include "TerrainMap.h"
#include "MapLoader.h"
#include "SpherifiedCube.h"

using namespace DirectX;
using namespace std;

namespace {
	inline float simplifyAndSolveFraction(__int32 numerator, __int32 denominator) {
		return static_cast<float>(static_cast<double>(numerator) / denominator);
	}

	static const auto RES = TerrainMap::RESOLUTION;
	static const auto HALFRES = RES / 2;
}

void TerrainMap::alignEdgeNormals() {
	static const auto setupIteration = []
	(int edge, int& initial, int& step, bool indexingOrder) {
		switch (edge) {
		case 0: initial = 0; step = 1; break;
		case 1: initial = RES - 1; step = RES; break;
		case 2: initial = (RES - 1) * RES; step = 1; break;
		case 3: initial = 0; step = RES; break;
		}
		if (!indexingOrder) {
			// reverse indexing order
			initial += step*(RES - 1);
			step = -step;
		}
	};

	for (int neigh = 0; neigh < 4; neigh++) {
		auto* neighbour = m_pPlane->m_neighbours[neigh];
		if (neighbour && neighbour->m_pTerrainMap->isComplete) {
			pair<int, bool> neighbourEdge = m_pPlane->getNeighboursNeighbourIndexOfThis(neigh);

			int initial, step;
			int initialNeighbour, stepNeighbour;
			setupIteration(neigh, initial, step, true);
			setupIteration(neighbourEdge.first, initialNeighbour, stepNeighbour, neighbourEdge.second);

			auto& neighbourMap = neighbour->m_pTerrainMap->m_heightMap;
			for (int iter = 0, i = initial, j = initialNeighbour;
				 iter < RES; iter++, i += step, j += stepNeighbour) {
				auto& normal = m_heightMap[i].data.normal;
				auto& normalNeighbour = neighbourMap[j].data.normal;
				XMVECTOR average = XMVector3Normalize(
					XMLoadFloat3(&normal) + XMLoadFloat3(&normalNeighbour));
				XMStoreFloat3(&normal, average);
				XMStoreFloat3(&normalNeighbour, average);
			}
		}
	}
}

TerrainMap::TerrainMap(const TerrainMap& base, unsigned regionIdx)
	: m_pPlane(base.m_pPlane->m_children[regionIdx].get()),
	m_desc(base.m_baseDesc), m_pMapLoader(base.m_pMapLoader),
	m_octavesToGenerate(CHILDREN_DEPTH), m_bComplete(false) {

	if (base.m_baseScaledRegions[regionIdx].empty()) {
		base.produceScaledRegions();
	}
	m_heightMap = std::move(base.m_baseScaledRegions[regionIdx]);
	for (int i = 0; i < 4; i++)
		m_edgesBackup[i] = std::move(base.m_backupEdgesScaled[regionIdx][i]);

	m_pMapLoader->pushToQueue(this);
}

TerrainMap::TerrainMap(SpherifiedPlane* plane, const Description& desc, MapLoader* pMapLoader)
	: m_heightMap(RESOLUTION * RESOLUTION, HeightmapTexel()),
	m_pPlane(plane), m_desc(desc), m_pMapLoader(pMapLoader),
	m_octavesToGenerate(GRID_DEPTH), m_bComplete(false) {

	for (auto& edge : m_edgesBackup) edge = HeightMapT(RESOLUTION, HeightmapTexel());

	m_pMapLoader->pushToQueue(this);
}

void TerrainMap::nextOctave() {
	m_desc.amplitude /= 2.f;
	m_desc.octave *= 2.f;
	if (m_octavesToGenerate-- == CHILDREN_DEPTH) {
		m_childrenBase = m_heightMap;
		m_baseDesc = m_desc;
	}
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
	xIndex += m_pPlane->m_planeIndex.x * (RESOLUTION - 1);
	yIndex += m_pPlane->m_planeIndex.y * (RESOLUTION - 1);

	// count global resolution
	int maxIndex = ((RESOLUTION - 1) << m_pPlane->m_level); // = resolution - 1

	XMFLOAT3 result {
		1.f - simplifyAndSolveFraction(xIndex, maxIndex) * 2.f,
		1.f - simplifyAndSolveFraction(yIndex, maxIndex) * 2.f,
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
	int terrainStartIdx = static_cast<int>(terrain.size());
	terrain.insert(terrain.end(), m_heightMap.begin(), m_heightMap.end());

	if (!trueEdges[0]) {
		for (int i = 0; i < RESOLUTION; i++)
			terrain[terrainStartIdx + i] = m_edgesBackup[0][i].f4;
	}
	if (!trueEdges[2]) {
		for (int i = 0; i < RESOLUTION; i++)
			terrain[terrainStartIdx + (RESOLUTION - 1) * RESOLUTION + i] = m_edgesBackup[2][i].f4;
	}
	if (!trueEdges[3]) {
		for (int i = 0; i < RESOLUTION; i++)
			terrain[terrainStartIdx + i * RESOLUTION] = m_edgesBackup[3][i].f4;
	}
	if (!trueEdges[1]) {
		for (int i = 0; i < RESOLUTION; i++)
			terrain[terrainStartIdx + (i + 1) * RESOLUTION - 1] = m_edgesBackup[1][i].f4;
	}
}

void TerrainMap::produceScaledRegions() const {
	for (int regionIdx = 0; regionIdx < 4; regionIdx++) {
		auto& region = (m_baseScaledRegions[regionIdx] = HeightMapT(RES * RES));

		int firstTexelJ = (regionIdx == 0 || regionIdx == 3 ? 0 : HALFRES);
		int firstTexelI = (regionIdx == 0 || regionIdx == 1 ? 0 : HALFRES);

		//row-wise
		for (int parentTexel = firstTexelI * RES + firstTexelJ, childTexel = 0;
			 childTexel < RES * RES; parentTexel += HALFRES, childTexel += RES) {
			for (int j = 0; j < HALFRES; j++, parentTexel++, childTexel += 2) {
				region[childTexel] = m_childrenBase[parentTexel];
				XMStoreFloat4(&region[childTexel + 1].f4,
					(XMLoadFloat4(&region[childTexel].f4) +
					 XMLoadFloat4(&m_childrenBase[parentTexel + 1].f4)) / 2.f);
			}
			//right edge texel
			region[childTexel++] = m_childrenBase[parentTexel++];
		}

		//column-wise
		for (int texel = RES; texel < RES * RES; texel += RES) {
			for (int j = 0; j < RES; j++, texel++) {
				XMStoreFloat4(&region[texel].f4,
					(XMLoadFloat4(&region[texel - RES].f4) +
					 XMLoadFloat4(&region[texel + RES].f4)) / 2.f);
			}
		}

		//backuping edges
		auto& edges = m_backupEdgesScaled[regionIdx];
		for (auto& edge : edges) edge = HeightMapT(RES);
		int firstTexel = firstTexelI * RES + firstTexelJ;
		for (int parentTexel = 0, childTexel = 0; childTexel < RES; parentTexel++, childTexel += 2) {
			edges[0][childTexel] = m_heightMap[firstTexel + parentTexel];
			edges[1][childTexel] = m_heightMap[firstTexel + parentTexel * RES + HALFRES];
			edges[2][childTexel] = m_heightMap[firstTexel + HALFRES * RES + parentTexel];
			edges[3][childTexel] = m_heightMap[firstTexel + parentTexel * RES];
		}
		for (int i = 1; i < RES; i += 2) {
			for (auto& edge : edges) XMStoreFloat4(&edge[i].f4,
				(XMLoadFloat4(&edge[i - 1].f4) + XMLoadFloat4(&edge[i + 1].f4)) / 2.f);
		}
	}
}
