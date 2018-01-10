#pragma once

class SpherifiedPlane;

class SpherifiedCube final {
public:
	struct Vertex {
		DirectX::XMFLOAT3 position;
	};

private:
	float m_radius;
	std::array<std::unique_ptr<SpherifiedPlane>, 6> m_cube;
	std::vector<Vertex> m_vertices;

	void buildCube();

public:
	SpherifiedCube(float radius);
	~SpherifiedCube();
	void divide(unsigned depth);
	DWORD createHalf(DWORD point1, DWORD point2);
	std::vector<DWORD> getIndicesBuffer();
	std::vector<Vertex> getVertices() const;
};
