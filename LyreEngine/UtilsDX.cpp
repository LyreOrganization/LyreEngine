#include "stdafx.h"
#include "UtilsDX.h"
#include "LyreEngine.h"

ID3D11Buffer* UtilsDX::createStreamOutputBuffer(UINT size, bool drawAuto) {
	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC bufferDesc;
	{
		ZeroStruct(bufferDesc);
		bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | 
			(drawAuto ? D3D11_BIND_VERTEX_BUFFER : 0);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = size;
	}
	HRESULT hr = LyreEngine::getDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer);
	if (FAILED(hr)) {
		throw std::runtime_error("UtilsDX: Failed to create stream output buffer.");
	}

	return buffer;
}
