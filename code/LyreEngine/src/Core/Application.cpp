#include "LyrePch.h"
#include "Application.h"
#include "Window.h"

Lyre::CApplication::CApplication()
	: m_running(true)
{
	m_window = CWindow::Create();
}

Lyre::CApplication::~CApplication()
{
	delete m_window;
}

void Lyre::CApplication::Run()
{
	while (m_running)
	{
		m_window->OnUpdate();
	}
}
