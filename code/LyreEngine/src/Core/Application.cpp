#include "LyrePch.h"
#include "Application.h"
#include "Window.h"

EVENT_MAP_BEGIN(Lyre::CApplication)
	ADD_LISTENER(Lyre::CWindowClosedEvent, OnWindowClosed)
EVENT_MAP_END()

Lyre::CApplication::CApplication()
	: m_running(true)
{
	m_window = CWindow::Create(this);
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

bool Lyre::CApplication::OnWindowClosed(CWindowClosedEvent const & event)
{
	m_running = false;
	return true;
}
