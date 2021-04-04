#pragma once

#include "ShaderDataType.h"
#include "PipelineResources/VertexBuffer.h"
#include "PipelineResources/IndexBuffer.h"
#include "PipelineResources/InputLayout.h"
#include "PipelineResources/ConstantBuffer.h"
#include "PipelineResources/Shader.h"
#include "PipelineResources/Texture.h"

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

		virtual void DrawIndexed(std::shared_ptr<CIndexBuffer> indexBuffer) = 0;

		virtual std::shared_ptr<CVertexBuffer> CreateVertexBuffer(float* vertices, unsigned size) = 0;
		virtual std::shared_ptr<CIndexBuffer> CreateIndexBuffer(unsigned* indices, unsigned size) = 0;
		virtual std::shared_ptr<CInputLayout> CreateInputLayout(std::initializer_list<CInputLayout::SEntry> layout) = 0;
		
		virtual std::shared_ptr<CConstantBuffer> CreateConstantBuffer(std::initializer_list<CConstantBuffer::SEntry> layout) = 0;
		
		virtual std::shared_ptr<CShader> CreateShaderFromSources(std::string const& vsSrc, std::string const& psSrc) = 0;
		virtual std::shared_ptr<CShader> CreateShaderFromFiles(std::string const& vsFile, std::string const& psFile) = 0;
		
		virtual std::shared_ptr<CTexture> CreateTextureFromFile(std::string const& filename) = 0;

		ERenderAPIType GetApiType() const { return m_apiType; }

	protected:
		CRenderAPI(ERenderAPIType apiType = ERenderAPIType::None);

	private:
		static std::unique_ptr<CRenderAPI> Create(ERenderAPIType apiType);

	private:
		ERenderAPIType m_apiType;
	};

}
