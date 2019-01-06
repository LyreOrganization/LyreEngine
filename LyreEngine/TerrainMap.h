#pragma once

#define HEIGHTMAP_RESOLUTION 65 // num planes + 1

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
		int currentOctaveDepth;
	};

private:
	MapLoader* m_pMapLoader;

	typedef std::vector<DirectX::XMFLOAT4> HeightMapType;

	HeightMapType m_heightMap;
	std::array<HeightMapType, 4> m_edgesBackup;

	//configuration for MapLoader
	Description m_desc;
	SpherifiedPlane* m_pPlane;
	//call when ready to generate new octave
	void nextOctave();

	//get vector from sphere center to surface 
	DirectX::XMFLOAT3 sampleSphere(__int32 xIndex, __int32 yIndex) const;

	mutable std::shared_mutex m_membersLock;

	//K-times magnification algorithm (K=2).
	void produceScaledRegions() const;
	//Child maps will be moved from here on construction.
	//Not needed in any other cases, so it is marked as mutable.
	mutable std::array<HeightMapType, 4> m_heightMapScaledRegions;

public:
	TerrainMap(const TerrainMap& base, unsigned regionIdx);
	TerrainMap(SpherifiedPlane* plane, const Description& desc, MapLoader* pMapLoader);
	void loadTerrain(std::vector<DirectX::XMFLOAT4>& terrain, const std::array<bool, 4>& trueEdges = { true, true, true, true }) const;
	bool isComplete() const;
};
