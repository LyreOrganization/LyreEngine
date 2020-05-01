#pragma once

#include <string>

class EventListener;

namespace Lyre
{

	class CWindow
	{
	public:
		struct SProperties
		{
			std::wstring title;

			int width;
			int height;

			int pos_x;
			int pos_y;
		};
	public:
		CWindow(EventListener* listener = nullptr, SProperties props = SProperties{})
			: m_windowEventListener(listener)
			, m_props(props)
		{}
		virtual ~CWindow() = default;

		virtual void OnUpdate() = 0;
		void SetListener(EventListener* listener) { m_windowEventListener = listener; }
		void SetProperties(SProperties props) { m_props = props; }

		std::wstring GetTitle() const { return m_props.title; }
		int GetWidth() const { return m_props.width; }
		int GetHeight() const { return m_props.height; }

		static CWindow* Create(EventListener* listener = nullptr, SProperties props = SProperties{});
	
	protected:
		SProperties m_props;
		EventListener* m_windowEventListener;
	};

}
