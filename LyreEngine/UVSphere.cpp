#include "stdafx.h"
#include "UVSphere.h"

using namespace std;
using namespace DirectX;

inline XMFLOAT3 UVSphere::toDecart(float radius, float theta, float phi) {
	return {
		radius * sinf(theta) * cosf(phi),
		radius * sinf(theta) * sinf(phi),
		radius * cosf(theta)
	};
}

UVSphere::UVSphere(float radius, int stacks, int slices, bool reverseFacing) {
	vector<XMFLOAT3> vertices;
	vector<DWORD> indices;

	// north pole
	vertices.push_back(toDecart(radius, 0.0f, 0.0f));

	for (int i = 1; i < stacks; ++i) {
		float theta = (static_cast<float>(i) / stacks) * XM_PI;

		for (int j = 0; j < slices; ++j) {
			float phi = (static_cast<float>(j) / slices) * 2 * XM_PI;

			vertices.push_back(toDecart(radius, theta, phi));
		}
	} // (stacks - 1) * slices vertices produced + 2 poles

	// south pole
	vertices.push_back(toDecart(radius, XM_PI, 0.0f));

	// north pole indices
	for (DWORD i = 1; i <= slices; ++i) {
		DWORD i1 = 0, i2 = i, i3 = i % slices + 1;
		if (reverseFacing) {
			indices.insert(indices.end(), { i1, i3, i2 });
		}
		else {
			indices.insert(indices.end(), { i1, i2, i3 });
		}
	}

	for (DWORD i = 1; i < stacks - 1; ++i) {
		for (DWORD j = 0; j < slices; ++j) {
			DWORD i1 = (i - 1) * slices + j + 1,
				i2 = i * slices + j + 1, 
				i3 = (i - 1) * slices + (j + 1) % slices + 1,
				i4 = i3,
				i5 = i2,
				i6 = i * slices + (j + 1) % slices + 1;

			if (reverseFacing) {
				indices.insert(indices.end(), { i1, i3, i2 });
				indices.insert(indices.end(), { i4, i6, i5 });
			}
			else {
				indices.insert(indices.end(), { i1, i2, i3 });
				indices.insert(indices.end(), { i4, i5, i6 });
			}
		}
	}

	// south pole indices
	const DWORD lastIdx = (stacks - 1) * slices + 1; //vertices.size() - 1;
	for (DWORD i = 1; i <= slices; ++i) {
		DWORD offset = (stacks - 2) * slices;
		DWORD i1 = offset + i,
			i2 = lastIdx,
			i3 = offset + i % slices + 1;
		if (reverseFacing) {
			indices.insert(indices.end(), { i1, i3, i2 });
		}
		else {
			indices.insert(indices.end(), { i1, i2, i3 });
		}
	}

	addVertexElement(
		{ "VERTEX", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);

	loadVertices(vertices, 0);
	loadIndices(indices);
	setTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
