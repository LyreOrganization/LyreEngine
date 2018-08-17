#include "stdafx.h"

#include "SpherifiedPlane.h"

#include "SpherifiedCube.h"
#include "TerrainMap.h"

using namespace std;
using namespace DirectX;

namespace {

	const int DETAIL_DEPTH = 3;

	constexpr DWORD nextIdx(DWORD index) { return ((index + 1) % 4); }		// next clockwise ement index (of 4)
	constexpr DWORD previousIdx(DWORD index) { return ((index + 3) % 4); }	// previous clockwise ement index (of 4)
	constexpr DWORD oppositeIdx(DWORD index) { return ((index + 2) % 4); }	// opposite ement index (of 4)
}

void SpherifiedPlane::loadTopology(vector<XMFLOAT4>& terrain, 
								   vector<DWORD>& indices, 
								   vector<NeighbourPatchDivision>& neighboursInfo, 
								   vector<int>& divisionInfo) {
	if (m_pTerrainMap == nullptr)
		return;

	if (m_divided) {
		for (const auto& child : m_children) {
			child->loadTopology(terrain, indices, neighboursInfo, divisionInfo);
		}
	}
	else {
		array<bool, 4> trueEdges;

		for (int i = 0; i < 4; i++) {
			indices.push_back(m_points[i]);
		}
		for (int i = 0; i < 4; i++) {
			if (trueEdges[i] = (m_neighbours[i] != nullptr)) {
				neighboursInfo.push_back(m_neighbours[i]->m_divided ?
										 NeighbourPatchDivision::MoreDivided :
										 NeighbourPatchDivision::EquallyDivided);
				indices.push_back(m_neighbours[i]->m_middle);
			}
			else {
				neighboursInfo.push_back(NeighbourPatchDivision::LessDivided);
				indices.push_back(m_pParent->m_neighbours[i]->m_middle);
			}
		}
		indices.push_back(m_middle);

		divisionInfo.push_back(m_level);

		m_pTerrainMap->loadTerrain(terrain, trueEdges);
	}
}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* parent)
	: m_pSphere(sphere), m_points(points), m_pParent(parent), m_pTerrainMap(nullptr) {

	m_middle = m_pSphere->createMidpoint(m_points);
	m_level = parent ? parent->m_level + 1 : 0;
}

bool SpherifiedPlane::tryDivide(int depth) {
	if (depth < 1) return false;

	if (!m_pTerrainMap->isComplete()) return false;

	if (!m_divided) {
		// We can't divide if any of our neighbours missing, so we try to 
		// divede respective parent's neighbour to create our own one.
		for (int i = 0; i < 4; i++) {
			if (m_neighbours[i] == nullptr && !m_pParent->m_neighbours[i]->tryDivide()) {
				return false;
			}
		}

		//creating halfs if they don't exist
		for (int i = 0; i < 4; i++) {
			if (!m_halfs[i]) {
				m_halfs[i] = m_pSphere->createHalf(m_points[i], m_points[nextIdx(i)]);
				if (m_neighbours[i]->m_neighbours[oppositeIdx(i)] == this) // we are on same cube face
					m_neighbours[i]->m_halfs[oppositeIdx(i)] = m_halfs[i].value();
				else
					m_neighbours[i]->m_halfs[i == 0 ? 3 : (i == 3 ? 0 : i)] = m_halfs[i].value();
			}
		}

		//creating children
		m_children[0] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_points[0],
			m_halfs[0].value(),
			m_middle,
			m_halfs[3].value()
		}, this);
		m_children[1] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_halfs[0].value(),
			m_points[1],
			m_halfs[1].value(),
			m_middle
		}, this);
		m_children[2] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_middle,
			m_halfs[1].value(),
			m_points[2],
			m_halfs[2].value()
		}, this);
		m_children[3] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_halfs[3].value(),
			m_middle,
			m_halfs[2].value(),
			m_points[3]
		}, this);

		//setting neighbours
		for (int i = 0; i < 4; i++) {
			// inner
			m_children[i]->m_neighbours[nextIdx(i)] = m_children[nextIdx(i)].get();
			m_children[i]->m_neighbours[oppositeIdx(i)] = m_children[previousIdx(i)].get();
			// outer
			if (m_neighbours[i]->m_divided) {
				if (m_neighbours[i]->m_neighbours[oppositeIdx(i)] == this) // we are on same cube face
				{
					m_children[i]->m_neighbours[i] = m_neighbours[i]->m_children[previousIdx(i)].get();
					m_children[i]->m_neighbours[i]->m_neighbours[oppositeIdx(i)] = m_children[i].get();
				}
				else {
					const int adjIdx = i ? i % 3 + 1 : 0;
					m_children[i]->m_neighbours[i] = m_neighbours[i]->m_children[adjIdx].get();
					m_children[i]->m_neighbours[i]->m_neighbours[previousIdx(adjIdx)] = m_children[i].get();
				}
			}
			if (m_neighbours[previousIdx(i)]->m_divided) {
				if (m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] == this) // we are on same cube face
				{
					m_children[i]->m_neighbours[previousIdx(i)] = m_neighbours[previousIdx(i)]->m_children[nextIdx(i)].get();
					m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] = m_children[i].get();
				}
				else {
					const int adjIdx = i ? (i + 1) % 3 + 1 : 0;
					m_children[i]->m_neighbours[previousIdx(i)] = m_neighbours[previousIdx(i)]->m_children[adjIdx].get();
					m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[adjIdx] = m_children[i].get();
				}
			}
		}

		//setting terrain map
		for (int i = 0; i < 4; ++i) {
			m_children[i]->m_pTerrainMap = make_unique<TerrainMap>(*m_pTerrainMap, i);
		}

		m_divided = true;
	}

	for (int i = 0; i < 4; ++i)
		m_children[i]->tryDivide(depth - 1);

	return true;
}

