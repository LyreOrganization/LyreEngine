#pragma once

#include "ControlsAction.h"
#include "ControlsActionGroup.h"
#include "ControlsKeyLayout.h"


namespace Controls {

	void setLayout(const KeyLayout&);

	void addActionGroup(const ActionGroup&);

	void process(DWORD ticksPerFrame);
	void press(WPARAM keyCode);
	void release(WPARAM keyCode);

};

