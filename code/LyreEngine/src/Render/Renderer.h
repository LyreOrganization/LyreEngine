#pragma once

#include "RenderAPI.h"

namespace Lyre
{

	class CMesh;
	class CCamera;

	class CRenderer final
	{
	public:

		static void CreateAPIIntance(ERenderAPIType apiType = ERenderAPIType::None);
		static CRenderAPI* GetAPI() { return s_RenderAPI.get(); }

		static void Submit(std::shared_ptr<CMesh> mesh);

		static void Present() { s_RenderAPI->Present(); }

	private:
		static std::unique_ptr<CRenderAPI> s_RenderAPI;
	};

}
