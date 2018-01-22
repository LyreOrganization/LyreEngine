#pragma once

#include "PerlinGrid.h"

class TerrainMap;

class MapLoader final {
	std::queue<TerrainMap*> m_loadingQueue;
	std::thread m_tLoader;
	std::mutex m_membersLock;
	std::condition_variable m_cvOnPush;
	bool m_bStop;
	
	PerlinGrid m_terrainGenerator;

public:
	MapLoader(unsigned seed);
	~MapLoader();
	void start();
	void pushToQueue(TerrainMap* map);
};
