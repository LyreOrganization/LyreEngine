#pragma once

namespace Controls {

	class Action final {

		friend class ActionGroup;

		struct Handlers {
			std::function<void(DWORD)> onHold;
			std::function<void()> onPress;
			std::function<void()> onRelease;
		};

		std::string m_name;
		Handlers m_handlers;

	public:
		Action(const std::string& name);

		void on(std::function<void(DWORD ticksPerFrame)>);
		void onTriggered(std::function<void()>, bool onRelease = false);

		const std::string& getName() const;
		const Handlers& getHandlers() const;
	};

}
