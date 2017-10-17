#include "stdafx.h"
#include "keymap.h"
#include "yaml-cpp\yaml.h"
#include <variant>

namespace {
	const std::unordered_map<std::string, in_t> key_dict {
		{ "a", in_t{ sf::Keyboard::A } },
		{ "b", in_t{ sf::Keyboard::B } },
		{ "c", in_t{ sf::Keyboard::C } },
		{ "d", in_t{ sf::Keyboard::D } },
		{ "e", in_t{ sf::Keyboard::E } },
		{ "f", in_t{ sf::Keyboard::F } },
		{ "g", in_t{ sf::Keyboard::G } },
		{ "h", in_t{ sf::Keyboard::H } },
		{ "i", in_t{ sf::Keyboard::I } },
		{ "j", in_t{ sf::Keyboard::J } },
		{ "k", in_t{ sf::Keyboard::K } },
		{ "l", in_t{ sf::Keyboard::L } },
		{ "m", in_t{ sf::Keyboard::M } },
		{ "n", in_t{ sf::Keyboard::N } },
		{ "o", in_t{ sf::Keyboard::O } },
		{ "p", in_t{ sf::Keyboard::P } },
		{ "q", in_t{ sf::Keyboard::Q } },
		{ "r", in_t{ sf::Keyboard::R } },
		{ "s", in_t{ sf::Keyboard::S } },
		{ "t", in_t{ sf::Keyboard::T } },
		{ "u", in_t{ sf::Keyboard::U } },
		{ "v", in_t{ sf::Keyboard::V } },
		{ "w", in_t{ sf::Keyboard::W } },
		{ "x", in_t{ sf::Keyboard::X } },
		{ "y", in_t{ sf::Keyboard::Y } },
		{ "z", in_t{ sf::Keyboard::Z } },
		{ "0", in_t{ sf::Keyboard::Num0 } },
		{ "1", in_t{ sf::Keyboard::Num1 } },
		{ "2", in_t{ sf::Keyboard::Num2 } },
		{ "3", in_t{ sf::Keyboard::Num3 } },
		{ "4", in_t{ sf::Keyboard::Num4 } },
		{ "5", in_t{ sf::Keyboard::Num5 } },
		{ "6", in_t{ sf::Keyboard::Num6 } },
		{ "7", in_t{ sf::Keyboard::Num7 } },
		{ "8", in_t{ sf::Keyboard::Num8 } },
		{ "9", in_t{ sf::Keyboard::Num9 } },
		{ "left-mouse-button", in_t{ sf::Mouse::Button::Left } },
		{ "right-mouse-button", in_t{ sf::Mouse::Button::Right } },
		{ "middle-mouse-button", in_t{ sf::Mouse::Button::Middle } },
		{ "mouse-button-4", in_t{ sf::Mouse::Button::XButton1 } },
		{ "mouse-button-5", in_t{ sf::Mouse::Button::XButton2 } },
		{ "mouse-scroll-up", in_t{ 5 } },
		{ "mouse-scroll-down", in_t{ 6 } }
	};

	const std::unordered_map<std::string, enum class mark::command::type> command_dict{
		{ "move", mark::command::type::move },
		{ "ability", mark::command::type::shoot },
		{ "ability-0", mark::command::type::ability_0 },
		{ "ability-1", mark::command::type::ability_1 },
		{ "ability-2", mark::command::type::ability_2 },
		{ "ability-3", mark::command::type::ability_3 },
		{ "ability-4", mark::command::type::ability_4 },
		{ "ability-5", mark::command::type::ability_5 },
		{ "ability-6", mark::command::type::ability_6 },
		{ "ability-7", mark::command::type::ability_7 },
		{ "ability-8", mark::command::type::ability_8 },
		{ "ability-9", mark::command::type::ability_9 },
		{ "reset", mark::command::type::reset },
		{ "activate", mark::command::type::activate }
	};

	static auto load_options(std::string filename) {
		const auto options = YAML::LoadFile(filename);
		const auto keybindings = options["keybindings"];
		std::unordered_map<in_t, enum class mark::command::type> map;
		for (const auto& pair : keybindings) {
			const auto key = pair.first.as<std::string>();
			const auto value = pair.second.as<std::string>();
			map[key_dict.at(value)] = command_dict.at(key);
		}
		return map;
	}

	static auto event_to_command(
		const std::unordered_map<in_t, enum class mark::command::type>& dict,
		const sf::Event& event) {

		mark::command command;
		command.type = mark::command::type::null;
		if (event.type == sf::Event::MouseButtonPressed
			|| event.type == sf::Event::MouseButtonReleased
			|| event.type == sf::Event::KeyPressed
			|| event.type == sf::Event::KeyReleased) {
			const auto is_mouse = event.type == sf::Event::MouseButtonPressed
				|| event.type == sf::Event::MouseButtonReleased;
			command.release = event.type == sf::Event::MouseButtonReleased
				|| event.type == sf::Event::KeyReleased;
			auto type_it = is_mouse
				? dict.find(event.mouseButton.button)
				: dict.find(event.key.code);
			if (type_it != dict.end()) {
				command.type = type_it->second;
			}
		}
		return command;
	}
}

mark::keymap::keymap(std::string filename) {
	m_map = ::load_options("options.yml");
}

auto mark::keymap::translate(sf::Event event) const -> mark::command {
	return ::event_to_command(m_map, event);
}