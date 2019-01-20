#include "stdafx.h"

#include "SpherifiedPlane.h"

#include "SpherifiedCube.h"
#include "TerrainMap.h"

using namespace std;
using namespace DirectX;

namespace {

	const int DETAIL_DEPTH = 3;

	constexpr DWORD nextIdx(DWORD index) { return ((index + 1) % 4); }		// next clockwise element index (of 4)
	constexpr DWORD previousIdx(DWORD index) { return ((index + 3) % 4); }	// previous clockwise element index (of 4)
	constexpr DWORD oppositeIdx(DWORD index) { return ((index + 2) % 4); }	// opposite element index (of 4)
}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, int faceIdx)
	: m_pSphere(sphere),
	m_faceIdx(faceIdx),
	m_level(0),
	m_planeIndex { 0, 0 },
	m_pParent(nullptr){}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, SpherifiedPlane* parent,
								 XMINT2 childPlaneIndex)
	: m_pSphere(sphere),
	m_faceIdx(parent->m_faceIdx),
	m_level(parent->m_level + 1),
	m_planeIndex {
	(parent->m_planeIndex.x << 1) + childPlaneIndex.x,
	(parent->m_planeIndex.y << 1) + childPlaneIndex.y },
	m_pParent(parent) {}

pair<int, bool> SpherifiedPlane::getNeighboursNeighbourIndexOfThis(int neighbour) {
	if (m_neighbours[neighbour]->m_neighbours[oppositeIdx(neighbour)] == this)
		return make_pair<int, bool>(oppositeIdx(neighbour), true);

	// we are on different faces
	switch (neighbour) {
	case 0: return make_pair<int, bool>(3, true);
	case 1: return make_pair<int, bool>(1, false);
	case 2: return make_pair<int, bool>(2, false);
		default:
	case 3:  return make_pair<int, bool>(0, true);
	}
}

void SpherifiedPlane::setChildrenNeighbours() {
	// Pretty complicated model plane neighbours connection formulas.
	for (int i = 0; i < 4; i++) {
		// inner
		m_children[i]->m_neighbours[nextIdx(i)] = m_children[nextIdx(i)].get();
		m_children[i]->m_neighbours[oppositeIdx(i)] = m_children[previousIdx(i)].get();
		// outer
		if (m_neighbours[i]->m_state.hasChildren()) {
			if (m_neighbours[i]->m_neighbours[oppositeIdx(i)] == this) {
				// we are on same cube face
				m_children[i]->m_neighbours[i] = m_neighbours[i]->m_children[previousIdx(i)].get();
				m_children[i]->m_neighbours[i]->m_neighbours[oppositeIdx(i)] = m_children[i].get();
			}
			else {
				const int adjIdx = i ? i % 3 + 1 : 0;
				m_children[i]->m_neighbours[i] = m_neighbours[i]->m_children[adjIdx].get();
				m_children[i]->m_neighbours[i]->m_neighbours[previousIdx(adjIdx)] = m_children[i].get();
			}
			m_children[i]->m_neighbours[i]->m_state.restore();
		}

		if (m_neighbours[previousIdx(i)]->m_state.hasChildren()) {
			if (m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] == this) {
				// we are on same cube face
				m_children[i]->m_neighbours[previousIdx(i)] = m_neighbours[previousIdx(i)]->m_children[nextIdx(i)].get();
				m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] = m_children[i].get();
			}
			else {
				const int adjIdx = i ? (i + 1) % 3 + 1 : 0;
				m_children[i]->m_neighbours[previousIdx(i)] = m_neighbours[previousIdx(i)]->m_children[adjIdx].get();
				m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[adjIdx] = m_children[i].get();
			}
			m_children[i]->m_neighbours[previousIdx(i)]->m_state.restore();
		}
	}
}

