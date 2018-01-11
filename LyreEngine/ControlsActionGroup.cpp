#include "stdafx.h"

#include "ControlsActionGroup.h"
#include "ControlsAction.h"

using namespace std;
using namespace Controls;

ActionGroup::ActionGroup(const string& name):
	m_name(name)
{}

const std::string& ActionGroup::getName() const {
	return m_name;
}

Action& ActionGroup::action(const string & name) {
	return this->emplace_back(name);
}
