#include "stdafx.h"

#include "KeyLayout.h"

#include "Actions.h"

using namespace std;

KeyLayout KeyLayout::loadFromFile(const char* file) {
	KeyLayout layout;
	ifstream input(file);

	WPARAM key;
	Action action;

	while (input.good()) {
		input >> key >> action;
		layout[key] = action;
	}

	return layout;
}
