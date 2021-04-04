#pragma once

#pragma push_macro("interface")
#undef interface

namespace Lyre
{
	struct SDirectXInterface;

	class CPipelineResourceDX11
	{
	public:
		CPipelineResourceDX11(SDirectXInterface const* interface)
			: m_interface(interface)
		{}

		inline SDirectXInterface const* GetDxInterface() const { return m_interface; }

	private:
		SDirectXInterface const* m_interface;
	};

}

#pragma pop_macro("interface")

