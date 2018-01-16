#pragma once

class D3DPipelineConfig {
	//Device
	ID3D11Device*					m_pDevice;
	ID3D11DeviceContext*			m_pContext;

	//Shaders
	CComPtr<ID3D11VertexShader>		m_iVS;
	CComPtr<ID3D11HullShader>		m_iHS;
	CComPtr<ID3D11DomainShader>		m_iDS;
	CComPtr<ID3D11GeometryShader>	m_iGS;
	CComPtr<ID3D11PixelShader>		m_iPS;


public:
	D3DPipelineConfig();

	void loadVertexShader(const WCHAR* fileName);
	void loadHullShader(const WCHAR* fileName);
	void loadDomainShader(const WCHAR* fileName);
	void loadGeometryShader(const WCHAR* fileName);
	void loadPixelShader(const WCHAR* fileName);
};
