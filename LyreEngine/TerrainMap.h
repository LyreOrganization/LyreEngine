#pragma once

// 64 + 1 (right edge texel for easy magnification)
#define HEIGHTMAP_RESOLUTION 65 

class TerrainMap final {
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
	MapLoader* m_pMapLoader;

	std::vector<DirectX::XMFLOAT4> m_heightMap;

	mutable std::mutex m_membersLock;

	Description m_desc;
	State m_state;

	//these are not thread-safe!
	DirectX::XMFLOAT3 sampleSphere(float u, float v) const;
	void nextOctave();

public:
	TerrainMap(const TerrainMap& base, unsigned regionIdx);
	TerrainMap(const Description& desc, MapLoader* pMapLoader);
	void loadTerrain(std::vector<DirectX::XMFLOAT4>& terrain);
	State getState() const;
};