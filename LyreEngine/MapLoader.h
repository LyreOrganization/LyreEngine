#pragma once

#include "PerlinGrid.h"

class TerrainMap;
class SpherifiedCube;

class MapLoader final {
	std::queue<TerrainMap*> m_loadingQueue;
	std::thread m_tLoader;
	std::mutex m_membersLock;
	std::condition_variable m_cvOnPush;
	bool m_bStop;
	
	PerlinGrid m_terrainGenerator;

	SpherifiedCube* m_pSphere;

public:
	MapLoader(SpherifiedCube* sphere, unsigned seed);
	~MapLoader();
	void start();
	void pushToQueue(TerrainMap* map);
};
