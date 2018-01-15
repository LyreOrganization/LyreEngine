#pragma once

#include "SpherifiedCube.h"

#define MAX_CBUFFERS_AMOUNT D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT
#define MAX_SAMPLERS_AMOUNT D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT
#define MAX_SRVS_AMOUNT D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT
#define MAX_SOBUFFERS_AMOUNT D3D11_SO_BUFFER_SLOT_COUNT

class Planet final {
private:
	CComPtr<ID3D11VertexShader>			m_iVS = nullptr;
	CComPtr<ID3D11HullShader>			m_iHS = nullptr;
	CComPtr<ID3D11DomainShader>			m_iDS = nullptr;
	CComPtr<ID3D11GeometryShader>		m_iGS = nullptr;

	struct {
		CComPtr<ID3D11VertexShader>		iVS = nullptr;
		CComPtr<ID3D11InputLayout>		iVertexLayout = nullptr;
		CComPtr<ID3D11Buffer>			iVertexBuffer = nullptr;
		CComPtr<ID3D11PixelShader>		iPS = nullptr;
	} m_geometryPipeline;

	struct {
		CComPtr<ID3D11VertexShader>		iVS = nullptr;
		CComPtr<ID3D11InputLayout>		iVertexLayout = nullptr;
		CComPtr<ID3D11Buffer>			iVertexBuffer = nullptr;
		CComPtr<ID3D11PixelShader>		iPS = nullptr;
	} m_normalsPipeline;

	CComPtr<ID3D11InputLayout>			m_iVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				m_iVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				m_iIndexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				m_iPlanetConstantBuffer = nullptr;

	CComPtr<ID3D11ShaderResourceView>	m_iTerrainSRV = nullptr;

	CComPtr<ID3D11Buffer>				m_iGeometryBuffer = nullptr;
	CComPtr<ID3D11Buffer>				m_iNormalsBuffer = nullptr;

	HRESULT setupStreamOutputBuffers();
	HRESULT initGeometryShader();

	HRESULT initGeometryPipeline();
	HRESULT initNormalsPipeline();

	SpherifiedCube						m_sphere;

	std::array<ID3D11Buffer*, MAX_CBUFFERS_AMOUNT> m_cBuffers;
	std::array<ID3D11SamplerState*, MAX_SAMPLERS_AMOUNT> m_samplers;
	std::array<ID3D11ShaderResourceView*, MAX_SRVS_AMOUNT> m_srvs;
	std::pair<
		std::array<ID3D11Buffer*, MAX_SOBUFFERS_AMOUNT>,
		std::array<UINT, MAX_SOBUFFERS_AMOUNT>
		> m_soBuffers;

public:
	Planet(float radius);
	HRESULT init();
	void render();
	void renderGeometry();
	void renderNormals();
};
