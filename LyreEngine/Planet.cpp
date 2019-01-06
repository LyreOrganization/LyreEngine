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

Planet::Planet(float radius, unsigned seed) : m_sphere(radius, seed) {}

HRESULT Planet::setupStreamOutputBuffers() {
	HRESULT hr = S_OK;

	ID3D11Buffer* buffer = UtilsDX::createStreamOutputBuffer((63 * 63 * 2 * 3) * (250) * sizeof(Geometry));
	m_renderConfig.setSOBuffer(buffer, 0);
	m_geometryPipeline.geometry.loadVertexBuffer(buffer, sizeof(Geometry), 0);

	buffer = UtilsDX::createStreamOutputBuffer((63 * 63 * 2 * 3) * (250) * sizeof(Normal) * 2);
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
	m_geometry.loadVertices(m_sphere.planes);
	m_geometry.loadIndices(m_sphere.indices);
	m_geometry.setTopology(D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST);

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

	size_t patchesAmount = m_sphere.indices.size() / 5;

	D3D11_TEXTURE2D_DESC texArrayDesc;
	{
		ZeroStruct(texArrayDesc);
		texArrayDesc.Width = texArrayDesc.Height = HEIGHTMAP_RESOLUTION;
		texArrayDesc.MipLevels = 1;
		texArrayDesc.ArraySize = static_cast<UINT>(patchesAmount);
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

	m_geometry.bind();
	m_renderConfig.bind();

	LyreEngine::getContext()->DrawIndexed(static_cast<UINT>(m_sphere.indices.size()), 0, 0);

	m_renderConfig.unbind();

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
