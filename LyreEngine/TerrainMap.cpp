#include "stdafx.h"

#include "TerrainMap.h"
#include "MapLoader.h"

using namespace DirectX;
using namespace std;

TerrainMap::TerrainMap(const TerrainMap& base, unsigned regionIdx)
	: m_heightMap(HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION) {

	m_heightMapScaledRegions.fill(vector<XMFLOAT4>());

	{
		shared_lock<shared_mutex> baseLocker(base.m_membersLock);

		if (base.m_desc.currentOctaveDepth > 0)
			throw std::runtime_error("Base terrain map is not ready yet.");

		m_desc = base.m_desc;
		m_pMapLoader = base.m_pMapLoader;

		//need to generate one more octave
		m_desc.currentOctaveDepth++;

		switch (regionIdx % 4) {
		case 0:
			// [0] coord remains
			m_desc.quad[1] = base.sampleSphere(0.5f, 0.f);
			m_desc.quad[2] = base.sampleSphere(0.5f, 0.5f);
			m_desc.quad[3] = base.sampleSphere(0.f, 0.5f);
			break;
		case 1:
			m_desc.quad[0] = base.sampleSphere(0.5f, 0.f);
			// [1] coord remains
			m_desc.quad[2] = base.sampleSphere(1.f, 0.5f);
			m_desc.quad[3] = base.sampleSphere(0.5f, 0.5f);
			break;
		case 2:
			m_desc.quad[0] = base.sampleSphere(0.5f, 0.5f);
			m_desc.quad[1] = base.sampleSphere(1.f, 0.5f);
			// [2] coord remains
			m_desc.quad[3] = base.sampleSphere(0.5f, 1.f);
			break;
		case 3:
			m_desc.quad[0] = base.sampleSphere(0.f, 0.5f);
			m_desc.quad[1] = base.sampleSphere(0.5f, 0.5f);
			m_desc.quad[2] = base.sampleSphere(0.5f, 1.f);
			// [3] coord remains
			break;
		}

		if (base.m_heightMapScaledRegions[regionIdx % 4].empty()) {
			base.produceScaledRegions();
		}

		m_heightMap = std::move(base.m_heightMapScaledRegions[regionIdx % 4]);
	}

	m_pMapLoader->pushToQueue(this);
}

TerrainMap::TerrainMap(const Description & desc, MapLoader* pMapLoader)
	: m_heightMap(HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION),
	m_desc(desc),
	m_pMapLoader(pMapLoader) {

	m_heightMapScaledRegions.fill(vector<XMFLOAT4>());

	XMFLOAT3 gridPosition;

	for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
		for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
			gridPosition = sampleSphere(
				j / static_cast<float>(HEIGHTMAP_RESOLUTION - 1),
				i / static_cast<float>(HEIGHTMAP_RESOLUTION - 1)
			);

			XMStoreFloat4(&m_heightMap[i * HEIGHTMAP_RESOLUTION + j],
						  XMVectorSetW(XMVector3Normalize(XMLoadFloat3(&gridPosition)), 0));
		}
	}

	m_pMapLoader->pushToQueue(this);
}

void TerrainMap::nextOctave() {
	m_desc.currentOctaveDepth--;
	m_desc.amplitude /= 2.f;
	m_desc.octave *= 2.f;
}

XMFLOAT3 TerrainMap::sampleSphere(float u, float v) const {
	XMFLOAT3 result;
	float angle = acosf(XMVectorGetX(XMVector3Dot(
		XMVector3Normalize(XMLoadFloat3(&m_desc.quad[1])), 
		XMVector3Normalize(XMLoadFloat3(&m_desc.quad[0])))));
	float radius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_desc.quad[0])));
	XMStoreFloat3(&result, XMVector3Normalize(
		(XMLoadFloat3(&m_desc.quad[1])*sin(u*angle) / sin(angle) +
		 XMLoadFloat3(&m_desc.quad[0])*sin((1-u)*angle) / sin(angle)) * sin((1 - v)*angle) / sin(angle) +
		 (XMLoadFloat3(&m_desc.quad[2])*sin(u*angle) / sin(angle) +
		  XMLoadFloat3(&m_desc.quad[3])*sin((1 - u)*angle) / sin(angle)) * sin(v*angle) / sin(angle))*radius);
	return result;
}

