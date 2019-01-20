#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "TerrainMap.h"
#include "FreeCamera.h"
#include "UtilsDX.h"

using namespace std;
using namespace DirectX;

namespace {
	struct Geometry {
		XMFLOAT4 position;
		XMFLOAT4 color;
	};

	struct Normal {
		XMFLOAT4 position;
	};
}

#define MAX_PATCHES D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION 

Planet::Planet(float radius, unsigned seed) 
	: m_sphere(radius, seed), m_lodAdapter(&m_sphere) {
	m_lodAdapter.start();
}

HRESULT Planet::setupStreamOutputBuffers() {
	HRESULT hr = S_OK;

	ID3D11Buffer* buffer = UtilsDX::createStreamOutputBuffer(64 * 64 * MAX_PATCHES * sizeof(Geometry));
	m_renderConfig.setSOBuffer(buffer, 0);
	m_geometryPipeline.geometry.loadVertexBuffer(buffer, sizeof(Geometry), 0);

	buffer = UtilsDX::createStreamOutputBuffer(64 * 64 * MAX_PATCHES * sizeof(Normal) * 2);
	m_renderConfig.setSOBuffer(buffer, 1);
	m_normalsPipeline.geometry.loadVertexBuffer(buffer, sizeof(Normal), 0);

	return hr;
}

HRESULT Planet::initGeometryShader() {
	vector<UINT> buffersStrides = { sizeof(Geometry), sizeof(Normal) * 2 };

	m_renderConfig.addSOEntry(
	{ 0, "POSITION", 0, 0, 4, 0 }
	);
	m_renderConfig.addSOEntry(
	{ 0, "COLOR", 0, 0, 4, 0 }
	);
	m_renderConfig.addSOEntry(
	{ 1, "POSITION", 0, 0, 4, 1 }
	);
	m_renderConfig.addSOEntry(
	{ 1, "POSITION", 1, 0, 4, 1 }
	);

	m_renderConfig.loadGSwithSO(L"planet_gs.cso", buffersStrides, D3D11_SO_NO_RASTERIZED_STREAM);

	return S_OK;
}

HRESULT Planet::initGeometryPipeline() {
	//Vertex shader
	std::vector<char> shaderBytecode = m_geometryPipeline.config.loadShader(Shader::VS, L"planet_geometry_vs.cso");
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
	m_geometryPipeline.config.loadShader(Shader::PS, L"planet_geometry_ps.cso");

	return S_OK;
}

