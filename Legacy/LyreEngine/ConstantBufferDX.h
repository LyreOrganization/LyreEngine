#pragma once

#include "stdafx.h"
#include "DeviceReference.h"

template<class BufferStruct>
class ConstantBufferDX : public DeviceReference {
public:
	ConstantBufferDX();

	void update();
	operator ID3D11Buffer*();
	ID3D11Buffer* getBuffer();

	BufferStruct data;

private:
	CComPtr<ID3D11Buffer> m_buffer;
};


// Implementation

template<class BufferStruct>
ConstantBufferDX<BufferStruct>::ConstantBufferDX() {
	D3D11_BUFFER_DESC desc;
	{
		ZeroStruct(desc);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(BufferStruct);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	}
	HRESULT hr = m_pDevice->CreateBuffer(&desc, nullptr, &m_buffer);
	if (FAILED(hr)) {
		throw std::runtime_error("ConstantBufferDX: device->CreateBuffer() failed.");
	}
}

template<class BufferStruct>
void ConstantBufferDX<BufferStruct>::update() {
	m_pContext->UpdateSubresource(m_buffer, 0, nullptr, &data, 0, 0);
}

template<class BufferStruct>
ConstantBufferDX<BufferStruct>::operator ID3D11Buffer*() {
	return m_buffer;
}

template<class BufferStruct>
ID3D11Buffer* ConstantBufferDX<BufferStruct>::getBuffer() {
	return m_buffer;
}
