#pragma once

#include "Render/PipelineResources/InputLayout.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CInputLayoutDX11 final : public CInputLayout
	{
	public:
		CInputLayoutDX11(std::initializer_list<SLayoutEntry> const& layout, NotOwn<SDirectXInterface> interface);

		void Bind() override;

		bool CreateDxResource(void const* shaderByteCode, size_t length);

	private:
		CComPtr<ID3D11InputLayout> m_layout;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_layoutDesc;

		NotOwn<SDirectXInterface> m_interface;
	};

}

#pragma pop_macro("interface")
