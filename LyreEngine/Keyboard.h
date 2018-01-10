#pragma once

class KeyLayout;
enum class Action;

namespace Keyboard {

	void setLayout(KeyLayout);

	void on(Action, std::function<void(DWORD ticksPerFrame)>);
	void onTriggered(Action, std::function<void()>, bool onRelease = false);

	void process(DWORD ticksPerFrame);
	void press(WPARAM keyCode);
	void release(WPARAM keyCode);
};
