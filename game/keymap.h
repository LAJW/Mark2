#pragma once
#include <string>
#include <unordered_map>
#include <SFML/Window/Event.hpp>
#include "command.h"

namespace std {
	template<>
	struct hash<std::pair<uint8_t, uint8_t>> {
		size_t operator()(const std::pair<uint8_t, uint8_t>& pair) const {
			return std::hash<uint16_t>()((pair.first * 256) + pair.second);
		}
	};
}

namespace mark {
	class keymap {
	public:
		keymap(std::string filename);
		auto translate(sf::Event) const -> mark::command;
	private:
		std::unordered_map<std::pair<uint8_t, uint8_t>, enum class mark::command::type> m_map;
	};
}