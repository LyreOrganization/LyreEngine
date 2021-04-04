#pragma once

#include <LyreEngine.h>
#include <WindowsWnd.h>

class WinApp : public Lyre::CApplication
{
public:
	WinApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
	{
		Lyre::CWindowsWnd* wnd = static_cast<Lyre::CWindowsWnd*>(GetWindow());
		wnd->SetProperties(
			{
				L"Lyre Engine",
				1280, 720,
				100, 50
			}
		);
		wnd->Init(hInstance, nCmdShow);
	}

};

