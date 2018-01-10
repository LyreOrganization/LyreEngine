#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "PerlinGrid.h"

using namespace std;
using namespace DirectX;

HRESULT Planet::init() {
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
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iVS);
	if (FAILED(hr))
		return hr;
	///Vertex buffer format
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout
	{
		//  { SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate }
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = LyreEngine::getDevice()->CreateInputLayout(&layout[0], static_cast<UINT>(layout.size()), shaderBytecode.data(),
													shaderBytecode.size(), &m_iVertexLayout);
	if (FAILED(hr))
		return hr;

	//Pixel shader
	hr = LyreEngine::ReadShaderFromFile(L"planet_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iPS);
	if (FAILED(hr))
		return hr;

	SpherifiedCube cube(2);
	cube.divide(7);
	m_vertices = cube.getVertices();
	m_indices = cube.getIndicesBuffer();

	PerlinGrid noise(777); // argument - seed for random
	for (auto& vertex : m_vertices) { //works for random mesh
		XMVECTOR posOriginal = XMLoadFloat3(&vertex.position);
		XMVECTOR posCurrent = posOriginal;
		XMFLOAT3 scaledPos;
		for (int i = 0; i < 6/*iMAX*/; i++) { // fractal, iMAX octaves
			XMStoreFloat3(&scaledPos, posOriginal * (1 << i/*octave*/));
			float height = noise.perlinNoise(scaledPos);
			if (i == 0) height = floor(height * 4.f) / 4.f; //kanion
			else if (i < 3) continue; // skip 2 octaves
			else height = fabs(height); // erosion
			posCurrent += posOriginal*(height / (5.f*(1 << i/*amplitude*/)));
		}
		XMStoreFloat3(&vertex.position, posCurrent);
	}

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(m_vertices));
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
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(m_indices));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	InitData.pSysMem = m_indices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &InitData, &m_iIndexBuffer);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Planet::render() {
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	m_cbuffers.fill(nullptr);

	pContext->IASetInputLayout(m_iVertexLayout);
	UINT stride = VecElementSize(m_vertices); UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_iVertexBuffer.p, &stride, &offset);
	pContext->IASetIndexBuffer(m_iIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pContext->VSSetShader(m_iVS, nullptr, 0);
	m_cbuffers[0] = LyreEngine::getViewProj();
	pContext->VSSetConstantBuffers(0, static_cast<UINT>(m_cbuffers.size()), m_cbuffers.data());

	pContext->PSSetShader(m_iPS, nullptr, 0);

	pContext->DrawIndexed(static_cast<UINT>(m_indices.size()), 0, 0);
}
