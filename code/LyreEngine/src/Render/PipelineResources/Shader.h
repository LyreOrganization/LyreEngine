#pragma once

namespace Lyre
{

	class CVertexBuffer;

	class CShader
	{
	public:
		virtual ~CShader() = default;

		virtual bool BindInputLayout(NotOwn<CVertexBuffer> vertexBuffer) = 0;
		virtual void Bind() = 0;

	protected:
		CShader() = default;
	};

}

