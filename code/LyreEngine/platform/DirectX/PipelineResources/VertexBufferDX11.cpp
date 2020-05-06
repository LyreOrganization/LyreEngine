#include "LyrePch.h"
#include "VertexBufferDX11.h"
#include "DirectX11API.h"

#undef interface

namespace Lyre
{

	CVertexBufferDX11::CVertexBufferDX11(float* vertices, int size, SDirectXInterface const* interface)
	{
		m_interface = interface;

		D3D11_BUFFER_DESC desc;
		{
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(float) * size;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
		}

		D3D11_SUBRESOURCE_DATA initData;
		{
			initData.pSysMem = vertices;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;
		}

		HRESULT hr = m_interface->device->CreateBuffer(&desc, &initData, &m_buffer);
		LYRE_ASSERT(SUCCEEDED(hr), , "Failed to create vertex buffer.");
	}

	void CVertexBufferDX11::Bind()
	{
		UINT stride = m_layout->GetStride();
		UINT offset = 0;
		m_interface->context->IASetVertexBuffers(0, 1, &m_buffer.p, &stride, &offset);
	}

}
