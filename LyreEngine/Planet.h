#pragma once

#include "SpherifiedCube.h"
#include "GeometryDX.h"
#include "ConstantBufferDX.h"
#include "PipelineConfigDX.h"
#include "LodAdapter.h"

#define MAX_CBUFFERS_AMOUNT D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT
#define MAX_SAMPLERS_AMOUNT D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
#define MAX_SRVS_AMOUNT D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT
#define MAX_SOBUFFERS_AMOUNT D3D11_SO_BUFFER_SLOT_COUNT

class Planet final {
private:
	//////////// Pipelines ////////////

	struct {
		GeometryDX						geometry;
		PipelineConfigDX				config;
	} m_geometryPipeline;

	struct {
		GeometryDX						geometry;
		PipelineConfigDX				config;
	} m_normalsPipeline;

	GeometryDX							m_geometry;
	PipelineConfigDX					m_renderConfig;


	//////////// ConstantBuffers ////////////

	struct CubeFacesCB {
		DirectX::XMFLOAT4X4 planeRotations[6];
	};
	ConstantBufferDX<CubeFacesCB>			m_cubeFacesCb;

	struct PlanetCB {
		DirectX::XMFLOAT3 planetPos;
		float radius;
	};
	ConstantBufferDX<PlanetCB>			m_planetCb;


	CComPtr<ID3D11ShaderResourceView>	m_iSlerpLookupSRV = nullptr;

	CComPtr<ID3D11Texture2D>			m_iTerrainTexArray = nullptr;
	CComPtr<ID3D11ShaderResourceView>	m_iTerrainSRV = nullptr;

	CComPtr<ID3D11UnorderedAccessView>	m_iLodDiffUAV = nullptr;

	HRESULT setupStreamOutputBuffers();
	HRESULT initGeometryShader();

	HRESULT initGeometryPipeline();
	HRESULT initNormalsPipeline();

	HRESULT initGeometryAndVS();

	SpherifiedCube						m_sphere;
	LodAdapter							m_lodAdapter;

public:
	Planet(float radius, unsigned seed);
	HRESULT init();
	void render();
	void renderGeometry();
	void renderNormals();
};
