#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "SpherifiedCube.h"

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

	SpherifiedCube cube(1);
	m_vertices = cube.getVertices();
	m_indices = cube.getIndicesBuffer();

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
	vector<ID3D11Buffer*> cbuffers(16, 0);
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	pContext->VSSetShader(m_iVS, nullptr, 0);
	pContext->IASetInputLayout(m_iVertexLayout);
	UINT stride = VecElementSize(m_vertices); UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_iVertexBuffer.p, &stride, &offset);
	pContext->IASetIndexBuffer(m_iIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cbuffers[0] = LyreEngine::getViewProj();
	pContext->VSSetConstantBuffers(0, 16, cbuffers.data());

	pContext->PSSetShader(m_iPS, nullptr, 0);

	pContext->DrawIndexed(m_indices.size(), 0, 0);
}