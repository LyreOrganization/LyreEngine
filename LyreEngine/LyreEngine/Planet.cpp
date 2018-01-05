#include "Planet.h"

#include "stdafx.h"

#include "LyreEngine.h"

using namespace std;
using namespace DirectX;

HRESULT Planet::init()
{
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA InitData;

	UINT width, height;
	LyreEngine::GetClientWH(width, height);

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = LyreEngine::ReadShaderFromFile(L"planet_vs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), VecBufferSize(shaderBytecode), nullptr, &m_iVS);
	if (FAILED(hr))
		return hr;
	///Vertex buffer format
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = LyreEngine::getDevice()->CreateInputLayout(&layout[0], layout.size(), shaderBytecode.data(),
		VecBufferSize(shaderBytecode), &m_iVertexLayout);
	if (FAILED(hr))
		return hr;

	//Pixel shader
	hr = LyreEngine::ReadShaderFromFile(L"planet_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), VecBufferSize(shaderBytecode), nullptr, &m_iPS);
	if (FAILED(hr))
		return hr;

	m_vertices.push_back(XMFLOAT3(-1., -1., -1.));
	m_vertices.push_back(XMFLOAT3(-1., -1., 1.));
	m_vertices.push_back(XMFLOAT3(-1., 1., 1.));
	m_vertices.push_back(XMFLOAT3(-1., 1., -1.));
	m_vertices.push_back(XMFLOAT3(1., -1., -1.));
	m_vertices.push_back(XMFLOAT3(1., -1., 1.));
	m_vertices.push_back(XMFLOAT3(1., 1., 1.));
	m_vertices.push_back(XMFLOAT3(1., 1., -1.));

	m_indices.push_back(0);
	m_indices.push_back(1);
	m_indices.push_back(2);
	m_indices.push_back(0);
	m_indices.push_back(2);
	m_indices.push_back(3);
	m_indices.push_back(4);
	m_indices.push_back(5);
	m_indices.push_back(6);
	m_indices.push_back(4);
	m_indices.push_back(6);
	m_indices.push_back(7);

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(m_vertices);
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	InitData.pSysMem = m_vertices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &InitData, &m_iVertexBuffer);
	if (FAILED(hr))
		return hr;

	//Setting index buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(m_indices);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	InitData.pSysMem = m_indices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &InitData, &m_iIndexBuffer);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Planet::render()
{
	LyreEngine::getContext()->VSSetShader(m_iVS, nullptr, 0);
	LyreEngine::getContext()->PSSetShader(m_iPS, nullptr, 0);

	LyreEngine::getContext()->IASetInputLayout(m_iVertexLayout);
	UINT stride = VecElementSize(m_vertices); UINT offset = 0;
	LyreEngine::getContext()->IASetVertexBuffers(0, 1, &m_iVertexBuffer.p, &stride, &offset);
	LyreEngine::getContext()->IASetIndexBuffer(m_iIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	LyreEngine::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	LyreEngine::getContext()->DrawIndexed(m_indices.size(), 0, 0);
}