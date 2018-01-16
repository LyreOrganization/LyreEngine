#pragma once

#include "stdafx.h"
#include "LyreEngine.h"

template<class T>
class D3DConstantBuffer {
	T m_bufferStruct;

	CComPtr<ID3D11Buffer> m_buffer;
public:

	D3DConstantBuffer() {
		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(T);
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		HRESULT hr = LyreEngine::getDevice()->CreateBuffer(&desc, nullptr, &m_buffer);
		if (FAILED(hr)) {
			throw std::runtime_error("D3DConstantBuffer: device->CreateBuffer() failed.");
		}
	}
};