void SpherifiedPlane::resetChildrenNeighbours() {
	for (int i = 0; i < 4; i++) {
		// skip inner, skip children at all, take only outer neighbours
		if (m_neighbours[i]->m_state.hasChildren()) {
			if (m_neighbours[i]->m_neighbours[oppositeIdx(i)] == this) {
				// we are on same cube face
				m_children[i]->m_neighbours[i]->m_neighbours[oppositeIdx(i)] = nullptr;
			}
			else {
				const int adjIdx = i ? i % 3 + 1 : 0;
				m_children[i]->m_neighbours[i]->m_neighbours[previousIdx(adjIdx)] = nullptr;
			}
		}
		if (m_neighbours[previousIdx(i)]->m_state.hasChildren()) {
			if (m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] == this) {
				// we are on same cube face
				m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[nextIdx(i)] = nullptr;
			}
			else {
				const int adjIdx = i ? (i + 1) % 3 + 1 : 0;
				m_children[i]->m_neighbours[previousIdx(i)]->m_neighbours[adjIdx] = nullptr;
			}
		}
	}
}

bool SpherifiedPlane::tryDivide() {
	switch (m_state.state) {
	case State::Ready:
		// We can't divide if any of our neighbours missing, so we try to 
		// divede respective parent's neighbour to create our own one.
		for (int i = 0; i < 4; i++)
			if (m_neighbours[i] == nullptr && !m_pParent->m_neighbours[i]->tryDivide())
				return false;

		// If perent was undivided - restore it completely.
		if (m_pParent != nullptr && !m_pParent->tryDivide()) return false;

		m_children[0].reset(new SpherifiedPlane(m_pSphere, this, XMINT2 { 0, 0 }));
		m_children[1].reset(new SpherifiedPlane(m_pSphere, this, XMINT2 { 1, 0 }));
		m_children[2].reset(new SpherifiedPlane(m_pSphere, this, XMINT2 { 1, 1 }));
		m_children[3].reset(new SpherifiedPlane(m_pSphere, this, XMINT2 { 0, 1 }));

		setChildrenNeighbours();

		for (int i = 0; i < 4; ++i) {
			m_children[i]->m_pTerrainMap = make_unique<TerrainMap>(*m_pTerrainMap, i);
		}

		m_state.divide();
		return true;

	case State::Divided:
		return true;

	case State::Undivided:
		for (const auto& child : m_children) 
			child->m_state.restore();
		m_state.divide();
		return true;

	default:
		return false;
	}
}

bool SpherifiedPlane::tryUndivide() {
	switch (m_state.state) {
	case State::Ready:
	case State::Deleted:
		return true;

	case State::Divided: 
		for (const auto& child : m_children)
			if (child->m_state.hasChildren()) return false;
		m_state.undivide(); 
		// dive

	case State::Undivided:
		// smth wrong here
		SpherifiedPlane* pPlane;
		for (int i = 0; i < 4; i++) {
			// We can't undivide if any of our children' neighbours has children.
			if (m_children[i]->m_state.state == State::Making) continue;
			if (((pPlane = m_children[i]->m_neighbours[i]) != nullptr &&
				 (!pPlane->tryUndivide() || pPlane->m_state.hasChildren())) ||
				((pPlane = m_children[i]->m_neighbours[previousIdx(i)]) != nullptr &&
				 (!pPlane->tryUndivide() || pPlane->m_state.hasChildren())))
				continue;
			m_children[i]->m_state.erase();
		}
		for (const auto& child : m_children) {
			if (child->m_state.state != State::Deleted)
				return true;
		}
		resetChildrenNeighbours();
		for (auto& child : m_children) {
			child = nullptr;
		}
		m_state.clear();
		return true;

	default:
		return false;
	}
}

void SpherifiedPlane::prepareToLoad() {
	bLoad = false;
	bLoadChildren = false;

	if (m_state.state == State::Making) {
		if (!m_pTerrainMap->isComplete) return;
		
		m_pTerrainMap->alignEdgeNormals();
		m_state.generate();
	}
	bLoad = true;

	if (m_state.hasChildren()) {
		for (const auto& child : m_children) {
			if (!child->m_pTerrainMap->isComplete) {
				for (const auto& child : m_children) child->bLoad = false;
				return;
			}	
		}
		for (const auto& child : m_children) child->prepareToLoad();
		bLoadChildren = true;
	}
}

