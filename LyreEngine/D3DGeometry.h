#pragma once

#include "stdafx.h"
#include "LyreEngine.h"

#define BUFFERS_AMOUNT D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

class D3DGeometry {
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
public:
	std::array<UINT, BUFFERS_AMOUNT> offsets;

	D3DGeometry();
	~D3DGeometry();

	template<class T>
	void loadVertices(const std::vector<T>& vertices, UINT slot) {
		m_strides[slot] = sizeof(T);

		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.ByteWidth = vertices.size() * sizeof(T);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		}
		D3D11_SUBRESOURCE_DATA initData;
		{
			initData.pSysMem = vertices.data();
		}
		HRESULT hr = LyreEngine::getDevice()->CreateBuffer(&desc, &initData, &m_vertexBuffers[slot]);
		if (FAILED(hr)) {
			throw std::runtime_error("D3DGeometry.loadVertices: device->CreateBuffer() failed.");
		}
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
