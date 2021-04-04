#pragma once

#include <Render/PipelineResources/InputLayout.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CInputLayoutDX11 final
		: public CInputLayout
		, private CPipelineResourceDX11
	{
	public:
		CInputLayoutDX11(std::initializer_list<SEntry> const& layout, SDirectXInterface const* interface);

		void Bind() override;

		bool CreateDxResource(void const* shaderByteCode, size_t length);

	private:
		CComPtr<ID3D11InputLayout> m_layout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_layoutDesc;
	};

}

#pragma pop_macro("interface")
