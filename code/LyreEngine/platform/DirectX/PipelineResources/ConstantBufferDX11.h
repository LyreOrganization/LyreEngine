#pragma once

#include <Render/ShaderDataType.h>
#include <Render/PipelineResources/ConstantBuffer.h>
#include "PipelineResourceDX11.h"

#pragma push_macro("interface")
#undef interface

namespace Lyre
{
	struct SDirectXInterface;

	class CConstantBufferDX11 final
		: public CConstantBuffer
		, private CPipelineResourceDX11
	{
		friend class CShaderDX11;
	public:
		CConstantBufferDX11(std::initializer_list<SEntry> const& layout, SDirectXInterface const* interface);

		void UpdateConstant(unsigned position, void const* data) override;
		void Update() override;

	private:
		CComPtr<ID3D11Buffer> m_buffer;
		bool m_dirty;
	};

}

#pragma pop_macro("interface")
