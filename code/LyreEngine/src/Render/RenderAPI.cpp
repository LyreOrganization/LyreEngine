#include "LyrePch.h"
#include "RenderAPI.h"

#ifdef LYRE_PLATFORM_WINDOWS
#include "DirectX11API.h"
#endif

namespace Lyre
{

	Scope<CRenderAPI> CRenderAPI::Create(ERenderAPIType apiType)
	{
		switch (apiType)
		{
		case Lyre::ERenderAPIType::None:
			LYRE_ASSERT(false, "Invalid API type provided.");
			return Scope<CRenderAPI>(nullptr);
#ifdef LYRE_PLATFORM_WINDOWS
		case Lyre::ERenderAPIType::DirectX_11:
			return MakeScope<CDirectX11API>();
#endif
		default:
			LYRE_ASSERT(false, "Current API type is not supported.");
			return Scope<CRenderAPI>(nullptr);
		}
	}

	CRenderAPI::CRenderAPI(ERenderAPIType apiType)
		: m_apiType(apiType)
	{}

}
