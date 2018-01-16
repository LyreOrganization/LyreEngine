#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "SpherifiedPlane.h"
#include "FreeCamera.h"
#include "Utils.h"

using namespace std;
using namespace DirectX;

namespace {
	struct PlanetConstantBuffer {
		XMFLOAT3 planetViewPos;
		float radius;
	};

	struct Geometry {
		XMFLOAT4 position;
		XMFLOAT4 color;
	};

	struct Normal {
		XMFLOAT4 position;
	};
}

Planet::Planet(float radius) : m_sphere(radius) {}

HRESULT Planet::setupStreamOutputBuffers() {
	HRESULT hr;

	ID3D11Buffer* buffer = Utils::createStreamOutputBuffer((63 * 63 * 2 * 3) * (6 * 4 * 4) * sizeof(Geometry));
	m_geometryPipeline.geometry.loadVertexBuffer(buffer, sizeof(Geometry), 0);

	buffer = Utils::createStreamOutputBuffer((63 * 63 * 2 * 3) * (6 * 4 * 4) * sizeof(Normal) * 2);
	m_normalsPipeline.geometry.loadVertexBuffer(buffer, sizeof(Normal), 0);

	m_soBuffers.second.fill(0);

	return S_OK;
}

HRESULT Planet::initGeometryShader() {
	HRESULT hr;

	std::vector<char> shaderBytecode;
	hr = LyreEngine::readShaderFromFile(L"planet_gs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;

	vector<D3D11_SO_DECLARATION_ENTRY> entries;
	D3D11_SO_DECLARATION_ENTRY entry;
	{
		ZeroStruct(entry);
		entry.Stream = 0;
		entry.SemanticName = "POSITION";
		entry.SemanticIndex = 0;
		entry.StartComponent = 0;
		entry.ComponentCount = 4;
		entry.OutputSlot = 0;
	}
	entries.push_back(entry);
	{
		ZeroStruct(entry);
		entry.Stream = 0;
		entry.SemanticName = "COLOR";
		entry.SemanticIndex = 0;
		entry.StartComponent = 0;
		entry.ComponentCount = 4;
		entry.OutputSlot = 0;
	}
	entries.push_back(entry);
	{
		ZeroStruct(entry);
		entry.Stream = 1;
		entry.SemanticName = "POSITION";
		entry.SemanticIndex = 0;
		entry.StartComponent = 0;
		entry.ComponentCount = 4;
		entry.OutputSlot = 1;
	}
	entries.push_back(entry);
	{
		ZeroStruct(entry);
		entry.Stream = 1;
		entry.SemanticName = "POSITION";
		entry.SemanticIndex = 1;
		entry.StartComponent = 0;
		entry.ComponentCount = 4;
		entry.OutputSlot = 1;
	}
	entries.push_back(entry);

	UINT bufferStrides[] = { sizeof(Geometry), sizeof(Normal)*2 };

	hr = LyreEngine::getDevice()->CreateGeometryShaderWithStreamOutput(
		shaderBytecode.data(), shaderBytecode.size(), 
		entries.data(), entries.size(), 
		bufferStrides, 2, 
		D3D11_SO_NO_RASTERIZED_STREAM, 
		nullptr, 
		&m_iGS
	);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Planet::initGeometryPipeline() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = LyreEngine::readShaderFromFile(L"planet_geometry_vs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_geometryPipeline.iVS);
	if (FAILED(hr))
		return hr;
	///Vertex buffer format
	m_geometryPipeline.geometry.addVertexElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_geometryPipeline.geometry.addVertexElement(
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_geometryPipeline.geometry.loadLayout(shaderBytecode.data(), shaderBytecode.size());
	m_geometryPipeline.geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Pixel shader
	hr = LyreEngine::readShaderFromFile(L"planet_geometry_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_geometryPipeline.iPS);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Planet::initNormalsPipeline() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = LyreEngine::readShaderFromFile(L"planet_normals_vs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_normalsPipeline.iVS);
	if (FAILED(hr))
		return hr;
	///Vertex buffer format
	m_normalsPipeline.geometry.addVertexElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_normalsPipeline.geometry.loadLayout(shaderBytecode.data(), shaderBytecode.size());
	m_normalsPipeline.geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//Pixel shader
	hr = LyreEngine::readShaderFromFile(L"planet_normals_ps.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_normalsPipeline.iPS);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Planet::initGeometryAndVS() {
	HRESULT hr;
	std::vector<char> shaderBytecode;

	hr = LyreEngine::readShaderFromFile(L"planet_vs.cso", shaderBytecode);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &m_iVS);
	if (FAILED(hr))
		return hr;

	m_geometry.addVertexElement(
		{"CONTROL_POINT_WORLD_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);

	/*auto& vertexElement = m_geometry.createVertexElement();
	vertexElement.SemanticName = "CONTROL_POINT_WORLD_POSITION";
	vertexElement.SemanticIndex = 0;
	vertexElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexElement.InputSlot = 0;
	vertexElement.AlignedByteOffset = 0;
	vertexElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vertexElement.InstanceDataStepRate = 0;*/

	m_sphere.divide(0);
	m_sphere.distort();
	m_sphere.applyTopology();

	m_geometry.loadLayout(shaderBytecode.data(), shaderBytecode.size());
	m_geometry.loadVertices(m_sphere.vertices(), 0);
	m_geometry.loadIndices(m_sphere.indices);
	m_geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST);

	return S_OK;
}

HRESULT Planet::init() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;

	//IA and VS
	initGeometryAndVS();

	std::vector<char> shaderBytecode;

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

	//Geometry shader
	if (FAILED(hr = setupStreamOutputBuffers()) ||
		FAILED(hr = initGeometryShader()) ||
		FAILED(hr = initGeometryPipeline()) ||
		FAILED(hr = initNormalsPipeline()))
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

	XMFLOAT4X4 view = LyreEngine::getCamera()->calculateViewMatrix();

	PlanetConstantBuffer cbPlanet;
	XMStoreFloat3(&cbPlanet.planetViewPos, XMVector4Transform(XMVectorSetW(XMVectorZero(), 1.f), XMMatrixTranspose(XMLoadFloat4x4(&view))));
	cbPlanet.radius = m_sphere.getRadius();
	pContext->UpdateSubresource(m_iPlanetConstantBuffer, 0, nullptr, &cbPlanet, 0, 0);

	m_geometry.bind();

	pContext->VSSetShader(m_iVS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = LyreEngine::getViewCB();
	pContext->VSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());

	pContext->HSSetShader(m_iHS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = LyreEngine::getLodCB();
	pContext->HSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());

	pContext->DSSetShader(m_iDS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = m_iPlanetConstantBuffer;
	m_cBuffers[1] = LyreEngine::getLightingCB();
	pContext->DSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());
	m_samplers.fill(nullptr);
	m_samplers[0] = LyreEngine::getSampler2D();
	pContext->DSSetSamplers(0, MAX_SAMPLERS_AMOUNT, m_samplers.data());
	m_srvs.fill(nullptr);
	m_srvs[0] = m_iTerrainSRV;
	pContext->DSSetShaderResources(0, MAX_SRVS_AMOUNT, m_srvs.data());

	pContext->GSSetShader(m_iGS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = LyreEngine::getViewCB();
	pContext->GSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());
	m_soBuffers.first.fill(nullptr);
	m_soBuffers.first[0] = m_geometryPipeline.geometry.getVertexBuffer(0);
	m_soBuffers.first[1] = m_normalsPipeline.geometry.getVertexBuffer(0);
	pContext->SOSetTargets(MAX_SOBUFFERS_AMOUNT, m_soBuffers.first.data(), m_soBuffers.second.data());

	pContext->PSSetShader(nullptr, nullptr, 0);

	pContext->DrawIndexed(static_cast<UINT>(m_sphere.indices.size()), 0, 0);

	pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->GSSetShader(nullptr, nullptr, 0);
	m_soBuffers.first.fill(nullptr);
	pContext->SOSetTargets(MAX_SOBUFFERS_AMOUNT, m_soBuffers.first.data(), m_soBuffers.second.data());

	renderGeometry();
	renderNormals();
}

void Planet::renderGeometry() {
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	m_geometryPipeline.geometry.bind();

	pContext->VSSetShader(m_geometryPipeline.iVS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = LyreEngine::getProjectionCB();
	pContext->VSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());

	pContext->PSSetShader(m_geometryPipeline.iPS, nullptr, 0);

	pContext->DrawAuto();
}

void Planet::renderNormals() {
	ID3D11DeviceContext* pContext = LyreEngine::getContext();

	m_normalsPipeline.geometry.bind();

	pContext->VSSetShader(m_normalsPipeline.iVS, nullptr, 0);
	m_cBuffers.fill(nullptr);
	m_cBuffers[0] = LyreEngine::getProjectionCB();
	pContext->VSSetConstantBuffers(0, MAX_CBUFFERS_AMOUNT, m_cBuffers.data());

	pContext->PSSetShader(m_normalsPipeline.iPS, nullptr, 0);

	pContext->DrawAuto();
}
