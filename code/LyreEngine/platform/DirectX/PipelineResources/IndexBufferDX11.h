#pragma once

#include "Render/PipelineResources/IndexBuffer.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CIndexBufferDX11 final : public CIndexBuffer
	{
		friend class CDirectX11API;
	public:
		CIndexBufferDX11(unsigned* indices, int size, NotOwn<SDirectXInterface> interface);

		void Bind(EDrawTopology topology) override;

	private:
		CComPtr<ID3D11Buffer> m_buffer;
		NotOwn<SDirectXInterface> m_interface;
	};

}

#pragma pop_macro("interface")