void SpherifiedPlane::loadPlane(vector<GPUDesc>& planes,
								DWORD& nextIndex) {
	if (bLoadChildren)
		for (const auto& child : m_children)
			child->loadPlane(planes, nextIndex);

	GPUDesc planeDesc;
	// Face index is in first byte.
	planeDesc.faceAndFlags = m_faceIdx;
	planeDesc.level = m_level;
	planeDesc.position = m_planeIndex;
	terrainEdgesToLoad.fill(true);
	for (int i = 0; i < 4; i++) {
		if (m_neighbours[i] != nullptr && m_neighbours[i]->bLoad) {
			// First 4 bits of second byte are 
			// set if corresponding neighbour is more divided.
			if (m_neighbours[i]->bLoadChildren)
				planeDesc.faceAndFlags |= 1 << (8 + i);
		}
		else terrainEdgesToLoad[i] = false;
	}

	planes.push_back(move(planeDesc));
	indexToLoad = nextIndex++;
}

void SpherifiedPlane::loadIndicesAndTerrain(std::vector<DWORD>& indices,
								  vector<DirectX::XMFLOAT4>& terrain) {
	if (bLoadChildren) {
		for (const auto& child : m_children)
			child->loadIndicesAndTerrain(indices, terrain);
	}
	else { 
		if (indexToLoad == (DWORD)-1) return;
		indices.push_back(indexToLoad);
		for (int i = 0; i < 4; i++) {
			if (m_neighbours[i] == nullptr || !m_neighbours[i]->bLoad) {
				if (m_pParent == nullptr) 
					indices.push_back(m_neighbours[i]->indexToLoad);
				else 
					indices.push_back(m_pParent->m_neighbours[i]->indexToLoad);
			}	
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


// Interesting automata, useful and checks some logic errors at once.

bool SpherifiedPlane::StateTransitions::hasChildren() {
	return m_state == State::Divided || m_state == State::Undivided;
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::generate() {
	switch (m_state) {
	case State::Making:
		OutputDebugStringA("Generated\n");
		return m_state = State::Ready;
	default:
		throw runtime_error("SpherifiedPlane generate() state transition failed!");
	}
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::divide() {
	switch (m_state) {
	case State::Ready:
	case State::Undivided:
		OutputDebugStringA("Divided\n");
		return m_state = State::Divided;
	default:
		throw runtime_error("SpherifiedPlane divide() state transition failed!");
	}
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::undivide() {
	switch (m_state) {
	case State::Divided:
		OutputDebugStringA("Undivided\n");
		return m_state = State::Undivided;
	case State::Ready:
	case State::Undivided:
		return m_state;
	default:
		throw runtime_error("SpherifiedPlane undivide() state transition failed!");
	}
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::clear() {
	switch (m_state) {
	case State::Undivided:
		OutputDebugStringA("Cleared\n");
		return m_state = State::Ready;
	default:
		throw runtime_error("SpherifiedPlane clear() state transition failed!");
	}
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::erase() {
	switch (m_state) {
	case State::Ready:
		OutputDebugStringA("Deleted\n");
		return m_state = State::Deleted;
	case State::Deleted:
		return m_state;
	default:
		throw runtime_error("SpherifiedPlane erase() state transition failed!");
	}
}

const SpherifiedPlane::State& SpherifiedPlane::StateTransitions::restore() {
	switch (m_state) {
	case State::Making:
	case State::Ready:
		return m_state;
	case State::Deleted:
		OutputDebugStringA("Restored\n");
		return m_state = State::Ready;
	default:
		throw runtime_error("SpherifiedPlane restore() state transition failed!");
	}
}
