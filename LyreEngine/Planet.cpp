#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "SpherifiedPlane.h"
#include "FreeCamera.h"

using namespace std;
using namespace DirectX;

namespace {
	struct PlanetConstantBuffer {
		XMFLOAT3 planetViewPos;
		float radius;
	};
}

Planet::Planet(float radius) : m_sphere(radius) {}

HRESULT Planet::init() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;

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
		{ "CONTROL_POINT_WORLD_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = LyreEngine::getDevice()->CreateInputLayout(&layout[0], static_cast<UINT>(layout.size()), shaderBytecode.data(),
													shaderBytecode.size(), &m_iVertexLayout);
	if (FAILED(hr))
		return hr;

	//Hull shader
	hr = LyreEngine::readShaderFromFile(L"planet_hs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateHullShader(shaderBytecode.data(), VecBufferSize(shaderBytecode), nullptr, &m_iHS);
	if (FAILED(hr))
		return hr;

	//Domain shader
	LyreEngine::readShaderFromFile(L"planet_ds.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateDomainShader(shaderBytecode.data(), VecBufferSize(shaderBytecode), nullptr, &m_iDS);
	if (FAILED(hr))
		return hr;

	//Pixel shader
	hr = LyreEngine::readShaderFromFile(L"planet_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iPS);
	if (FAILED(hr))
		return hr;

	m_sphere.divide(2);
	m_sphere.distort();
	m_sphere.applyTopology();
	const vector<SpherifiedCube::Vertex>& vertices = m_sphere.vertices();

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(vertices));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	initData.pSysMem = vertices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &initData, &m_iVertexBuffer);
	if (FAILED(hr))
		return hr;

	//Setting index buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = static_cast<UINT>(VecBufferSize(m_sphere.indices));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	initData.pSysMem = m_sphere.indices.data();
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, &initData, &m_iIndexBuffer);
	if (FAILED(hr))
		return hr;

	//Planet constant buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(PlanetConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	}
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, nullptr, &m_iPlanetConstantBuffer);
	if (FAILED(hr))
		return hr;

	size_t patchesAmount = m_sphere.indices.size() / 9;
	D3D11_TEXTURE2D_DESC texArrayDesc;
	{
		ZeroStruct(texArrayDesc);
		texArrayDesc.Width = texArrayDesc.Height = HEIGHTMAP_RESOLUTION;
		texArrayDesc.MipLevels = 1;
		texArrayDesc.ArraySize = patchesAmount;
		texArrayDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texArrayDesc.SampleDesc.Count = 1;
		texArrayDesc.SampleDesc.Quality = 0;
		texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	CComPtr<ID3D11Texture2D> texArray = nullptr;
	hr = LyreEngine::getDevice()->CreateTexture2D(&texArrayDesc, nullptr, &texArray);
	if (FAILED(hr))
		return hr;
	for (int i = 0; i < patchesAmount; i++) {
		LyreEngine::getContext()->UpdateSubresource(texArray, i, nullptr, 
													&m_sphere.terrain[HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION*i],
													HEIGHTMAP_RESOLUTION * VecElementSize(m_sphere.terrain),
													HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION * VecElementSize(m_sphere.terrain));
	}
	hr = LyreEngine::getDevice()->CreateShaderResourceView(texArray, nullptr, &m_iTerrainSRV);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Planet::render() {
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	XMFLOAT4X4 view = LyreEngine::getCamera()->getView();

	PlanetConstantBuffer cbPlanet;
	XMStoreFloat3(&cbPlanet.planetViewPos, XMVector4Transform(XMVectorSetW(XMVectorZero(), 1.f), XMMatrixTranspose(XMLoadFloat4x4(&view))));
	cbPlanet.radius = m_sphere.getRadius();
	pContext->UpdateSubresource(m_iPlanetConstantBuffer, 0, nullptr, &cbPlanet, 0, 0);

	pContext->IASetInputLayout(m_iVertexLayout);
	pContext->IASetIndexBuffer(m_iIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST);
	UINT stride = VecElementSize(m_sphere.vertices());
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_iVertexBuffer.p, &stride, &offset);

	pContext->VSSetShader(m_iVS, nullptr, 0);
	m_cbuffers.fill(nullptr);
	m_cbuffers[0] = LyreEngine::getViewCB();
	pContext->VSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cbuffers.data());

	pContext->HSSetShader(m_iHS, nullptr, 0);
	m_cbuffers.fill(nullptr);
	m_cbuffers[0] = LyreEngine::getLodCB();
	pContext->HSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cbuffers.data());

	pContext->DSSetShader(m_iDS, nullptr, 0);
	m_cbuffers.fill(nullptr);
	m_cbuffers[0] = m_iPlanetConstantBuffer;
	m_cbuffers[1] = LyreEngine::getProjectionCB();
	m_cbuffers[2] = LyreEngine::getLightingCB();
	pContext->DSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cbuffers.data());
	m_samplers.fill(nullptr);
	m_samplers[0] = LyreEngine::getSampler2D();
	pContext->DSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_samplers.data());
	m_srvs.fill(nullptr);
	m_srvs[0] = m_iTerrainSRV;
	pContext->DSSetShaderResources(0, MAX_SRVS_AMOUNT, m_srvs.data());

	pContext->PSSetShader(m_iPS, nullptr, 0);

	pContext->DrawIndexed(static_cast<UINT>(m_sphere.indices.size()), 0, 0);
}