HRESULT Planet::initNormalsPipeline() {
	//Vertex shader
	std::vector<char> shaderBytecode = m_normalsPipeline.config.loadShader(Shader::VS, L"planet_normals_vs.cso");

	///Vertex buffer format
	m_normalsPipeline.geometry.addVertexElement(
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_normalsPipeline.geometry.loadLayout(shaderBytecode.data(), shaderBytecode.size());
	m_normalsPipeline.geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	//Pixel shader
	m_normalsPipeline.config.loadShader(Shader::PS, L"planet_normals_ps.cso");

	return S_OK;
}

HRESULT Planet::initGeometryAndVS() {
	std::vector<char> shaderBytecode = m_renderConfig.loadShader(Shader::VS, L"planet_vs.cso");

	m_geometry.addVertexElement(
	{ "POSITION", 0, DXGI_FORMAT_R32G32_SINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_geometry.addVertexElement(
	{ "FACE_AND_FLAGS", 0, DXGI_FORMAT_R32_SINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	m_geometry.addVertexElement(
	{ "LEVEL", 0, DXGI_FORMAT_R32_SINT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);

	m_geometry.loadLayout(shaderBytecode.data(), shaderBytecode.size());
	m_geometry.loadVertices<SpherifiedPlane::GPUDesc>(MAX_PATCHES * 4);
	m_geometry.loadIndices(MAX_PATCHES * 5);
	m_geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST);

	return S_OK;
}

HRESULT Planet::precomputeHeightMap() {
	HRESULT hr;

	SphereTopology* topology = m_sphere.getTopology();
	size_t patchesAmount = topology->indices.size() / 5;
	m_sphere.releseTopology();
	D3D11_TEXTURE1D_DESC desc;
	{
		ZeroStruct(desc);
		desc.Width = patchesAmount;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	}
	CComPtr<ID3D11Texture1D> deviations = nullptr;
	hr = LyreEngine::getDevice()->CreateTexture1D(&desc, nullptr, &deviations);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateUnorderedAccessView(deviations, nullptr, &m_iCurvMapUAV);
	if (FAILED(hr))
		return hr;

	m_precompHeightmap.loadShader(L"planet_cs.cso");
	m_precompHeightmap.setSRV(m_iTerrainSRV, 0);
	m_precompHeightmap.setUAV(m_iCurvMapUAV, 0);

	m_precompHeightmap.bind();

	LyreEngine::getContext()->Dispatch(patchesAmount, 1, 1);

	m_precompHeightmap.unbind();

	CComPtr<ID3D11ShaderResourceView> srv;
	LyreEngine::getDevice()->CreateShaderResourceView(deviations, nullptr, &srv);

	m_renderConfig.setSRV(Shader::HS, srv, 0);

	return S_OK;
}

HRESULT Planet::init() {
	HRESULT hr;

	//IA and VS
	initGeometryAndVS();

	//Hull shader
	m_renderConfig.loadShader(Shader::HS, L"planet_hs.cso");

	//Domain shader
	m_renderConfig.loadShader(Shader::DS, L"planet_ds.cso");

	//Geometry shader
	if (FAILED(hr = setupStreamOutputBuffers()) ||
		FAILED(hr = initGeometryShader()) ||
		FAILED(hr = initGeometryPipeline()) ||
		FAILED(hr = initNormalsPipeline()))
		return hr;

	D3D11_TEXTURE2D_DESC texArrayDesc;
	{
		ZeroStruct(texArrayDesc);
		texArrayDesc.Width = texArrayDesc.Height = static_cast<UINT>(TerrainMap::RESOLUTION);;
		texArrayDesc.MipLevels = 1;
		texArrayDesc.ArraySize = static_cast<UINT>(MAX_PATCHES);
		texArrayDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texArrayDesc.SampleDesc.Count = 1;
		texArrayDesc.SampleDesc.Quality = 0;
		texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	hr = LyreEngine::getDevice()->CreateTexture2D(&texArrayDesc, nullptr, &m_iTerrainTexArray);
	if (FAILED(hr))
		return hr;
	hr = LyreEngine::getDevice()->CreateShaderResourceView(m_iTerrainTexArray, nullptr, &m_iTerrainSRV);
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC bufferDesc;
	{
		ZeroStruct(bufferDesc);
		bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		bufferDesc.ByteWidth = MAX_PATCHES * sizeof(float);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	CComPtr<ID3D11Buffer> buffer = nullptr;
	hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer);
	if (FAILED(hr))
		return hr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	{
		ZeroStruct(uavDesc);
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
		uavDesc.Buffer.NumElements = MAX_PATCHES;
	}
	hr = LyreEngine::getDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &m_iLodDiffUAV);
	if (FAILED(hr))
		return hr;

	/*hr = precomputeHeightMap();
	if (FAILED(hr))
		return hr;*/

	return S_OK;
}

void Planet::render() {
	static const UINT RES = static_cast<UINT>(TerrainMap::RESOLUTION);

	static int numPatches = 0;
	
	auto topologyDeleter = [this](SphereTopology* p) { 
		if (p != nullptr) m_sphere.releseTopology(); 
	};
	unique_ptr<SphereTopology, decltype(topologyDeleter)> pNewTopology(
		m_sphere.getTopology(), topologyDeleter);
	if (pNewTopology) {
		numPatches = static_cast<int>(pNewTopology->indices.size()) / 5;
		if (numPatches == 0) return;
		m_geometry.updateVertices(pNewTopology->planes);
		m_geometry.updateIndices(pNewTopology->indices);
		for (int i = 0; i < numPatches; i++) {
			LyreEngine::getContext()->UpdateSubresource(m_iTerrainTexArray, i, nullptr,
														&pNewTopology->terrain[RES*RES*i],
														RES * VecElementSize(pNewTopology->terrain),
														RES * RES * VecElementSize(pNewTopology->terrain));
		}
	}
	if (numPatches == 0) return;

	for (int i = 0; i < 6; i++) {
		m_cubeFacesCb.data.planeRotations[i] = SpherifiedCube::getFaceRotation(i);
	}
	m_cubeFacesCb.update();
	XMStoreFloat3(&m_planetCb.data.planetPos, XMVectorZero());
	m_planetCb.data.radius = m_sphere.getRadius();
	m_planetCb.update();

	m_renderConfig.setConstantBuffer(Shader::VS, m_cubeFacesCb.getBuffer(), 0);
	m_renderConfig.setConstantBuffer(Shader::VS, m_planetCb.getBuffer(), 1);
	m_renderConfig.setConstantBuffer(Shader::VS, LyreEngine::getCameraCB(), 2);
	m_renderConfig.setConstantBuffer(Shader::VS, LyreEngine::getLodCB(), 3);

	m_renderConfig.setConstantBuffer(Shader::DS, m_cubeFacesCb.getBuffer(), 0);
	m_renderConfig.setConstantBuffer(Shader::DS, m_planetCb.getBuffer(), 1);
	m_renderConfig.setConstantBuffer(Shader::DS, LyreEngine::getLightingCB(), 2);
	m_renderConfig.setSampler(Shader::DS, LyreEngine::getSamplerPoint(), 0);
	m_renderConfig.setSampler(Shader::DS, LyreEngine::getSamplerLinear(), 1);
	m_renderConfig.setSRV(Shader::DS, m_iTerrainSRV, 0);

	m_renderConfig.setUAV(m_iLodDiffUAV, 0);

	m_geometry.bind();
	m_renderConfig.bind();

	LyreEngine::getContext()->DrawIndexed(static_cast<UINT>(numPatches) * 5, 0, 0);

	m_renderConfig.unbind();

	if (pNewTopology) m_lodAdapter.readCurrentLods(m_iLodDiffUAV, *pNewTopology);

	renderGeometry();
	//renderNormals();
}

void Planet::renderGeometry() {
	m_geometryPipeline.geometry.bind();

	m_geometryPipeline.config.setConstantBuffer(Shader::VS, LyreEngine::getViewProjCB(), 0);
	m_geometryPipeline.config.bind();

	LyreEngine::getContext()->DrawAuto();
}

void Planet::renderNormals() {
	m_normalsPipeline.geometry.bind();

	m_normalsPipeline.config.setConstantBuffer(Shader::VS, LyreEngine::getViewProjCB(), 0);
	m_normalsPipeline.config.bind();

	LyreEngine::getContext()->DrawAuto();
}
