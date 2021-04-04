#pragma once

#include <Render/PipelineResources/Shader.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{

	struct SDirectXInterface;

	class CShaderDX11 final 
		: public CShader
		, private CPipelineResourceDX11
	{
	public:
		CShaderDX11(std::string const& vsSrc, std::string const& psSrc, SDirectXInterface const* interface);

		bool BindInputLayout(CVertexBuffer* vertexBuffer) override;
		void Bind() override;

	private:
		CComPtr<ID3DBlob> m_vsBlob;
		CComPtr<ID3DBlob> m_psBlob;

		CComPtr<ID3D11VertexShader> m_vs;
		CComPtr<ID3D11PixelShader> m_ps;

		CComPtr<ID3D11SamplerState> m_defaultSampler;
	};

}

#pragma pop_macro("interface")
