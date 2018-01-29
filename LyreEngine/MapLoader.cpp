#include "stdafx.h"

#include "MapLoader.h"

#include "TerrainMap.h"
#include "PerlinGrid.h"

using namespace std;
using namespace DirectX;

MapLoader::MapLoader(unsigned seed)
	: m_bStop(false), m_terrainGenerator(seed) {}

MapLoader::~MapLoader() {
	{
		unique_lock<mutex> locker(m_membersLock);
		m_bStop = true;
	}
	m_cvOnPush.notify_all();
	m_tLoader.join();
}

void MapLoader::start() {
	m_tLoader = thread([this]() {
		XMFLOAT3 gridPosition;
		XMVECTOR originalPosition, normal;
		XMFLOAT4 perlin;

		while (true) {
			unique_lock<mutex> queueLocker(m_membersLock);
			m_cvOnPush.wait(queueLocker, [this]() {
				return m_bStop || !m_loadingQueue.empty();
			});
			if (m_bStop) return;

			TerrainMap* pMap = m_loadingQueue.front();
			m_loadingQueue.pop();

			queueLocker.unlock();

			unique_lock<shared_mutex> mapLocker(pMap->m_membersLock); // writer
			if (pMap->m_desc.currentOctaveDepth <= 0) continue;

			for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
				for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
					gridPosition = pMap->sampleSphere(
						j / static_cast<float>(HEIGHTMAP_RESOLUTION - 1),
						i / static_cast<float>(HEIGHTMAP_RESOLUTION - 1)
					);

					originalPosition = XMLoadFloat3(&gridPosition);
					normal = XMVector3Normalize(originalPosition);

					gridPosition.x += pMap->m_desc.shift;
					gridPosition.y += pMap->m_desc.shift;
					gridPosition.z += pMap->m_desc.shift;
					XMStoreFloat3(&gridPosition, originalPosition * pMap->m_desc.octave);
					perlin = m_terrainGenerator.perlinNoise(gridPosition);

					XMStoreFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION],
								  XMLoadFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION]) +
								  XMLoadFloat4(&perlin) * pMap->m_desc.amplitude);
				}
			}

			pMap->nextOctave();

			if (pMap->m_desc.currentOctaveDepth > 0) {
				mapLocker.unlock(); // avoid locking two at the same time
				queueLocker.lock();
				m_loadingQueue.push(pMap);
			}
		}
	});
}

void MapLoader::pushToQueue(TerrainMap* pMap) {
	if (pMap == nullptr) return;
	unique_lock<mutex> locker(m_membersLock);
	m_loadingQueue.push(pMap);
	m_cvOnPush.notify_all();
}
