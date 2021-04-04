#include "stdafx.h"

#include "MapLoader.h"

#include "TerrainMap.h"
#include "PerlinGrid.h"
#include "SpherifiedCube.h"

using namespace std;
using namespace DirectX;

MapLoader::MapLoader(SpherifiedCube* sphere, unsigned seed)
	: m_pSphere(sphere), m_terrainGenerator(seed), m_bStop(false) {}

MapLoader::~MapLoader() {
	{
		unique_lock<mutex> locker(m_membersLock);
		m_bStop = true;
	}
	m_cvOnPush.notify_all();
	if (m_tLoader.joinable()) m_tLoader.join();
}

void MapLoader::start() {
	static const int RES = TerrainMap::RESOLUTION;

	m_tLoader = thread([this]() {
		XMFLOAT3 gridPosition;
		float perlin;

		while (true) {
			unique_lock<mutex> queueLocker(m_membersLock);
			m_cvOnPush.wait(queueLocker, [this]() {
				return m_bStop || !m_loadingQueue.empty();
			});
			if (m_bStop) return;

			TerrainMap* pMap = m_loadingQueue.front();
			m_loadingQueue.pop();

			queueLocker.unlock();

			typedef __declspec(align(16)) struct {
				DirectX::XMFLOAT3 pos;
			} TexelPosition;
			array<TexelPosition, RES * RES> positionsCache;

			for (int i = 0; i < RES; i++) {
				for (int j = 0; j < RES; j++) {
					positionsCache[i * RES + j].pos = pMap->sampleSphere(j, i);
				}
			}

			auto& hMap = pMap->m_heightMap;
			auto& desc = pMap->m_desc;

			while (pMap->m_octavesToGenerate) {
				for (int i = 0; i < RES * RES; i++) {
					XMStoreFloat3(&gridPosition, XMVectorReplicate(desc.shift) +
								  XMLoadFloat3(&positionsCache[i].pos) * pMap->m_desc.octave);

					perlin = m_terrainGenerator.perlinNoise(gridPosition);

					hMap[i].data.height += perlin * pMap->m_desc.amplitude;
				}
				pMap->nextOctave();
			}

			float radius = m_pSphere->getRadius();

			for (int i = 0; i < RES * RES; i++) {
				XMStoreFloat3(&positionsCache[i].pos,
							  XMLoadFloat3(&positionsCache[i].pos) *
							  (radius + hMap[i].data.height));
			}

			XMVECTOR pos, normal, crossProduct;
			array<XMVECTOR, 4> edgeVecs;
			array<bool, 4> edgePresent;
			for (int i = 0; i < RES; i++) {
				for (int j = 0; j < RES; j++) {
					pos = XMLoadFloat3(&positionsCache[i * RES + j].pos);
					normal = XMVectorZero();

					if (edgePresent[0] = (j > 0)) edgeVecs[0] = 
						XMLoadFloat3(&positionsCache[i * RES + j - 1].pos) - pos;
					if (edgePresent[1] = (i > 0)) edgeVecs[1] = 
						XMLoadFloat3(&positionsCache[(i - 1) * RES + j].pos) - pos;
					if (edgePresent[2] = (j < RES - 1)) edgeVecs[2] = 
						XMLoadFloat3(&positionsCache[i * RES + j + 1].pos) - pos;
					if (edgePresent[3] = (i > RES - 1)) edgeVecs[3] = 
						XMLoadFloat3(&positionsCache[(i + 1) * RES + j].pos) - pos;

					for (int i = 0; i < 4; i++) {
						if (!edgePresent[(i+1)%4]) {
							i++;
							continue;
						}
						if (!edgePresent[i]) continue;
						crossProduct = XMVector3Cross(edgeVecs[i], edgeVecs[(i + 1) % 4]);
						normal += XMVector3Normalize(
							XMVectorGetX(XMVector3Dot(crossProduct, pos)) > 0.f ?
							crossProduct : -crossProduct);
					}

					XMStoreFloat3(&hMap[i * RES + j].data.normal, XMVector3Normalize(normal));
				}
			}

			pMap->m_bComplete.store(true);
		}
	});
}

void MapLoader::pushToQueue(TerrainMap* pMap) {
	if (pMap == nullptr) return;
	unique_lock<mutex> locker(m_membersLock);
	m_loadingQueue.push(pMap);
	m_cvOnPush.notify_all();
}
