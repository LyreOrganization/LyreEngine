#pragma once

class KeyLayout;
enum class Action;

namespace Keyboard {

	void setLayout(KeyLayout);

	void on(Action, std::function<void(DWORD ticksPerFrame)>);
	void onTriggered(Action, std::function<void()>);

	void process();
	void press(WPARAM);
	void release(WPARAM);
};
