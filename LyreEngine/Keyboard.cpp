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

	struct triggerActionDesc {
		std::function<void()> handler;
		bool onRelease;
	};

	unordered_map<Action, triggerActionDesc> g_triggerActions;
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

void Keyboard::onTriggered(Action action, std::function<void()> callback, bool onRelease) {
	g_triggerActions[action] = { callback, onRelease };
}

void Keyboard::process(DWORD ticksPerFrame) {
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

void handleTriggeredAction(Action action) {
	triggerActionDesc& trigger = g_triggerActions.at(action);
	if (trigger.onRelease) {
		trigger.handler();
	}
}

void Keyboard::press(WPARAM keyCode) {
	try {
		KeyDesc& key = g_keys.at(keyCode);
		key.pressed = true;

		triggerActionDesc& trigger = g_triggerActions.at(key.action);
		if (!trigger.onRelease) {
			trigger.handler();
		}
	}
	catch (std::out_of_range) {
		return;
	}
}

void Keyboard::release(WPARAM keyCode) {
	try {
		KeyDesc& key = g_keys.at(keyCode);
		key.pressed = false;

		triggerActionDesc& trigger = g_triggerActions.at(key.action);
		if (trigger.onRelease) {
			trigger.handler();
		}
	}
	catch (std::out_of_range) {
		return;
	}
}
