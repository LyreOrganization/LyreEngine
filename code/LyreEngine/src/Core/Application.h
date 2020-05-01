#pragma once

#include "Event.h"
#include "WindowLifeTimeEvents.h"

namespace Lyre
{
	class CWindow;

	class CApplication : public EventListener
	{
	public:
		CApplication();
		CApplication(CApplication const&) = delete;
		CApplication(CApplication &&) = delete;

		virtual ~CApplication();

		void Init();
		void Run();

		CWindow* GetWindow() { return m_window; }
		CWindow const* GetWindow() const { return m_window; }

		EVENT_MAP(CApplication)
		bool OnWindowClosed(CWindowClosedEvent const& event);

	private:
		bool m_running;
		CWindow* m_window;
	};

}
