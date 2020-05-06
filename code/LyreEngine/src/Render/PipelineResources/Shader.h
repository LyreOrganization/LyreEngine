#pragma once

namespace Lyre
{

	class CVertexBuffer;
	class CConstantBuffer;

	class CShader
	{
	public:
		virtual ~CShader() = default;

		virtual bool BindInputLayout(CVertexBuffer* vertexBuffer) = 0;
		virtual void Bind() = 0;

		void AddConstantBuffer(std::shared_ptr<CConstantBuffer> const& constantBuffer) { m_constantBuffers.push_back(constantBuffer); }

	protected:
		CShader() = default;

	protected:
		std::vector<std::shared_ptr<CConstantBuffer>> m_constantBuffers;
	};

}

