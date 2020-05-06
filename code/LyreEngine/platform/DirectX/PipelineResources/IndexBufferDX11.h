#pragma once

#include <Render/PipelineResources/IndexBuffer.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CIndexBufferDX11 final
		: public CIndexBuffer
		, private CPipelineResourceDX11
	{
		friend class CDirectX11API;
	public:
		CIndexBufferDX11(unsigned* indices, unsigned size, SDirectXInterface const* interface);

		void Bind(EDrawTopology topology) override;

	private:
		CComPtr<ID3D11Buffer> m_buffer;
	};

}

#pragma pop_macro("interface")
