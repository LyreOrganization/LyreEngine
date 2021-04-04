#pragma once

#include <Render/PipelineResources/VertexBuffer.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CVertexBufferDX11 final
		: public CVertexBuffer
		, private CPipelineResourceDX11
	{
		friend class CDirectX11API;
	public:
		CVertexBufferDX11(float* vertices, unsigned size, SDirectXInterface const* interface);

		void Bind() override;

	private:
		CComPtr<ID3D11Buffer> m_buffer;
	};

}

#pragma pop_macro("interface")

