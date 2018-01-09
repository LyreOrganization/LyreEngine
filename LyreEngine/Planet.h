#pragma once

#include "SpherifiedCube.h"

#define MAX_CBUFFERS_AMOUNT D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT

class Planet final
{
private:
	CComPtr<ID3D11VertexShader>			m_iVS = nullptr;
	CComPtr<ID3D11HullShader>			m_iHS = nullptr;
	CComPtr<ID3D11DomainShader>			m_iDS = nullptr;
	CComPtr<ID3D11PixelShader>			m_iPS = nullptr;

	CComPtr<ID3D11InputLayout>			m_iVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				m_iVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				m_iIndexBuffer = nullptr;

	std::vector<SpherifiedCube::Vertex> m_vertices;
	std::vector<DWORD>					m_indices;

	std::array<ID3D11Buffer*, MAX_CBUFFERS_AMOUNT> m_cbuffers;

public:
	HRESULT init();
	void render();
};
