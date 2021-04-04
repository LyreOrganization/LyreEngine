#pragma once

#include "SpherifiedPlane.h"
#include "MapLoader.h"

class SphereTopology final {
	friend class SpherifiedCube;

	std::vector<DirectX::XMFLOAT4> m_terrain;
	std::vector<SpherifiedPlane::GPUDesc> m_planes;
	std::vector<DWORD> m_indices;

public:
	SphereTopology() = default;
	SphereTopology(const SphereTopology&) = delete;
	SphereTopology& operator=(const SphereTopology&) = delete;
	SphereTopology(SphereTopology&& r);
	SphereTopology& operator=(SphereTopology&& r);

	const std::vector<DirectX::XMFLOAT4>& terrain = m_terrain;
	const std::vector<SpherifiedPlane::GPUDesc>& planes = m_planes;
	const std::vector<DWORD>& indices = m_indices;
};

class SpherifiedCube final {
private:
	float m_radius;
	std::array<std::unique_ptr<SpherifiedPlane>, 6> m_cube;

	MapLoader m_mapLoader;

	SphereTopology m_topology;
	std::shared_mutex m_topologyLock;

	void buildCube();

public:
	SpherifiedCube(float radius, unsigned seed);
	~SpherifiedCube();
	float getRadius() const;

	// load changes into topology buffers
	void loadTopology();
	// call releseTopology() to unlock, if object returned
	SphereTopology* getTopology();
	void releseTopology();

	static const DirectX::XMFLOAT4X4& getFaceRotation(int faceIdx);

	// tryUndivide() on any other SpherifiedPlane of
	// this object can invalidate the returned pointer.
	SpherifiedPlane* getPlane(const SpherifiedPlane::GPUDesc& desc);
};
