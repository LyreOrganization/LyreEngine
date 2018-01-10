#include "stdafx.h"

#include "Keyboard.h"

#include "KeyLayout.h"
#include "Actions.h"

using namespace std;
using namespace DirectX;

namespace {

	struct KeyDesc {
		Action action;
		bool pressed;
	};

	unordered_map<Action, std::function<void()>> g_triggeringActions;
	unordered_map<Action, std::function<void(DWORD)>> g_continuousActions;
	unordered_map<WPARAM, KeyDesc> g_keys;

}

void Keyboard::setLayout(KeyLayout layout) {
	g_keys.clear();
	for (auto& kv : layout) {
		g_keys[kv.first] = { kv.second, false };
	}
}

void Keyboard::on(Action action, std::function<void(DWORD)> callback) {
	g_continuousActions[action] = callback;
}

void Keyboard::onTriggered(Action action, std::function<void()> callback) {
	g_triggeringActions[action] = callback;
}

void Keyboard::process() {
	static DWORD s_previousTime = GetTickCount();
	DWORD ticksPerFrame = (GetTickCount() - s_previousTime);
	s_previousTime = GetTickCount();

	for (auto& pair : g_keys) {
		KeyDesc key = pair.second;
		if (key.pressed) {
			try {
				g_continuousActions.at(key.action)(ticksPerFrame);
			}
			catch (std::out_of_range) {
				continue;
			}
		}
	}
}

void Keyboard::press(WPARAM key) {
	try {
		KeyDesc& kd = g_keys.at(key);
		kd.pressed = true;
		g_triggeringActions.at(kd.action)();
	}
	catch (std::out_of_range) {
		return;
	}
}

void Keyboard::release(WPARAM key) {
	try {
		g_keys.at(key).pressed = false;
	}
	catch (std::out_of_range) {
		return;
	}
}
