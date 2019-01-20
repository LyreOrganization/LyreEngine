#include "stdafx.h"

#include "Planet.h"

#include "LyreEngine.h"
#include "SpherifiedPlane.h"
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

Planet::Planet(float radius) : m_sphere(radius) {}

HRESULT Planet::setupStreamOutputBuffers() {
	HRESULT hr;

	CComPtr<ID3D11Buffer> geometry = UtilsDX::createStreamOutputBuffer((63 * 63 * 2 * 3) * (6 * 4 * 4) * sizeof(Geometry));
	m_renderConfig.setSOBuffer(geometry, 0);
	m_geometryPipeline.geometry.loadVertexBuffer(geometry, sizeof(Geometry), 0);

	CComPtr<ID3D11Buffer> normals = UtilsDX::createStreamOutputBuffer((63 * 63 * 2 * 3) * (6 * 4 * 4) * sizeof(Normal) * 2);
	m_renderConfig.setSOBuffer(normals, 1);
	m_normalsPipeline.geometry.loadVertexBuffer(normals, sizeof(Normal), 0);

	return S_OK;
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

HRESULT Planet::precomputeHeightMap() {
	HRESULT hr;

	size_t patchesAmount = m_sphere.indices.size() / 9;
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

	hr = precomputeHeightMap();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Planet::render() {
	if (!visible) return;

	XMFLOAT4X4 view = LyreEngine::getCamera()->calculateViewMatrix();

	XMStoreFloat3(&m_planetCb.data.planetViewPos, XMVector4Transform(XMVectorSetW(XMVectorZero(), 1.f), XMMatrixTranspose(XMLoadFloat4x4(&view))));
	m_planetCb.data.radius = m_sphere.getRadius();
	m_planetCb.update();

	m_renderConfig.setConstantBuffer(Shader::VS, LyreEngine::getViewCB(), 0);

	m_renderConfig.setConstantBuffer(Shader::HS, LyreEngine::getLodCB(), 0);

	m_renderConfig.setConstantBuffer(Shader::DS, m_planetCb, 0);
	m_renderConfig.setConstantBuffer(Shader::DS, LyreEngine::getLightingCB(), 1);
	m_renderConfig.setSampler(Shader::DS, LyreEngine::getSampler2D(), 0);
	m_renderConfig.setSRV(Shader::DS, m_iTerrainSRV, 0);

	m_renderConfig.setConstantBuffer(Shader::GS, LyreEngine::getViewCB(), 0);

	m_geometry.bind();
	m_renderConfig.bind();

	LyreEngine::getContext()->DrawIndexed(static_cast<UINT>(m_sphere.indices.size()), 0, 0);

	m_renderConfig.unbind();

	renderGeometry();
	renderNormals();
}

void Planet::renderGeometry() {
	m_geometryPipeline.geometry.bind();

	m_geometryPipeline.config.setConstantBuffer(Shader::VS, LyreEngine::getProjectionCB(), 0);
	m_geometryPipeline.config.bind();

	LyreEngine::getContext()->DrawAuto();
}

void Planet::renderNormals() {
	m_normalsPipeline.geometry.bind();

	m_normalsPipeline.config.setConstantBuffer(Shader::VS, LyreEngine::getProjectionCB(), 0);
	m_normalsPipeline.config.bind();

	LyreEngine::getContext()->DrawAuto();
}
