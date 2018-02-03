#pragma once

#define HEIGHTMAP_RESOLUTION 64

// All together terrain maps form six "region quadtrees". The only difference from 
// region quadtrees described in Wikipedia is that the tree is actually not a trie: 
// leafs contain full terrain map of their surface subregion. Still they represent 
// a variable resolution data field. Children copy map region from parent adding new
// higher-resolution details to it.

class TerrainMap final {
	friend class MapLoader;

public:
	struct Description final {
		float amplitude;
		float octave;
		float shift;
		int currentOctaveDepth;
		DirectX::XMFLOAT3 quad[4];
	};

private:
	MapLoader* m_pMapLoader;

	typedef std::vector<DirectX::XMFLOAT4> HeightMapType;

	HeightMapType m_heightMap;
	std::array<HeightMapType, 4> m_edgesBackup;

	//configuration for MapLoader
	Description m_desc;
	//call when ready to generate new octave
	void nextOctave();

	//get vector from sphere center to surface 
	DirectX::XMFLOAT3 sampleSphere(float u, float v) const;

	mutable std::shared_mutex m_membersLock;

	//K-times magnification algorithm (K=2).
	//Operation is done for all 4 regions all together
	//to keep algorithm simplicity and code readability.
	void produceScaledRegions() const;
	//Child maps will be moved from here on construction.
	//Not needed in any other cases, so it is marked as mutable.
	mutable std::array<HeightMapType, 4> m_heightMapScaledRegions;

	bool bDeleteMark;

public:
	TerrainMap(const TerrainMap& base, unsigned regionIdx);
	TerrainMap(const Description& desc, MapLoader* pMapLoader);
	void loadTerrain(std::vector<DirectX::XMFLOAT4>& terrain) const;
	bool isComplete() const;
};
