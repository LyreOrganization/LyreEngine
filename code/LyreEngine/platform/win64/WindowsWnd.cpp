#include "LyrePch.h"
#include "WindowsWnd.h"

#include "resource.h"

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

namespace Lyre
{

	CWindow* CWindow::Create(EventListener* listener, SProperties props)
	{
		return new CWindowsWnd(listener, props);
	}

	bool CWindowsWnd::Init(HINSTANCE hInst, int nCmdShow)
	{
		m_hInst = hInst;

		WNDCLASSEX wcex;
		{
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = MsgProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = hInst;
			wcex.hIcon = LoadIcon(hInst, reinterpret_cast<LPCTSTR>(IDI_ICON));
			wcex.hCursor = LoadCursor(nullptr, reinterpret_cast<LPCTSTR>IDC_ARROW);
			wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
			wcex.lpszMenuName = nullptr;
			wcex.lpszClassName = m_props.title.c_str();
			wcex.hIconSm = LoadIcon(wcex.hInstance, reinterpret_cast<LPCTSTR>(IDI_ICON));
		}
		if (!RegisterClassEx(&wcex))
		{
			return false;
		}

		RECT rc = { 0, 0, m_props.width, m_props.height };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		m_hWindow = CreateWindow(
			m_props.title.c_str(),
			m_props.title.c_str(),
			WS_OVERLAPPEDWINDOW,
			m_props.pos_x, m_props.pos_y,
			rc.right - rc.left, rc.bottom - rc.top,
			nullptr, nullptr, hInst, nullptr);

		if (!m_hWindow)
		{
			return false;
		}
		ShowCursor(TRUE);
		SetCursorPos(m_props.width / 2, m_props.height / 2);
		ShowWindow(m_hWindow, nCmdShow);

		return true;
	}

	void CWindowsWnd::OnUpdate()
	{
		MSG msg = { 0 };
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

}
