#pragma once

#include "Render/PipelineResources/Shader.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CShaderDX11 final : public CShader
	{
	public:
		CShaderDX11(std::string const& vsSrc, std::string const& psSrc, NotOwn<SDirectXInterface> interface);

		bool BindInputLayout(NotOwn<CVertexBuffer> vertexBuffer) override;
		void Bind() override;

	private:
		CComPtr<ID3DBlob> m_vsBlob;
		CComPtr<ID3DBlob> m_psBlob;

		CComPtr<ID3D11VertexShader> m_vs;
		CComPtr<ID3D11PixelShader> m_ps;
		
		NotOwn<SDirectXInterface> m_interface;
	};

}

#pragma pop_macro("interface")