bool SpherifiedPlane::tryUndivide() {
	if (!m_divided) return true;

	bool bChildrenAreLeaves = true;

	for (int i = 0; i < 4; i++) {
		if (m_children[i]->m_divided && !m_children[i]->tryUndivide()) {
			bChildrenAreLeaves = false;
		}
	}

	if (!bChildrenAreLeaves)
		return bChildrenAreLeaves;


}

//void SpherifiedPlane::generateTerrain(int idx, int nOctaves) {
//	m_pTerrainMap = make_unique<TerrainMap>();
//
//	if (m_pParent != nullptr) {
//		idx %= 4;
//		m_pTerrainMap
//	}
//
//	if (m_octave < 0)
//		return;
//
		//auto& r_vertices = m_pSphere->vertices();

		//for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
		//	for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
		//		XMFLOAT3 originalPosition = sampleSphere(
		//			j / static_cast<float>(HEIGHTMAP_RESOLUTION - 1),
		//			i / static_cast<float>(HEIGHTMAP_RESOLUTION - 1)
		//		);

		//		XMVECTOR original = XMLoadFloat3(&originalPosition);
		//		XMVECTOR normal = XMVector3Normalize(original);
		//		XMVECTOR surfaceDerivative = XMVectorZero();
		//		XMFLOAT3 scaledPos;
		//		float height = 0;
		//		for (int i = 0; i < 3/*iMAX*/; i++) { // fractal, iMAX octaves
		//			XMStoreFloat3(&scaledPos, original * (float)(1 << (i + 2/*octave*/)));
		//			XMFLOAT4 perlin = g_noise.perlinNoise(scaledPos);
		//			XMVECTOR vecNormal = XMLoadFloat4(&perlin);

		//			//basic
		//			height += perlin.w / (float)(1 << (i + 4/*amplitude*/));
		//			surfaceDerivative += vecNormal / (float)(1 << (i + 1/*amplitude*/));

		//			////erosion
		//			//height += abs(perlin.w) / (float)(1 << (i + 2/*amplitude*/));
		//			//float smooth = fabs(perlin.w) > 0.1f ? 1.f : (perlin.w / 0.1f);
		//			//smooth = smooth*smooth;
		//			//surfaceDerivative += (perlin.w > 0 ? vecNormal : -vecNormal) * smooth / (float)(1 << (i + 2/*amplitude*/));

		//			////ridges
		//			//height += -abs(perlin.w) / (float)(1 << (i + 2/*amplitude*/));
		//			//float smooth = fabs(perlin.w) > 0.1f ? 1.f : (perlin.w / 0.1f);
		//			//smooth = smooth*smooth;
		//			//surfaceDerivative += (perlin.w > 0 ? -vecNormal : vecNormal) * smooth / (float)(1 << (i + 2/*amplitude*/));

		//			////plates
		//			//float plate = floor(perlin.w * 4.f) / 4.f;
		//			//float rest = perlin.w - plate;
		//			//float smoothA = 1.f - rest / 0.25f;
		//			//float smoothB = 1.f - (0.25f - rest) / 0.25f;
		//			//if (smoothA > smoothB) {
		//			//	//smoothA *= smoothA*smoothA;
		//			//	height += (plate + rest*smoothA*2.f) / (float)(1 << (i + 2/*amplitude*/));
		//			//	surfaceDerivative += vecNormal * smoothA / (float)(1 << (i + 2/*amplitude*/));
		//			//}
		//			//else {
		//			//	//smoothB *= smoothB*smoothB;
		//			//	height += (plate + 0.25f - rest*smoothB*2.f) / (float)(1 << (i + 2/*amplitude*/));
		//			//	surfaceDerivative -= vecNormal * smoothB / (float)(1 << (i + 2/*amplitude*/));
		//			//}

		//		}
		//		XMStoreFloat4(&m_terrainMap.value()[j + i * HEIGHTMAP_RESOLUTION], XMVectorSetW(
		//			XMVector3Normalize(normal - (surfaceDerivative - XMVector3Dot(surfaceDerivative, normal) * normal)),
		//			height));
		//	}
		//}

		//if (m_divided) {
		//	for (const auto& child : m_children)
		//		child->generateTerrain(m_pTerrainMap);
		//}
//}

