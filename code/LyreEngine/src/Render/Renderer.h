#pragma once

#include "RenderAPI.h"

namespace Lyre
{

	class CRenderer final
	{
	public:

		static void CreateAPIIntance(ERenderAPIType apiType = ERenderAPIType::None);
		static NotOwn<CRenderAPI> GetAPI() { return s_RenderAPI.get(); }

		static void Submit(
			Ref<CVertexBuffer> vertexBuffer,
			Ref<CIndexBuffer> indexBuffer,
			Ref<CShader> shader
		);

		static void Present() { s_RenderAPI->Present(); }

	private:
		static Scope<CRenderAPI> s_RenderAPI;
	};

}
