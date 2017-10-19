#pragma once
#include "stdafx.h"
#include <SFML/Window/Event.hpp>
#include "command.h"
#include <variant>

using in_t = std::variant<sf::Keyboard::Key, sf::Mouse::Button, int>;

namespace mark {
	class keymap {
	public:
		keymap(std::string filename);
		auto translate(sf::Event) const -> command;
	private:
		std::unordered_map<in_t, enum class command::type> m_map;
	};
}