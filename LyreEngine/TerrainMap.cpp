#include "stdafx.h"

#include "TerrainMap.h"

using namespace DirectX;
using namespace std;

TerrainMap::TerrainMap(const TerrainMap & base, unsigned regionIdx)
	: m_state(State::NoMap), m_desc(base.m_desc) {

	if (base.m_desc.currentOctaveDepth > 0)
		throw std::runtime_error("Base terrain map is not ready yet.");
	m_heightMap.fill({ 0.f, 0.f, 0.f, 0.f });
	m_desc.amplitude /= 2.f;
	m_desc.octave *= 2.f;
	m_desc.shift *= sqrt(3); //sqrt(3) has no meaning, just random
	m_desc.currentOctaveDepth = 1; //need to generate one more octave
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
}

TerrainMap::TerrainMap(const Description & desc) {}

XMFLOAT3 TerrainMap::sampleSphere(float u, float v) const {
	XMFLOAT3 result;
	XMVECTOR q0 = XMLoadFloat3(&m_desc.quad[0]);
	XMStoreFloat3(&result, XMVector3ClampLengthV(
		(q0*u +
		 XMLoadFloat3(&m_desc.quad[1])*(1 - u))*v +
		 (XMLoadFloat3(&m_desc.quad[2])*u +
		  XMLoadFloat3(&m_desc.quad[3])*(1 - u))*(1 - v),
		q0, q0));
	return result;
}

bool TerrainMap::loadTerrain(std::vector<XMFLOAT4>& terrain) {
	unique_lock<mutex> locker(m_membersLock, defer_lock);
	if (!locker.try_lock() || m_state == State::MapLoading)
		return false;

	if (m_state == State::NoMap) {
		//TODO: load map
		return false;
	}

	if (m_state == State::DeleteMark)
		m_state = State::MapReady;

	terrain.insert(terrain.end(), m_heightMap.begin(), m_heightMap.end());

	return true;
}
