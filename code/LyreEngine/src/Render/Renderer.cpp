#include "LyrePch.h"
#include "Renderer.h"

namespace Lyre
{

	Scope<CRenderAPI> CRenderer::s_RenderAPI = nullptr;

	void CRenderer::CreateAPIIntance(ERenderAPIType apiType)
	{
		s_RenderAPI = CRenderAPI::Create(apiType);
	}

	void CRenderer::Submit(Ref<CVertexBuffer> vertexBuffer, Ref<CIndexBuffer> indexBuffer, Ref<CShader> shader)
	{
		bool succcess = shader->BindInputLayout(vertexBuffer.get());
		LYRE_ASSERT(succcess, "Input layout binding failed.");
		
		shader->Bind();
		vertexBuffer->Bind();
		indexBuffer->Bind(EDrawTopology::Triangles);

		s_RenderAPI->DrawIndexed(indexBuffer);
	}

}
