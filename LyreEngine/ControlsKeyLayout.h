#pragma once


namespace Controls {

	typedef std::unordered_map<WPARAM, std::string> KeyMapping;

	class KeyLayout final : public std::unordered_map<std::string, KeyMapping> {
	public:
		//static KeyLayout loadFromFile(const char*);
	};

}
