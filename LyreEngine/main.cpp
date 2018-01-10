#include "stdafx.h"

#include "LyreEngine.h"
#include "FreeCamera.h"
#include "Keyboard.h"
#include "Actions.h"
#include "KeyLayout.h"

using namespace std;
using namespace DirectX;

void setupKeyboardLayout();

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	case WM_KEYDOWN:
		Keyboard::press(wParam);
		break;
	case WM_KEYUP:
		Keyboard::release(wParam);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE m_hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(LyreEngine::initWindow(m_hInstance, nCmdShow, MsgProc))) {
		MessageBox(0, L"Window init failed!", 0, 0);
		return EXIT_FAILURE;
	}
	try {
		LyreEngine::getDevice();
	}
	catch (std::runtime_error e) {
		wchar_t msg[100];
		size_t msgLen;
		//Convert c-style string to ebuchiy winapishnyy LPCWSTR
		mbstowcs_s(&msgLen, msg, e.what(), strlen(e.what()));

		MessageBox(0, msg, 0, 0);
		return EXIT_FAILURE;
	}

	setupKeyboardLayout();
	Keyboard::onTriggered(Action::Exit, []() { exit(0); }, true);

	DWORD previousTime = GetTickCount();
	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			DWORD ticksPerFrame = (GetTickCount() - previousTime);
			previousTime = GetTickCount();

			UINT width, height;
			LyreEngine::GetClientWH(width, height);

			POINT cursor;
			GetCursorPos(&cursor);
			LyreEngine::getCamera()->tilt((cursor.y - static_cast<LONG>(height / 2)) / 1000.f);
			LyreEngine::getCamera()->pan((static_cast<LONG>(width / 2) - cursor.x) / 1000.f);
			SetCursorPos(width / 2, height / 2);

			Keyboard::process(ticksPerFrame);
			LyreEngine::render(ticksPerFrame);
		}
	}
	return static_cast<int>(msg.wParam);
}

void setupKeyboardLayout() {
	KeyLayout keys;

	keys[WindowsLetterIdx('Q')] = Action::Camera_RollCCW;
	keys[WindowsLetterIdx('E')] = Action::Camera_RollCW;

	keys[WindowsLetterIdx('W')] = Action::Camera_MoveForward;
	keys[WindowsLetterIdx('A')] = Action::Camera_MoveLeft;
	keys[WindowsLetterIdx('S')] = Action::Camera_MoveBackward;
	keys[WindowsLetterIdx('D')] = Action::Camera_MoveRight;

	keys[VK_ESCAPE] = Action::Exit;

	Keyboard::setLayout(keys);
}
