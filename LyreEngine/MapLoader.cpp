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
		XMVECTOR originalPosition, normal, surfaceDerivative;
		XMFLOAT4 perlin;
		float height;

		while (true) {
			unique_lock<mutex> queueLocker(m_membersLock);
			m_cvOnPush.wait(queueLocker, [this]() {
				return m_bStop || !m_loadingQueue.empty();
			});
			if (m_bStop) return;

			TerrainMap* pMap = m_loadingQueue.front();
			m_loadingQueue.pop();

			queueLocker.unlock();

			unique_lock<mutex> mapLocker(pMap->m_membersLock);
			if (pMap->m_desc.currentOctaveDepth <= 0 ||
				pMap->m_state != TerrainMap::State::NoMap)
				continue;

			pMap->m_state = TerrainMap::State::MapLoading;

			for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
				for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
					gridPosition = pMap->sampleSphere(
						j / static_cast<float>(HEIGHTMAP_RESOLUTION - 1),
						i / static_cast<float>(HEIGHTMAP_RESOLUTION - 1)
					);

					originalPosition = XMLoadFloat3(&gridPosition);
					normal = XMVector3Normalize(originalPosition);

					XMStoreFloat3(&gridPosition, originalPosition * pMap->m_desc.octave);
					perlin = m_terrainGenerator.perlinNoise(gridPosition);

					height = perlin.w * pMap->m_desc.amplitude;
					surfaceDerivative = 2.f * XMLoadFloat4(&perlin) * pMap->m_desc.amplitude;

					XMStoreFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION],
								  XMLoadFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION]) +
								  XMVectorSetW(XMVector3Normalize(normal - (surfaceDerivative - XMVector3Dot(surfaceDerivative, normal) * normal)),
											   height));
				}

				//just give it a break
				mapLocker.unlock();
				this_thread::sleep_for(chrono::milliseconds(1));
				mapLocker.lock();
			}

			if (--(pMap->m_desc.currentOctaveDepth) > 0) {
				pMap->nextOctave();
				pMap->m_state = TerrainMap::State::NoMap;
				mapLocker.unlock();
				pushToQueue(pMap);
			}
			else {
				pMap->m_state = TerrainMap::State::MapReady;
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
