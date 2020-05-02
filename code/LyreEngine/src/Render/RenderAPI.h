#pragma once

#include "PipelineResources/VertexBuffer.h"
#include "PipelineResources/IndexBuffer.h"
#include "PipelineResources/InputLayout.h"
#include "PipelineResources/Shader.h"

namespace Lyre
{
	class CApplication;

	enum class ERenderAPIType
	{
		None = 0,
		DirectX_11
	};

	class CRenderAPI
	{
		friend class CRenderer;
	public:
		virtual ~CRenderAPI() = default;

		virtual bool Init(CApplication const& app) = 0;

		virtual void Clear(float color[4]) = 0;
		virtual void Present() = 0;

		virtual void DrawIndexed(Ref<CIndexBuffer> indexBuffer) = 0;

		virtual Ref<CVertexBuffer> CreateVertexBuffer(float* vertices, int size) = 0;
		virtual Ref<CIndexBuffer> CreateIndexBuffer(unsigned* indices, int size) = 0;
		virtual Ref<CInputLayout> CreateInputLayout(std::initializer_list<SLayoutEntry> layout) = 0;
		virtual Ref<CShader> CreateShader(std::string const& vsSrc, std::string const& psSrc) = 0;

		ERenderAPIType GetApiType() const { return m_apiType; }

	protected:
		CRenderAPI(ERenderAPIType apiType = ERenderAPIType::None);

	private:
		static Scope<CRenderAPI> Create(ERenderAPIType apiType);

	private:
		ERenderAPIType m_apiType;
	};

}
