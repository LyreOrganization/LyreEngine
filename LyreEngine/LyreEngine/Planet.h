#pragma once

class Planet
{
private:
	CComPtr<ID3D11VertexShader>			m_iVS = nullptr;
	CComPtr<ID3D11PixelShader>			m_iPS = nullptr;

	CComPtr<ID3D11InputLayout>			m_iVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				m_iVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				m_iIndexBuffer = nullptr;

	std::vector<DirectX::XMFLOAT3>		m_vertices;
	std::vector<DWORD>					m_indices;

public:
	HRESULT init();
	void render();
};
