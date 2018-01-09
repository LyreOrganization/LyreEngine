#include "stdafx.h"
#include "KeyLayout.h"

KeyLayout KeyLayout::loadFromFile(const char* file) {
	KeyLayout layout;
	std::ifstream input(file);

	WPARAM key;
	Action action;
	for (
		;
		input;
		input >> key >> action
	) {
		layout[key] = action;
	}

	return layout;
}
