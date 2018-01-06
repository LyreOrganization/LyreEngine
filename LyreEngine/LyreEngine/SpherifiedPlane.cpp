#include "stdafx.h"

#include "SpherifiedPlane.h"

#include "SpherifiedCube.h"

SpherifiedPlane::SpherifiedPlane(SpherifiedCube* sphere, DWORD4 points, SpherifiedPlane* parent)
	: m_pSphere(sphere), m_points(points), m_pParent(parent)
{}


void SpherifiedPlane::divide(int depth)
{
	if (depth < 1) return;

	if (!m_divided)
	{
		//divide father's neighbour if it is not divided
		if (m_neighbours[0] == nullptr) m_pParent->m_neighbours[0]->divide();
		if (m_neighbours[1] == nullptr) m_pParent->m_neighbours[1]->divide();
		if (m_neighbours[2] == nullptr) m_pParent->m_neighbours[2]->divide();
		if (m_neighbours[3] == nullptr) m_pParent->m_neighbours[3]->divide();
		
		/*
		//creating halfs if they don't exist
		///halfs are indexed correctrly for inner (children[3]) opposite-directional triangle
		if (m_halfs[0] == nullptr)
		{
			m_halfs[0] = m_pSphere->createHalf(m_points[1], m_points[2]);
			///setting neighbour's halfs even if they aren't divided
			///neighbour with index "0" must be opposite-directional
			m_neighbours[0]->setHalf(0, m_halfs[0]);
		}
		if (m_halfs[1] == nullptr)
		{
			m_halfs[1] = m_pSphere->createHalf(m_points[2], m_points[0]);
			if (m_neighbours[1]->m_direction == m_direction)
				m_neighbours[1]->setHalf(2, m_halfs[1]);
			else m_neighbours[1]->setHalf(1, m_halfs[1]);
		}
		if (m_halfs[2] == nullptr)
		{
			m_halfs[2] = m_pSphere->createHalf(m_points[0], m_points[1]);
			if (m_neighbours[2]->m_direction == m_direction)
				m_neighbours[2]->setHalf(1, m_halfs[2]);
			else m_neighbours[2]->setHalf(2, m_halfs[2]);
		}

		//creating children
		m_children[0] = std::make_unique<SpherifiedPlane>(m_pSphere, m_points[0], m_halfs[2], m_halfs[1], m_direction, this);
		m_children[1] = std::make_unique<SpherifiedPlane>(m_pSphere, m_halfs[2], m_points[1], m_halfs[0], m_direction, this);
		m_children[2] = std::make_unique<SpherifiedPlane>(m_pSphere, m_halfs[1], m_halfs[0], m_points[2], m_direction, this);
		///opposite-directional triangle
		m_children[3] = std::make_unique<SpherifiedPlane>(m_pSphere, m_halfs[0], m_halfs[1], m_halfs[2], m_direction ^ true, this);

		//setting children's neighbours
		///other children for inner child
		m_children[3]->m_neighbours[0] = m_children[0].get();
		m_children[3]->m_neighbours[1] = m_children[1].get();
		m_children[3]->m_neighbours[2] = m_children[2].get();
		///inner child for other children
		m_children[0]->m_neighbours[0] = m_children[3].get();
		m_children[1]->m_neighbours[1] = m_children[3].get();
		m_children[2]->m_neighbours[2] = m_children[3].get();
		///outer neighbours
		if (m_neighbours[0]->m_divided)
		{
			m_children[1]->m_neighbours[0] = m_neighbours[0]->m_children[2].get();
			m_children[2]->m_neighbours[0] = m_neighbours[0]->m_children[1].get();
			m_neighbours[0]->m_children[1]->m_neighbours[0] = m_children[2].get();
			m_neighbours[0]->m_children[2]->m_neighbours[0] = m_children[1].get();
		}
		if (m_neighbours[1]->m_divided)
		{
			if (m_neighbours[1]->m_direction != m_direction)
			{
				m_children[0]->m_neighbours[1] = m_neighbours[1]->m_children[2].get();
				m_children[2]->m_neighbours[1] = m_neighbours[1]->m_children[0].get();
				m_neighbours[1]->m_children[0]->m_neighbours[1] = m_children[2].get();
				m_neighbours[1]->m_children[2]->m_neighbours[1] = m_children[0].get();
			}
			else
			{
				m_children[0]->m_neighbours[1] = m_neighbours[1]->m_children[0].get();
				m_children[2]->m_neighbours[1] = m_neighbours[1]->m_children[1].get();
				m_neighbours[1]->m_children[0]->m_neighbours[2] = m_children[0].get();
				m_neighbours[1]->m_children[1]->m_neighbours[2] = m_children[2].get();
			}
		}
		if (m_neighbours[2]->m_divided)
		{
			if (m_neighbours[2]->m_direction != m_direction)
			{
				m_children[0]->m_neighbours[2] = m_neighbours[2]->m_children[1].get();
				m_children[1]->m_neighbours[2] = m_neighbours[2]->m_children[0].get();
				m_neighbours[2]->m_children[0]->m_neighbours[2] = m_children[1].get();
				m_neighbours[2]->m_children[1]->m_neighbours[2] = m_children[0].get();
			}
			else
			{
				m_children[0]->m_neighbours[2] = m_neighbours[2]->m_children[0].get();
				m_children[1]->m_neighbours[2] = m_neighbours[2]->m_children[2].get();
				m_neighbours[2]->m_children[0]->m_neighbours[1] = m_children[0].get();
				m_neighbours[2]->m_children[2]->m_neighbours[1] = m_children[1].get();
			}
		}

		m_divided = true;
		*/
	}

	for (int i = 0; i < 4; ++i)
		m_children[i]->divide(depth-1);
}

/*
void SpherifiedPlane::setHalf(int halfInd, SpherifiedPoint * half)
{
	if (m_divided) throw std::exception("No permission to change divided TrianPlane!");
	m_halfs[halfInd] = half;
	if (m_validHalf != INVALID_HALF) divide();
	else m_validHalf = halfInd;
}

DWORD SpherifiedPlane::getTrianPointIndex(int pointInd)
{
	return m_points[pointInd]->m_index;
}

DWORD SpherifiedPlane::getHalfIndex(int halfInd)
{
	return m_halfs[halfInd]->m_index;
}

int SpherifiedPlane::getTrianglesAmount()
{
	if (!m_divided)
	{
		if (m_validHalf != INVALID_HALF) return 2;
		else return 1;
	}
	return m_children[0]->getTrianglesAmount() +
		m_children[1]->getTrianglesAmount() +
		m_children[2]->getTrianglesAmount() +
		m_children[3]->getTrianglesAmount();
}

void SpherifiedPlane::clearChildren()
{
	for (int i = 0; i < 4; ++i)
		m_children[i].reset();
}
*/