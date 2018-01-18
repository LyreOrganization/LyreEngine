#pragma once

#include "stdafx.h"
#include "LyreEngine.h"

#define BUFFERS_AMOUNT D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

class GeometryDX {
	//Device
	ID3D11Device*					m_pDevice;
	ID3D11DeviceContext*			m_pContext;

	// IA data
	std::array<ID3D11Buffer*, BUFFERS_AMOUNT > m_vertexBuffers;
	CComPtr<ID3D11Buffer> m_indexBuffer;
	CComPtr<ID3D11InputLayout> m_layout;

	std::vector<D3D11_INPUT_ELEMENT_DESC> m_elements;

	D3D11_PRIMITIVE_TOPOLOGY m_topology;
	std::array<UINT, BUFFERS_AMOUNT> m_strides;

	void _loadVertices(const void* data, UINT size, UINT stride, UINT slot);
public:
	std::array<UINT, BUFFERS_AMOUNT> offsets;

	GeometryDX();
	~GeometryDX();

	template<class VertexStruct>
	void loadVertices(const std::vector<VertexStruct>& vertices, UINT slot) {
		_loadVertices(vertices.data(), vertices.size(), sizeof(VertexStruct), slot);
	}
	void loadVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT slot);
	void loadIndices(const std::vector<DWORD>& indices);

	D3D11_INPUT_ELEMENT_DESC& createVertexElement();
	void addVertexElement(D3D11_INPUT_ELEMENT_DESC);
	void loadLayout(const void* shaderByteCode, SIZE_T shaderByteCodeLength);

	void setTopology(D3D11_PRIMITIVE_TOPOLOGY);
	ID3D11Buffer* getVertexBuffer(UINT slot);

	void bind();
	void unbind();
};
