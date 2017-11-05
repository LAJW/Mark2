#pragma once
#include "stdafx.h"
#include <variant>
#include <SFML/Window/event.hpp>
#include "command.h"

namespace mark {
class hid final {
public:
	using button = std::variant<sf::Keyboard::Key, sf::Mouse::Button, int>;
	// Create a new HID from keybindings YAML Node
	hid(const YAML::Node& node);
	// Handle an sfml event
	void handle(const sf::Event& event);
	// Generate list of commands, flush hid buffers. Mouse pos for commands,
	// relative to the world
	auto commands(const vector<double>& mouse_pos) -> std::vector<command::any>;
	using make_command_type = std::function<command::any(
		const mark::vector<double>& mouse_pos, bool shift, bool release)>;
private:
	// Buttons pressed in a specific frame
	std::vector<button> m_pressed;
	// Buttons released in specific frame
	std::vector<button> m_released;
	// Map from sfml device button to command (Loaded from file)
	std::unordered_map<button, make_command_type> m_to_command;
	bool m_shift;
};
}