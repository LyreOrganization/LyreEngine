#pragma once

#include <unordered_map>
#include "Actions.h"


class KeyLayout: public std::unordered_map<WPARAM, Action> {
public:

	static KeyLayout loadFromFile(const char*);
};
