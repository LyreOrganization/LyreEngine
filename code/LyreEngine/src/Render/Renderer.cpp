#include "LyrePch.h"
#include "Renderer.h"

namespace Lyre
{

	Scope<CRenderAPI> CRenderer::s_RenderAPI = nullptr;

	void CRenderer::CreateAPIIntance(ERenderAPIType apiType)
	{
		s_RenderAPI = CRenderAPI::Create(apiType);
	}

}
