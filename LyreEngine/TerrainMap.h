#pragma once

// All together terrain maps form six "region quadtrees". The only difference from 
// region quadtrees described in Wikipedia is that the tree is actually not a trie: 
// leafs contain full terrain map of their surface subregion. Still they represent 
// a variable resolution data field. Children copy map region from parent adding new
// higher-resolution details to it.

class SpherifiedPlane;

class TerrainMap final {
	friend class MapLoader;

public:
	struct Description final {
		float amplitude;
		float octave;
		float shift;
	};

	static const int GRID_DEPTH = 6;
	static const int RESOLUTION = (1 << GRID_DEPTH) + 1;

private:
	// From which octave to save base for children
	static const int CHILDREN_DEPTH = GRID_DEPTH;

	union HeightmapTexel {
		struct {
			DirectX::XMFLOAT3 normal = { 0.f, 0.f, 0.f };
			float height = 0;
		} data;
		DirectX::XMFLOAT4 f4;

		HeightmapTexel() : data() {}
		operator DirectX::XMFLOAT4() const { return f4; }
		HeightmapTexel& operator=(const HeightmapTexel& another) {
			f4 = another.f4; return *this; }
	};

	typedef std::vector<HeightmapTexel> HeightMapT;
	HeightMapT m_heightMap;
	std::array<HeightMapT, 4> m_edgesBackup;
	// backup of parent's backup ;P (TODO)
	std::array<HeightmapTexel, 4> m_cornersBackupBackup;

	MapLoader* m_pMapLoader;
	//configuration for MapLoader
	Description m_desc;
	int m_octavesToGenerate;
	std::atomic<bool> m_bComplete;
	SpherifiedPlane* m_pPlane;
	//call when ready to generate new octave
	void nextOctave();

	//get vector from sphere center to surface 
	DirectX::XMFLOAT3 sampleSphere(__int32 xIndex, __int32 yIndex) const;

	HeightMapT m_childrenBase;
	Description m_baseDesc;
	//K-times magnification algorithm (K=2).
	void produceScaledRegions() const;
	//Child maps will be moved from here on construction.
	mutable std::array<HeightMapT, 4> m_baseScaledRegions;
	//Child backup edges will be moved from here on construction.
	mutable std::array<std::array<HeightMapT, 4>, 4> m_backupEdgesScaled;

public:
	TerrainMap(const TerrainMap& base, unsigned regionIdx);
	TerrainMap(SpherifiedPlane* plane, const Description& desc, MapLoader* pMapLoader);
	void loadTerrain(std::vector<DirectX::XMFLOAT4>& terrain, const std::array<bool, 4>& trueEdges = { true, true, true, true }) const;
	void alignEdgeNormals();
	const std::atomic<bool>& isComplete = m_bComplete;
};
