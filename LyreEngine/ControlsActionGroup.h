#pragma once

namespace Controls {

	class Action;
	class KeyLayout;

	class ActionGroup final :
		private std::vector<Action> {

		std::string m_name;

	public:
		ActionGroup(const std::string& name);

		const std::string& getName() const;
		Action& action(const std::string& name);

		using std::vector<Action>::begin;
		using std::vector<Action>::end;
	};

}
