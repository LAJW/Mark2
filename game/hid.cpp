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

auto make_make_command(int8_t id) -> mark::hid::make_command_type
{
	return [=](const mark::vector<int>&,
			   const mark::vector<double>& mouse_pos,
			   bool shift,
			   bool release) -> mark::command::any {
		if (shift) {
			if (release) {
				return mark::command::release{ id, mouse_pos };
			}
			return mark::command::queue{ id, mouse_pos };
		}
		if (release) {
			return mark::command::release{ id, mouse_pos };
		}
		return mark::command::activate{ id, mouse_pos };
	};
}

auto make_command_move(
	const mark::vector<int>& screen_pos,
	const mark::vector<double>& mouse_pos,
	bool shift,
	bool release) -> mark::command::any
{
	return mark::command::move{ mouse_pos, release, shift, screen_pos };
};

auto make_command_guide(
	const mark::vector<int>& screen_pos,
	const mark::vector<double>& mouse_pos,
	bool,
	bool) -> mark::command::any
{
	return mark::command::guide{ mouse_pos, screen_pos };
};

auto make_command_use(
	const mark::vector<int>&,
	const mark::vector<double>&,
	bool,
	bool release) -> mark::command::any
{
	if (release) {
		return std::monostate();
	}
	return mark::command::use();
};

const std::unordered_map<std::string, mark::hid::make_command_type>
	command_dict{ { "move", make_command_move },
				  { "guide", make_command_guide },
				  { "ability", make_make_command(0) },
				  { "ability-0", make_make_command(1) },
				  { "ability-1", make_make_command(2) },
				  { "ability-2", make_make_command(3) },
				  { "ability-3", make_make_command(4) },
				  { "ability-4", make_make_command(5) },
				  { "ability-5", make_make_command(6) },
				  { "ability-6", make_make_command(7) },
				  { "ability-7", make_make_command(8) },
				  { "ability-8", make_make_command(9) },
				  { "ability-9", make_make_command(10) },
				  { "activate", make_command_use } };
} // namespace

mark::hid::hid(const YAML::Node& keybindings)
{
	for (let& pair : keybindings) {
		let key = pair.first.as<std::string>();
		let value = pair.second.as<std::string>();
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

auto mark::hid::commands(
	const mark::vector<int>& screen_pos,
	const mark::vector<double>& world_pos) -> std::vector<command::any>
{
	std::vector<command::any> out;
	for (let& button : m_pressed) {
		let it = m_to_command.find(button);
		if (it != m_to_command.cend()) {
			out.push_back(it->second(screen_pos, world_pos, m_shift, false));
		} else if (let key = std::get_if<sf::Keyboard::Key>(&button)) {
			if (*key == sf::Keyboard::Escape) {
				out.push_back(command::cancel());
			}
		}
	}
	m_pressed.clear();
	for (let& button : m_released) {
		let it = m_to_command.find(button);
		if (it != m_to_command.cend()) {
			out.push_back(it->second(screen_pos, world_pos, m_shift, true));
		}
	}
	m_released.clear();

	out.push_back(
		command_dict.at("guide")(screen_pos, world_pos, m_shift, true));

	for (let& command : out) {
		if (let move = std::get_if<command::move>(&command)) {
			m_moving = !move->release;
		}
	}
	if (m_moving) {
		out.push_back(command::move{ world_pos, true });
	}
	return out;
}
