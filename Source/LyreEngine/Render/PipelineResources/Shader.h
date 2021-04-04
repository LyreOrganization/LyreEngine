#pragma once

namespace Lyre
{

	class CVertexBuffer;
	class CConstantBuffer;
	class CTexture;

	class CShader
	{
	public:
		virtual ~CShader() = default;

		virtual bool BindInputLayout(CVertexBuffer* vertexBuffer) = 0;
		virtual void Bind() = 0;

		void AddConstantBuffer(std::shared_ptr<CConstantBuffer> const& constantBuffer) { m_constantBuffers.push_back(constantBuffer); }
		void AddTexture(std::shared_ptr<CTexture> const& texture) { m_textures.push_back(texture); }

	protected:
		CShader() = default;

	protected:
		std::vector<std::shared_ptr<CConstantBuffer>> m_constantBuffers;
		std::vector<std::shared_ptr<CTexture>> m_textures;
	};

}

