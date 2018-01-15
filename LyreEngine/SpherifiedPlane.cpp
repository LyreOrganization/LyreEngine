#include "stdafx.h"

#include "SpherifiedPlane.h"

#include "SpherifiedCube.h"
#include "PerlinGrid.h"

using namespace std;
using namespace DirectX;

namespace {

	constexpr DWORD nextIdx(DWORD index) { return ((index + 1) % 4); }		// next clockwise ement index (of 4)
	constexpr DWORD previousIdx(DWORD index) { return ((index + 3) % 4); }	// previous clockwise ement index (of 4)
	constexpr DWORD oppositeIdx(DWORD index) { return ((index + 2) % 4); }	// opposite ement index (of 4)

	PerlinGrid g_noise(11); // argument - seed for random

}

XMFLOAT3 SpherifiedPlane::uv2pos(float u, float v) const {
	XMFLOAT3 result;
	XMStoreFloat3(&result, (XMLoadFloat3(&m_pSphere->vertices()[m_points[0]].position)*u +
							XMLoadFloat3(&m_pSphere->vertices()[m_points[1]].position)*(1 - u))*v +
							(XMLoadFloat3(&m_pSphere->vertices()[m_points[3]].position)*u +
							 XMLoadFloat3(&m_pSphere->vertices()[m_points[2]].position)*(1 - u))*(1 - v));
	return result;
}

void SpherifiedPlane::loadTopology(std::vector<XMFLOAT4>& terrain, std::vector<DWORD>& indices) {
	if (m_divided) {
		for (const auto& child : m_children) {
			child->loadTopology(terrain, indices);
		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			indices.push_back(m_points[i]);
		}
		for (int i = 0; i < 4; i++) {
			indices.push_back(m_neighbours[i]->m_middle);
		}
		indices.push_back(m_middle);

		terrain.insert(terrain.end(), m_terrainMap.value().begin(), m_terrainMap.value().end());
	}
}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* parent)
	: m_pSphere(sphere), m_points(points), m_pParent(parent), m_terrainMap() {
	m_middle = m_pSphere->createMidpoint(m_points);
}

void SpherifiedPlane::divide(int depth) {
	if (depth < 1) return;

	if (!m_divided) {
		//divide father's neighbour if it is not divided
		for (int i = 0; i < 4; i++)
			if (m_neighbours[i] == nullptr) m_pParent->m_neighbours[i]->divide();

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

		m_divided = true;
	}

	for (int i = 0; i < 4; ++i)
		m_children[i]->divide(depth - 1);
}

void SpherifiedPlane::generateTerrain() {
	if (m_divided) {
		for (const auto& child : m_children)
			child->generateTerrain();
	}
	else {
		auto& r_vertices = m_pSphere->vertices();
		m_terrainMap.emplace();

		for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
			for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
				XMFLOAT3 originalPosition = uv2pos(
					i / static_cast<float>(HEIGHTMAP_RESOLUTION - 1),
					j / static_cast<float>(HEIGHTMAP_RESOLUTION - 1)
				);

				XMVECTOR original = XMLoadFloat3(&originalPosition);
				XMVECTOR normal = XMVector3Normalize(original);
				XMVECTOR surfaceDerivative = XMVectorZero();
				XMFLOAT3 scaledPos;
				float height = 0;
				for (int i = 0; i < 7/*iMAX*/; i++) { // fractal, iMAX octaves
					XMStoreFloat3(&scaledPos, original * (float)(1 << (i + 0/*octave*/)));
					XMFLOAT4 perlin = g_noise.perlinNoise(scaledPos);
					XMVECTOR vecNormal = XMLoadFloat4(&perlin);

					//normal
					height += perlin.w / (float)(1 << (i + 2/*amplitude*/));
					surfaceDerivative += vecNormal / (float)(1 << (i + 2/*amplitude*/));

					////erosion
					//height += abs(perlin.w) / (float)(1 << (i + 2/*amplitude*/));
					//float smooth = fabs(perlin.w) > 0.1f ? 1.f : (perlin.w / 0.1f);
					//smooth = smooth*smooth;
					//surfaceDerivative += (perlin.w > 0 ? vecNormal : -vecNormal) * smooth / (float)(1 << (i + 2/*amplitude*/));

					////ridges
					//height += -abs(perlin.w) / (float)(1 << (i + 2/*amplitude*/));
					//float smooth = fabs(perlin.w) > 0.1f ? 1.f : (perlin.w / 0.1f);
					//smooth = smooth*smooth;
					//surfaceDerivative += (perlin.w > 0 ? -vecNormal : vecNormal) * smooth / (float)(1 << (i + 2/*amplitude*/));

					////plates
					//float plate = floor(perlin.w * 4.f) / 4.f;
					//float rest = perlin.w - plate;
					//float smoothA = 1.f - rest / 0.25f;
					//float smoothB = 1.f - (0.25f - rest) / 0.25f;
					//if (smoothA > smoothB) {
					//	//smoothA *= smoothA*smoothA;
					//	height += (plate + rest*smoothA*2.f) / (float)(1 << (i + 2/*amplitude*/));
					//	surfaceDerivative += vecNormal * smoothA / (float)(1 << (i + 2/*amplitude*/));
					//}
					//else {
					//	//smoothB *= smoothB*smoothB;
					//	height += (plate + 0.25f - rest*smoothB*2.f) / (float)(1 << (i + 2/*amplitude*/));
					//	surfaceDerivative -= vecNormal * smoothB / (float)(1 << (i + 2/*amplitude*/));
					//}

				}
				XMStoreFloat4(&m_terrainMap.value()[j + i * HEIGHTMAP_RESOLUTION], XMVectorSetW(
					XMVector3Normalize(normal - (surfaceDerivative - XMVector3Dot(surfaceDerivative, normal) * normal)),
					height));
			}
		}
	}
}

