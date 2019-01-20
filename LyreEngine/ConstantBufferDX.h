#pragma once

#include "stdafx.h"
#include "LyreEngine.h"

template<class BufferStruct>
class ConstantBufferDX final {

	//Device
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;

	//Data
	CComPtr<ID3D11Buffer> m_buffer;
public:
	BufferStruct data;

	ConstantBufferDX():
		m_pDevice(LyreEngine::getDevice()),
		m_pContext(LyreEngine::getContext()) 
	{
		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(BufferStruct);
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		HRESULT hr = LyreEngine::getDevice()->CreateBuffer(&desc, nullptr, &m_buffer);
		if (FAILED(hr)) {
			throw std::runtime_error("ConstantBufferDX: device->CreateBuffer() failed.");
		}
	}

	ID3D11Buffer* getBuffer() {
		return m_buffer;
	}

	void update() {
		m_pContext->UpdateSubresource(m_buffer, 0, nullptr, &data, 0, 0);
	}
};
