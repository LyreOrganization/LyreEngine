#include "stdafx.h"

#include "SpherifiedPlane.h"

#include "SpherifiedCube.h"

using namespace std;
using namespace DirectX;

namespace {

	static constexpr DWORD nextIdx(DWORD index) { return ((index + 1) % 4); }		// next clockwise ement index (of 4)
	static constexpr DWORD previousIdx(DWORD index) { return ((index + 3) % 4); }	// previous clockwise ement index (of 4)
	static constexpr DWORD oppositeIdx(DWORD index) { return ((index + 2) % 4); }	// opposite ement index (of 4)

}

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* parent)
	: m_pSphere(sphere), m_points(points), m_pParent(parent) {}

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

		//create middle point
		DWORD middle = m_pSphere->createHalf(m_halfs[0].value(), m_halfs[2].value());

		//creating children
		m_children[0] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_points[0],
			m_halfs[0].value(),
			middle,
			m_halfs[3].value()
		}, this);
		m_children[1] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_halfs[0].value(),
			m_points[1],
			m_halfs[1].value(),
			middle
		}, this);
		m_children[2] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			middle,
			m_halfs[1].value(),
			m_points[2],
			m_halfs[2].value()
		}, this);
		m_children[3] = make_unique<SpherifiedPlane>(m_pSphere, DWORD4 {
			m_halfs[3].value(),
			middle,
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

vector<DWORD> SpherifiedPlane::getIndicesBuffer() {
	vector<DWORD> indices;
	if (m_divided) {
		for (const auto& child : m_children) {
			vector<DWORD> childIndices = child->getIndicesBuffer();
			indices.insert(indices.end(), childIndices.begin(), childIndices.end());
		}
	}
	else {
		for (int i = 0; i < 4; i++) {
			if (m_halfs[i]) {
				if (m_halfs[nextIdx(i)])
					return {
					m_halfs[i].value(), m_points[nextIdx(i)], m_halfs[nextIdx(i)].value(),
					m_halfs[nextIdx(i)].value(), m_points[oppositeIdx(i)],  m_points[previousIdx(i)],
					m_points[previousIdx(i)], m_points[i], m_halfs[i].value(),
					m_halfs[i].value(), m_halfs[nextIdx(i)].value(), m_points[previousIdx(i)]
				};
				if (m_halfs[oppositeIdx(i)])
					return {
					m_points[previousIdx(i)],  m_points[i], m_halfs[i].value(),
					m_points[previousIdx(i)], m_halfs[i].value(), m_halfs[oppositeIdx(i)].value(),
					m_halfs[oppositeIdx(i)].value(), m_halfs[i].value(), m_points[nextIdx(i)],
					m_halfs[oppositeIdx(i)].value(), m_points[nextIdx(i)], m_points[oppositeIdx(i)],
				};
				return {
					m_points[previousIdx(i)],  m_points[i], m_halfs[i].value(),
					m_halfs[i].value(), m_points[nextIdx(i)], m_points[oppositeIdx(i)],
					m_points[oppositeIdx(i)], m_points[previousIdx(i)], m_halfs[i].value()
				};
			}
		}
		return {
			m_points[0],
			m_points[1],
			m_points[2],
			m_points[0],
			m_points[2],
			m_points[3],
		};
	}
	return indices;
}