void TerrainMap::loadTerrain(std::vector<XMFLOAT4>& terrain, const std::array<bool, 4>& trueEdges) const {
	shared_lock<shared_mutex> locker(m_membersLock);

	int terrainStartIdx = terrain.size();
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

	const int HALF_SIZE = HEIGHTMAP_RESOLUTION / 2;

	// y, x
	int i, j;
	// child idx
	int ii;
	// read comments below
	int idx[4], idxP[4];

	//row-wise

	for (i = 0; i < HALF_SIZE; i++) {
		for (j = 0; j < HALF_SIZE; j++) {

			// map[x] = average(map[x-1], map[x+1])

			// Indices in child maps:
			// * in maps 2 and 3 we start from second row;
			// * in maps 1 and 2 we start from second column but then
			//   fill previous point (instead of next point for 0 and 3).
			idx[0] = i * 2 * HEIGHTMAP_RESOLUTION + j * 2;
			idx[1] = idx[0] + 1;
			idx[3] = (i * 2 + 1) * HEIGHTMAP_RESOLUTION + j * 2;
			idx[2] = idx[3] + 1;

			// Corresponding starting points in this map
			idxP[0] = i * HEIGHTMAP_RESOLUTION + j;
			idxP[1] = i * HEIGHTMAP_RESOLUTION + (HALF_SIZE + j);
			idxP[2] = (HALF_SIZE + i) * HEIGHTMAP_RESOLUTION + (HALF_SIZE + j);
			idxP[3] = (HALF_SIZE + i) * HEIGHTMAP_RESOLUTION + j;

			for (int ii = 1; ii < 3; ii++) {
				m_heightMapScaledRegions[ii][idx[ii]] =
					m_heightMap[idxP[ii]];
				XMStoreFloat4(&m_heightMapScaledRegions[ii][idx[ii] - 1],
					(XMLoadFloat4(&m_heightMap[idxP[ii]]) +
					 XMLoadFloat4(&m_heightMap[idxP[ii] - 1])) / 2.f);
			}

			for (int ii = 3; ii != 1; ii = (ii + 1) % 4) {
				m_heightMapScaledRegions[ii][idx[ii]] =
					m_heightMap[idxP[ii]];
				XMStoreFloat4(&m_heightMapScaledRegions[ii][idx[ii] + 1],
					(XMLoadFloat4(&m_heightMap[idxP[ii]]) +
					 XMLoadFloat4(&m_heightMap[idxP[ii] + 1])) / 2.f);
			}
		}
	}

	//column-wise

	for (i = 0; i < HALF_SIZE - 1; i++) {
		for (j = 0; j < HEIGHTMAP_RESOLUTION; j++) {

			// map[y] = average(map[y-1], map[y+1])

			// just cache
			for (ii = 0; ii < 4; ii++) {
				idx[ii] = (i * 2 + ii) * HEIGHTMAP_RESOLUTION + j;
			}

			// For child maps 0 and 1 start from second row.
			for (ii = 0; ii < 2; ii++) {
				XMStoreFloat4(&m_heightMapScaledRegions[ii][idx[1]],
					(XMLoadFloat4(&m_heightMapScaledRegions[ii][idx[0]]) +
					 XMLoadFloat4(&m_heightMapScaledRegions[ii][idx[2]])) / 2.f);
			}
			// For child maps 2 and 3 start from third row.
			for (ii = 2; ii < 4; ii++) {
				XMStoreFloat4(&m_heightMapScaledRegions[ii][idx[2]],
					(XMLoadFloat4(&m_heightMapScaledRegions[ii][idx[1]]) +
					 XMLoadFloat4(&m_heightMapScaledRegions[ii][idx[3]])) / 2.f);
			}
		}
	}

	//cut-line row
	for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {

		// map[y] = average(map[y-1], map[y+1]); y E { yMin, yMax }

		// First do it for upper plane:
		// * idx0 - in last row;
		// * idx1 - in (last - 1) row;
		// * idxP1 - second row in lower plane.
		idx[0] = (HEIGHTMAP_RESOLUTION - 1) * HEIGHTMAP_RESOLUTION + j;
		idx[1] = (HEIGHTMAP_RESOLUTION - 2) * HEIGHTMAP_RESOLUTION + j;
		idxP[1] = 1 * HEIGHTMAP_RESOLUTION + j;
		// (idxP[0] = j, esli sho)

		XMStoreFloat4(&m_heightMapScaledRegions[0][idx[0]],
			(XMLoadFloat4(&m_heightMapScaledRegions[0][idx[1]]) +
			 XMLoadFloat4(&m_heightMapScaledRegions[3][idxP[1]])) / 2.f);

		m_heightMapScaledRegions[3][j] = m_heightMapScaledRegions[0][idx[0]];

		XMStoreFloat4(&m_heightMapScaledRegions[1][idx[0]],
			(XMLoadFloat4(&m_heightMapScaledRegions[1][idx[1]]) +
			 XMLoadFloat4(&m_heightMapScaledRegions[2][idxP[1]])) / 2.f);

		m_heightMapScaledRegions[2][j] = m_heightMapScaledRegions[1][idx[0]];
	}
}