#include "stdafx.h"
#include "hid.h"

namespace {
const std::unordered_map<std::string, mark::hid::button> key_dict{
	{ "a", mark::hid::button{ sf::Keyboard::A } },
	{ "b", mark::hid::button{ sf::Keyboard::B } },
	{ "c", mark::hid::button{ sf::Keyboard::C } },
	{ "d", mark::hid::button{ sf::Keyboard::D } },
	{ "e", mark::hid::button{ sf::Keyboard::E } },
	{ "f", mark::hid::button{ sf::Keyboard::F } },
	{ "g", mark::hid::button{ sf::Keyboard::G } },
	{ "h", mark::hid::button{ sf::Keyboard::H } },
	{ "i", mark::hid::button{ sf::Keyboard::I } },
	{ "j", mark::hid::button{ sf::Keyboard::J } },
	{ "k", mark::hid::button{ sf::Keyboard::K } },
	{ "l", mark::hid::button{ sf::Keyboard::L } },
	{ "m", mark::hid::button{ sf::Keyboard::M } },
	{ "n", mark::hid::button{ sf::Keyboard::N } },
	{ "o", mark::hid::button{ sf::Keyboard::O } },
	{ "p", mark::hid::button{ sf::Keyboard::P } },
	{ "q", mark::hid::button{ sf::Keyboard::Q } },
	{ "r", mark::hid::button{ sf::Keyboard::R } },
	{ "s", mark::hid::button{ sf::Keyboard::S } },
	{ "t", mark::hid::button{ sf::Keyboard::T } },
	{ "u", mark::hid::button{ sf::Keyboard::U } },
	{ "v", mark::hid::button{ sf::Keyboard::V } },
	{ "w", mark::hid::button{ sf::Keyboard::W } },
	{ "x", mark::hid::button{ sf::Keyboard::X } },
	{ "y", mark::hid::button{ sf::Keyboard::Y } },
	{ "z", mark::hid::button{ sf::Keyboard::Z } },
	{ "0", mark::hid::button{ sf::Keyboard::Num0 } },
	{ "1", mark::hid::button{ sf::Keyboard::Num1 } },
	{ "2", mark::hid::button{ sf::Keyboard::Num2 } },
	{ "3", mark::hid::button{ sf::Keyboard::Num3 } },
	{ "4", mark::hid::button{ sf::Keyboard::Num4 } },
	{ "5", mark::hid::button{ sf::Keyboard::Num5 } },
	{ "6", mark::hid::button{ sf::Keyboard::Num6 } },
	{ "7", mark::hid::button{ sf::Keyboard::Num7 } },
	{ "8", mark::hid::button{ sf::Keyboard::Num8 } },
	{ "9", mark::hid::button{ sf::Keyboard::Num9 } },
	{ "left-mouse-button", mark::hid::button{ sf::Mouse::Button::Left } },
	{ "right-mouse-button", mark::hid::button{ sf::Mouse::Button::Right } },
	{ "middle-mouse-button", mark::hid::button{ sf::Mouse::Button::Middle } },
	{ "mouse-button-4", mark::hid::button{ sf::Mouse::Button::XButton1 } },
	{ "mouse-button-5", mark::hid::button{ sf::Mouse::Button::XButton2 } },
	{ "mouse-scroll-up", mark::hid::button{ 5 } },
	{ "mouse-scroll-down", mark::hid::button{ 6 } }
};

const std::unordered_map<std::string, enum class mark::command::type>
command_dict {
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
}

mark::hid::hid(const YAML::Node& keybindings)
{
	for (const auto& pair : keybindings) {
		const auto key = pair.first.as<std::string>();
		const auto value = pair.second.as<std::string>();
		m_to_command[key_dict.at(value)] = command_dict.at(key);
	}
}

void mark::hid::handle(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed) {
		m_pressed.push_back(event.mouseButton.button);
	} else if (event.type == sf::Event::MouseButtonReleased) {
		m_released.push_back(event.mouseButton.button);
	} else if (event.type == sf::Event::KeyPressed) {
		m_pressed.push_back(event.key.code);
		if (event.key.code == sf::Keyboard::LShift) {
			m_shift = true;
		}
	} else if (event.type == sf::Event::KeyReleased) {
		m_released.push_back(event.key.code);
		if (event.key.code == sf::Keyboard::LShift) {
			m_shift = false;
		}
	}
}

auto mark::hid::commands(const mark::vector<double>& mouse_pos)
	-> std::vector<command>
{
	std::vector<command> out;
	for (const auto& button : m_pressed) {
		const auto it = m_to_command.find(button);
		if (it != m_to_command.cend()) {
			if (it->second == command::type::activate) {
				mark::command command;
				command.type = it->second;
				command.pos =  mouse_pos;
				command.shift = m_shift;
				command.release = false;
				out.push_back(command);
			} else {
				m_active.insert(it->second);
			}
		}
	}
	for (const auto& button : m_released) {
		const auto it = m_to_command.find(button);
		if (it != m_to_command.cend()) {
			m_active.erase(it->second);
			mark::command command;
			command.type = it->second;
			command.pos =  mouse_pos;
			command.shift = m_shift;
			command.release = true;
			out.push_back(command);
		}
	}
	for (const auto& command_type : m_active) {
		mark::command command;
		command.type = command_type;
		command.pos =  mouse_pos;
		command.shift = m_shift;
		command.release = false;
		out.push_back(command);
	}
	mark::command command;
	command.type = command::type::guide;
	command.pos =  mouse_pos;
	command.release = false;
	command.shift = m_shift;
	out.push_back(command);
	m_pressed.clear();
	m_released.clear();
	return out;
}
