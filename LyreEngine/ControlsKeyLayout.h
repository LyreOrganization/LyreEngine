#pragma once


namespace Controls {

	typedef std::unordered_map<WPARAM, std::string> KeyMapping;

	namespace {
		typedef std::unordered_map<std::string, KeyMapping> KeyLayoutBase;
	}
	
	class KeyLayout final : private KeyLayoutBase {
	public:
		KeyLayout() = default;
		KeyLayout(const char* fileName);

		using KeyLayoutBase::operator[];
	};

	
}
