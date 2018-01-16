#include "stdafx.h"
#include "D3DGeometry.h"

D3DGeometry::D3DGeometry() {
	m_pDevice = LyreEngine::getDevice();
	m_pContext = LyreEngine::getContext();

	m_vertexBuffers.fill(nullptr);
	m_strides.fill(0);
	offsets.fill(0);
}

D3DGeometry::~D3DGeometry() {
	for (auto buffer : m_vertexBuffers) {
		if (buffer) {
			buffer->Release();
		}
	}
}

void D3DGeometry::loadVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT slot) {
	m_vertexBuffers[slot] = buffer;
	m_strides[slot] = stride;
}

void D3DGeometry::loadIndices(const std::vector<DWORD>& indices) {
	D3D11_BUFFER_DESC desc;
	{
		ZeroStruct(desc);
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.ByteWidth = indices.size() * sizeof(DWORD);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	D3D11_SUBRESOURCE_DATA initData;
	{
		initData.pSysMem = indices.data();
	}
	HRESULT hr = m_pDevice->CreateBuffer(&desc, &initData, &m_indexBuffer);
	if (FAILED(hr)) {
		throw std::runtime_error("D3DGeometry.loadIndices: device->CreateBuffer() failed.");
	}
}

D3D11_INPUT_ELEMENT_DESC& D3DGeometry::createVertexElement() {
	m_elements.emplace_back();
	return m_elements.back();
}

void D3DGeometry::addVertexElement(D3D11_INPUT_ELEMENT_DESC element) {
	m_elements.push_back(element);
}

void D3DGeometry::loadLayout(const void* shaderByteCode, SIZE_T shaderByteCodeLength) {
	HRESULT hr = m_pDevice->CreateInputLayout(
		m_elements.data(), 
		m_elements.size(), 
		shaderByteCode, 
		shaderByteCodeLength,
		&m_layout
	);

	if (FAILED(hr)) {
		throw std::runtime_error("D3DGeometry.loadLayout: device->CreateInputLayout() failed.");
	}
}

void D3DGeometry::setTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
	m_topology = topology;
}

ID3D11Buffer * D3DGeometry::getVertexBuffer(UINT slot) {
	return m_vertexBuffers[slot];
}

void D3DGeometry::bind() {
	m_pContext->IASetInputLayout(m_layout);
	m_pContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pContext->IASetPrimitiveTopology(m_topology);
	m_pContext->IASetVertexBuffers(
		0, 
		m_vertexBuffers.size(), 
		m_vertexBuffers.data(), 
		m_strides.data(), 
		offsets.data()
	);
}

void D3DGeometry::unbind() {
	m_pContext->IASetInputLayout(nullptr);
	m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);

	/*std::array<ID3D11Buffer*, BUFFERS_AMOUNT> null;
	null.fill(nullptr);
	context->IASetVertexBuffers(
		0,
		null.size(),
		null.data(),
		m_strides.data(),
		offsets.data()
	);*/
}
