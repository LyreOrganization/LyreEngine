#include "LyrePch.h"
#include "Renderer.h"

namespace Lyre
{

	std::unique_ptr<CRenderAPI> CRenderer::s_RenderAPI = nullptr;

	void CRenderer::CreateAPIIntance(ERenderAPIType apiType)
	{
		s_RenderAPI = CRenderAPI::Create(apiType);
	}

	void CRenderer::Submit(std::shared_ptr<CVertexBuffer> vertexBuffer, std::shared_ptr<CIndexBuffer> indexBuffer, std::shared_ptr<CShader> shader)
	{
		bool succcess = shader->BindInputLayout(vertexBuffer.get());
		LYRE_ASSERT(succcess, "Input layout binding failed.");
		
		shader->Bind();
		vertexBuffer->Bind();
		indexBuffer->Bind(EDrawTopology::Triangles);

		s_RenderAPI->DrawIndexed(indexBuffer);
	}

}
