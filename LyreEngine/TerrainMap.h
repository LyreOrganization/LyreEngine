#pragma once

#define HEIGHTMAP_RESOLUTION 64

class TerrainMap {
	friend class MapLoader;

public:
	enum class State : int {
		NoMap = 0,
		MapLoading,
		MapReady,
		DeleteMark
	};

	struct Description final {
		float amplitude;
		float octave;
		float shift;
		int currentOctaveDepth;
		DirectX::XMFLOAT3 quad[4];
	};

private:
	std::array<DirectX::XMFLOAT4, HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION> m_heightMap;

	std::mutex m_membersLock;

	Description m_desc;
	State m_state;

public:
	TerrainMap(const TerrainMap& base, unsigned regionIdx);
	TerrainMap(const Description& desc);
	DirectX::XMFLOAT3 sampleSphere(float u, float v) const;
	bool loadTerrain(std::vector<DirectX::XMFLOAT4>& terrain);
};