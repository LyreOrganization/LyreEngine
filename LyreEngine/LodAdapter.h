#pragma once

#include "SpherifiedCube.h"

class LodAdapter final {
	std::thread m_tAdapter;
	std::mutex m_membersLock;
	bool m_bStop;
	std::multimap<float, SpherifiedPlane::GPUDesc> m_lodData;
	bool m_bNewData;
	std::condition_variable m_cvOnRead;
	
	SphereTopology m_writingTopology;
	std::mutex m_topologyLock;

	SpherifiedCube* m_pSphere;

public:
	LodAdapter(SpherifiedCube* pSphere);
	~LodAdapter();
	void start();
	void readCurrentLods(ID3D11UnorderedAccessView* pUav, 
						 const SphereTopology& topology);
};


