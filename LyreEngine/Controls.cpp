#include "stdafx.h"

#include "Controls.h"

using namespace std;

namespace {

	struct KeyDesc {
		const Controls::Action* action;
		bool pressed;
	};

	vector<Controls::ActionGroup> g_actionGroups;
	vector<unordered_map<WPARAM, KeyDesc>> g_activeActionGroups;

	Controls::KeyLayout g_layout;

}

void createKeyActionMap(const Controls::ActionGroup& group) {
	try {
		auto keyActionNameMap = g_layout.at(group.getName());

		unordered_map<WPARAM, KeyDesc> keyActionMap;
		for (auto& keyAction : keyActionNameMap) {
			auto action = find_if(group.begin(), group.end(), [&](Controls::Action action) {
				return action.getName() == keyAction.second;
			});
			if (action != group.end()) {
				keyActionMap[keyAction.first] = { &(*action), false };
			}
		}
		if (!keyActionMap.empty()) {
			g_activeActionGroups.push_back(move(keyActionMap));
		}
	}
	catch (out_of_range) {
		return;
	}
}

void Controls::setLayout(const KeyLayout& layout) {
	g_layout = layout;

	for (auto& group : g_actionGroups) {
		createKeyActionMap(group);
	}
}

void Controls::addActionGroup(const ActionGroup& group) {
	g_actionGroups.push_back(group);
	createKeyActionMap(g_actionGroups.back());
}

void Controls::process(DWORD ticksPerFrame) {
	for (auto& group : g_activeActionGroups) {
		for (auto& value : group) {
			KeyDesc key = value.second;
			if (key.pressed) {
				try {
					key.action->getHandlers().onHold(ticksPerFrame);
				}
				catch (bad_function_call) {
					continue;
				}
			}
		}
	}
}

void Controls::press(WPARAM keyCode) {
	for (auto& group : g_activeActionGroups) {
		try {
			KeyDesc& key = group.at(keyCode);
			key.pressed = true;
			key.action->getHandlers().onPress();
		}
		catch (exception) {
			continue;
		}
	}
}

void Controls::release(WPARAM keyCode) {
	for (auto& group : g_activeActionGroups) {
		try {
			KeyDesc& key = group.at(keyCode);
			key.pressed = false;
			key.action->getHandlers().onRelease();
		}
		catch (exception) {
			continue;
		}
	}
}
