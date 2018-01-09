#pragma once

#include <functional>
#include "Actions.h"
#include "KeyLayout.h"

namespace Keyboard {

	void setLayout(KeyLayout);

	void on(Action, std::function<void(DWORD tpf)>);
	void onTriggered(Action, std::function<void()>);

	void process();
	void press(WPARAM);
	void release(WPARAM);
};
