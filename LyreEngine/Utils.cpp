#include "stdafx.h"
#include "Utils.h"
#include "LyreEngine.h"

ID3D11Buffer* Utils::createStreamOutputBuffer(UINT size) {
	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC bufferDesc;
	{
		ZeroStruct(bufferDesc);
		bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = size;
	}
	HRESULT hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer);
	if (FAILED(hr)) {
		throw std::runtime_error("Utils: Failed to create stream output buffer.");
	}

	return buffer;
}
