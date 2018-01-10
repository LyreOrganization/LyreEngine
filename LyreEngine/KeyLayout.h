#pragma once

enum class Action;

class KeyLayout final : public std::unordered_map<WPARAM, Action> {
public:
	static KeyLayout loadFromFile(const char*);
};
