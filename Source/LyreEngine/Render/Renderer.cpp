#include "LyrePch.h"
#include "Renderer.h"
#include "Mesh.h"

namespace Lyre
{

	std::unique_ptr<CRenderAPI> CRenderer::s_RenderAPI = nullptr;

	void CRenderer::CreateAPIIntance(ERenderAPIType apiType)
	{
		s_RenderAPI = CRenderAPI::Create(apiType);
	}

	void CRenderer::Submit(std::shared_ptr<CMesh> mesh)
	{
		bool succcess = mesh->m_shader->BindInputLayout(mesh->m_vertexBuffer.get());
		LYRE_ASSERT(succcess, "Input layout binding failed.");
		
		mesh->m_shader->Bind();
		mesh->m_vertexBuffer->Bind();
		mesh->m_indexBuffer->Bind(EDrawTopology::Triangles);

		s_RenderAPI->DrawIndexed(mesh->m_indexBuffer);
	}

}
