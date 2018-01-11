#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"

using namespace std;
using namespace DirectX;

Planet::Planet() : m_sphere(1.) {}

static size_t indicesAmount = 0; //TODO
static UINT verticesBufSize = 0;

HRESULT Planet::init() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA InitData;

	UINT width, height;
	LyreEngine::getClientWH(width, height);

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = LyreEngine::readShaderFromFile(L"planet_vs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iVS);
	if (FAILED(hr))
		return hr;
	///Vertex buffer format
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout
	{
		//  { SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate }
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = LyreEngine::getDevice()->CreateInputLayout(&layout[0], static_cast<UINT>(layout.size()), shaderBytecode.data(),
													shaderBytecode.size(), &m_iVertexLayout);
	if (FAILED(hr))
		return hr;

	//Pixel shader
	hr = LyreEngine::readShaderFromFile(L"planet_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iPS);
	if (FAILED(hr))
		return hr;

	m_sphere.divide(8);
	m_sphere.distort();
	vector<SpherifiedCube::Vertex> vertices = m_sphere.getVertices();
	vector<DWORD> indices = m_sphere.getIndicesBuffer();

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(vertices));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	InitData.pSysMem = vertices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &InitData, &m_iVertexBuffer);
	if (FAILED(hr))
		return hr;
	verticesBufSize = static_cast<UINT>(VecBufferSize(vertices));

	//Setting index buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(indices));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	InitData.pSysMem = indices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &InitData, &m_iIndexBuffer);
	if (FAILED(hr))
		return hr;
	indicesAmount = indices.size();

	return S_OK;
}

void Planet::render() {
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	m_cbuffers.fill(nullptr);

	pContext->IASetInputLayout(m_iVertexLayout);
	
	pContext->IASetIndexBuffer(m_iIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pContext->VSSetShader(m_iVS, nullptr, 0);
	UINT stride = sizeof(SpherifiedCube::Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_iVertexBuffer.p, &stride, &offset);
	m_cbuffers[0] = LyreEngine::getViewProj();
	m_cbuffers[1] = LyreEngine::getLighting();
	pContext->VSSetConstantBuffers(0, static_cast<UINT>(m_cbuffers.size()), m_cbuffers.data());

	pContext->PSSetShader(m_iPS, nullptr, 0);

	pContext->DrawIndexed(static_cast<UINT>(indicesAmount), 0, 0);
}
