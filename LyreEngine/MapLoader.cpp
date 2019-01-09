#include "stdafx.h"

#include "MapLoader.h"

#include "TerrainMap.h"
#include "PerlinGrid.h"
#include "SpherifiedPlane.h"

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
	if (m_tLoader.joinable()) m_tLoader.join();
}

void MapLoader::start() {
	m_tLoader = thread([this]() {
		XMFLOAT3 gridPosition;
		XMVECTOR originalPosition;
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

			for (auto& edge : pMap->m_edgesBackup) edge = vector<XMFLOAT4>(HEIGHTMAP_RESOLUTION);

			for (int i = 0; i < HEIGHTMAP_RESOLUTION; i++) {
				for (int j = 0; j < HEIGHTMAP_RESOLUTION; j++) {
					gridPosition = pMap->sampleSphere(j, i);

					originalPosition = XMLoadFloat3(&gridPosition);

					XMStoreFloat3(&gridPosition, originalPosition * pMap->m_desc.octave);
					gridPosition.x += pMap->m_desc.shift;
					gridPosition.y += pMap->m_desc.shift;
					gridPosition.z += pMap->m_desc.shift;

					perlin = m_terrainGenerator.perlinNoise(gridPosition);
					perlin.w *= pMap->m_desc.amplitude;
					// Calculate normal to surface
					XMVECTOR perlinVec = XMLoadFloat4(&perlin);
					XMStoreFloat4(&perlin, perlinVec + perlinVec * originalPosition * (pMap->m_desc.amplitude - 1.f));

					// Backup edges
					if (i == 0) {
						pMap->m_edgesBackup[0][j] = pMap->m_heightMap[j];
					}
					else if (i == HEIGHTMAP_RESOLUTION - 1) {
						pMap->m_edgesBackup[2][j] = pMap->m_heightMap[j + (HEIGHTMAP_RESOLUTION - 1) * HEIGHTMAP_RESOLUTION];
					}
					if (j == 0) {
						pMap->m_edgesBackup[3][i] = pMap->m_heightMap[i * HEIGHTMAP_RESOLUTION];
					}
					else if (j == HEIGHTMAP_RESOLUTION - 1) {
						pMap->m_edgesBackup[1][i] = pMap->m_heightMap[(i + 1) * HEIGHTMAP_RESOLUTION - 1];
					}

					XMStoreFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION],
								  XMLoadFloat4(&pMap->m_heightMap[j + i * HEIGHTMAP_RESOLUTION]) +
								  XMLoadFloat4(&perlin));
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
