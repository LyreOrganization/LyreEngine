#include "LyrePch.h"
#include "VertexBufferDX11.h"
#include "DirectX11API.h"

#undef interface

namespace Lyre
{

	CVertexBufferDX11::CVertexBufferDX11(float* vertices, unsigned size, SDirectXInterface const* interface)
		: CPipelineResourceDX11(interface)
	{

		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(float) * size;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		}

		D3D11_SUBRESOURCE_DATA initData;
		ZeroStruct(initData);
		initData.pSysMem = vertices;

		HRESULT hr = GetDxInterface()->device->CreateBuffer(&desc, &initData, &m_buffer);
		LYRE_ASSERT(SUCCEEDED(hr), , "Failed to create vertex buffer.");
	}

	void CVertexBufferDX11::Bind()
	{
		UINT stride = m_layout->GetStride();
		UINT offset = 0;
		GetDxInterface()->context->IASetVertexBuffers(0, 1, &m_buffer.p, &stride, &offset);
	}

}
