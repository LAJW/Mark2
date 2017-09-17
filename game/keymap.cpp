#include "stdafx.h"
#include "keymap.h"
#include "yaml-cpp\yaml.h"

namespace {
	
	const std::unordered_map<std::string, std::pair<uint8_t, uint8_t>> key_dict{
		{ "a", { 0, sf::Keyboard::A } },
		{ "b", { 0, sf::Keyboard::B } },
		{ "c", { 0, sf::Keyboard::C } },
		{ "d", { 0, sf::Keyboard::D } },
		{ "e", { 0, sf::Keyboard::E } },
		{ "f", { 0, sf::Keyboard::F } },
		{ "g", { 0, sf::Keyboard::G } },
		{ "h", { 0, sf::Keyboard::H } },
		{ "i", { 0, sf::Keyboard::I } },
		{ "j", { 0, sf::Keyboard::J } },
		{ "k", { 0, sf::Keyboard::K } },
		{ "l", { 0, sf::Keyboard::L } },
		{ "m", { 0, sf::Keyboard::M } },
		{ "n", { 0, sf::Keyboard::N } },
		{ "o", { 0, sf::Keyboard::O } },
		{ "p", { 0, sf::Keyboard::P } },
		{ "q", { 0, sf::Keyboard::Q } },
		{ "r", { 0, sf::Keyboard::R } },
		{ "s", { 0, sf::Keyboard::S } },
		{ "t", { 0, sf::Keyboard::T } },
		{ "u", { 0, sf::Keyboard::U } },
		{ "v", { 0, sf::Keyboard::V } },
		{ "w", { 0, sf::Keyboard::W } },
		{ "x", { 0, sf::Keyboard::X } },
		{ "y", { 0, sf::Keyboard::Y } },
		{ "z", { 0, sf::Keyboard::Z } },
		{ "0", { 0, sf::Keyboard::Num0 } },
		{ "1", { 0, sf::Keyboard::Num1 } },
		{ "2", { 0, sf::Keyboard::Num2 } },
		{ "3", { 0, sf::Keyboard::Num3 } },
		{ "4", { 0, sf::Keyboard::Num4 } },
		{ "5", { 0, sf::Keyboard::Num5 } },
		{ "6", { 0, sf::Keyboard::Num6 } },
		{ "7", { 0, sf::Keyboard::Num7 } },
		{ "8", { 0, sf::Keyboard::Num8 } },
		{ "9", { 0, sf::Keyboard::Num9 } },
		{ "left-mouse-button", { 1, sf::Mouse::Button::Left } },
		{ "right-mouse-button", { 1, sf::Mouse::Button::Right } },
		{ "middle-mouse-button", { 1, sf::Mouse::Button::Middle } },
		{ "mouse-button-4", { 1, sf::Mouse::Button::XButton1 } },
		{ "mouse-button-5", { 1, sf::Mouse::Button::XButton2 } },
		{ "mouse-scroll-up", { 1, 5 } },
		{ "mouse-scroll-down", { 1, 6 } }
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
		std::unordered_map<std::pair<uint8_t, uint8_t>, enum class mark::command::type> map;
		for (const auto& pair : keybindings) {
			const auto key = pair.first.as<std::string>();
			const auto value = pair.second.as<std::string>();
			map[key_dict.at(value)] = command_dict.at(key);
		}
		return map;
	}

	static auto event_to_command(
		const std::unordered_map<std::pair<uint8_t, uint8_t>,
		enum class mark::command::type>& dict,
		const sf::Event& event) {

		mark::command command;
		command.type = mark::command::type::null;
		if (event.type == sf::Event::MouseButtonPressed
			|| event.type == sf::Event::MouseButtonReleased
			|| event.type == sf::Event::KeyPressed
			|| event.type == sf::Event::KeyReleased) {
			const auto is_mouse = event.type == sf::Event::MouseButtonPressed
				|| event.type == sf::Event::MouseButtonReleased;
			const auto released = event.type == sf::Event::MouseButtonReleased
				|| event.type == sf::Event::KeyReleased;
			const auto button = is_mouse
				? static_cast<uint8_t>(event.mouseButton.button)
				: static_cast<uint8_t>(event.key.code);
			command.release = released;
			const auto type_it = dict.find({ static_cast<uint8_t>(is_mouse), static_cast<uint8_t>(button) });
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