#include "LyrePch.h"
#include "TextureDX11.h"

#undef interface

namespace Lyre
{

	CTextureDX11::CTextureDX11(SDirectXInterface const* interface)
		: CTexture()
		, CPipelineResourceDX11(interface)
	{}

}
