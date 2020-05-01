#pragma once

#include "Core/Window.h"

namespace Lyre
{

	class CWindowsWnd final : public CWindow
	{
	public:
		using CWindow::CWindow;

		bool Init(HINSTANCE hInst, int nCmdShow);
		void OnUpdate() override;

		HWND getHandle() const { return m_hWindow; }

	private:
		HINSTANCE m_hInst;
		HWND m_hWindow;
	};

}
