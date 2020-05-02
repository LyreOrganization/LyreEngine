#pragma once

#include "Render/PipelineResources/VertexBuffer.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CVertexBufferDX11 final : public CVertexBuffer
	{
		friend class CDirectX11API;
	public:
		CVertexBufferDX11(float* vertices, int size, NotOwn<SDirectXInterface> interface);

		void Bind() override;

	private:
		CComPtr<ID3D11Buffer> m_buffer;
		NotOwn<SDirectXInterface> m_interface;
	};

}

#pragma pop_macro("interface")

