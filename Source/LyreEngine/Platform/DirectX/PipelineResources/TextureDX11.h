#pragma once

#include <Render/PipelineResources/Texture.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	class CTextureDX11 final
		: public CTexture
		, private CPipelineResourceDX11
	{
		friend class CDirectX11API;
		friend class CShaderDX11;
	public:
		CTextureDX11(SDirectXInterface const* interface);

	private:
		CComPtr<ID3D11Texture2D> m_texture;
		CComPtr<ID3D11ShaderResourceView> m_view;
	};

}

#pragma pop_macro("interface")
