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

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, int faceIdx)
	: m_pSphere(sphere),
	m_faceIdx(faceIdx),
	m_level(0),
	m_planeIndex { 0, 0 },
	m_pParent(nullptr),
	m_divided(false) {}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, SpherifiedPlane* parent,
								 XMINT2 childPlaneIndex)
	: m_pSphere(sphere),
	m_faceIdx(parent->m_faceIdx),
	m_level(parent->m_level + 1),
	m_planeIndex {
	parent->m_planeIndex.x * 2 + childPlaneIndex.x,
	parent->m_planeIndex.y * 2 + childPlaneIndex.y },
	m_pParent(parent),
	m_divided(false) {}

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

		//creating children
		m_children[0] = make_unique<SpherifiedPlane>(m_pSphere, this, XMINT2 { 0, 0 });
		m_children[1] = make_unique<SpherifiedPlane>(m_pSphere, this, XMINT2 { 1, 0 });
		m_children[2] = make_unique<SpherifiedPlane>(m_pSphere, this, XMINT2 { 1, 1 });
		m_children[3] = make_unique<SpherifiedPlane>(m_pSphere, this, XMINT2 { 0, 1 });

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

void SpherifiedPlane::loadPlane(vector<GPUDesc>& planes,
								DWORD& nextIndex) {
	if (m_pTerrainMap == nullptr)
		return;

	if (m_divided) {
		for (const auto& child : m_children) {
			child->loadPlane(planes, nextIndex);
		}
	}

	GPUDesc planeDesc;
	// Face index is in first byte.
	planeDesc.faceAndFlags = m_faceIdx;
	planeDesc.level = m_level;
	planeDesc.position = m_planeIndex;
	for (int i = 0; i < 4; i++) {
		if (terrainEdgesToLoad[i] = (m_neighbours[i] != nullptr)) {
			// First 4 bits of second byte are 
			// set if corresponding neighbour is more divided.
			if (m_neighbours[i]->m_divided)
				planeDesc.faceAndFlags |= 1 << (8 + i);
		}
	}

	planes.push_back(move(planeDesc));
	indexToLoad = nextIndex++;
}

void SpherifiedPlane::loadIndicesAndTerrain(std::vector<DWORD>& indices,
								  vector<DirectX::XMFLOAT4>& terrain) {
	if (m_divided) {
		for (const auto& child : m_children) {
			child->loadIndicesAndTerrain(indices, terrain);
		}
	}
	else
	{ 
		if (indexToLoad == (DWORD)-1) return;
		indices.push_back(indexToLoad);
		for (int i = 0; i < 4; i++) {
			if (m_neighbours[i] == nullptr) 
				indices.push_back(m_pParent->m_neighbours[i]->indexToLoad);
			else indices.push_back(m_neighbours[i]->indexToLoad);
		}
		m_pTerrainMap->loadTerrain(terrain, terrainEdgesToLoad);
	}
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

