#include "stdafx.h"

#include "ControlsAction.h"

using namespace Controls;

Action::Action(const std::string& name) :
	m_name(name)
{}

void Action::on(std::function<void(DWORD)> callback) {
	m_handlers.onHold = callback;
}

void Action::onTriggered(std::function<void()> callback, bool onRelease) {
	if (onRelease) {
		m_handlers.onRelease = callback;
	}
	else {
		m_handlers.onPress = callback;
	}
}

const std::string& Action::getName() const {
	return m_name;
}

const Action::Handlers& Action::getHandlers() const {
	return m_handlers;
}
