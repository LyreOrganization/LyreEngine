#include "LyrePch.h"
#include "IndexBufferDX11.h"
#include "DirectX11API.h"

#undef interface

namespace Lyre
{

	D3D11_PRIMITIVE_TOPOLOGY toDxTopology(EDrawTopology topology)
	{
		switch (topology)
		{
		case Lyre::EDrawTopology::Triangles:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		default:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}
	}

	CIndexBufferDX11::CIndexBufferDX11(unsigned* indices, unsigned size, SDirectXInterface const* interface)
		: CIndexBuffer(size)
		, CPipelineResourceDX11(interface)
	{
		D3D11_BUFFER_DESC desc;
		{
			ZeroStruct(desc);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.ByteWidth = sizeof(unsigned) * size;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		}

		D3D11_SUBRESOURCE_DATA initData;
		ZeroStruct(initData);
		initData.pSysMem = indices;

		HRESULT hr = interface->device->CreateBuffer(&desc, &initData, &m_buffer);
		LYRE_ASSERT(SUCCEEDED(hr), "Failed to create index buffer.");
	}

	void CIndexBufferDX11::Bind(EDrawTopology topology)
	{
		GetDxInterface()->context->IASetIndexBuffer(m_buffer, DXGI_FORMAT_R32_UINT, 0);
		GetDxInterface()->context->IASetPrimitiveTopology(toDxTopology(topology));
	}

}
