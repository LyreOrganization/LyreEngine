#pragma once

namespace Lyre
{
	class CWindow;

	class CApplication
	{
	public:
		CApplication();
		CApplication(CApplication const&) = delete;
		CApplication(CApplication &&) = delete;

		virtual ~CApplication();

		void Run();

		CWindow* GetWindow() const { return m_window; }

	private:
		bool m_running;
		CWindow* m_window;
	};

}
