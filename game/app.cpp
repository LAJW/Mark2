#include <chrono>
#include "app.h"
#include <SFML/Graphics.hpp>
#include <yaml-cpp/yaml.h>
#include "resource_image.h"
#include "resource_manager.h"
#include "world.h"
#include "sprite.h"
#include "vector.h"
#include <iostream>
#include "command.h"

namespace std {
	template<>
	struct hash<std::pair<uint8_t, uint8_t>> {
		size_t operator()(const std::pair<uint8_t, uint8_t>& pair) const {
			return std::hash<uint16_t>()((pair.first * 256) + pair.second);
		}
	};
}

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
		const std::unordered_map<std::pair<uint8_t, uint8_t>, enum class mark::command::type>& dict,
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

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
	m_fragment_shader.loadFromFile("shader.glsl", sf::Shader::Fragment);
	m_buffer.create(1920, 1080);
	m_buffer2.create(1920, 1080);
}

void mark::app::main() {
	const auto key_map = ::load_options("options.yml");
	auto world = std::make_unique<mark::world>(m_resource_manager);

	auto last = std::chrono::system_clock::now();

	sf::RenderTexture occlusion_map;
	occlusion_map.create(1920, 1080);
	sf::RenderTexture vbo;
	vbo.create(512, 1);
	sf::Shader occlusion_shader;
	occlusion_shader.loadFromFile("occlusion.glsl", sf::Shader::Fragment);
	sf::Shader shadows_shader;
	shadows_shader.loadFromFile("shadows.glsl", sf::Shader::Fragment);
	sf::Shader bump_mapping;
	bump_mapping.loadFromFile("bump_mapping.glsl", sf::Shader::Fragment);

	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;

		if (dt >= 1.0 / 60.0) {
			last = now;
			sf::Event event;

			const auto target = world->camera() + mark::vector<double>(sf::Mouse::getPosition(m_window)) - mark::vector<double>(m_window.getSize()) / 2.0;
			while (m_window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					m_window.close();
				} else {
					auto command = event_to_command(key_map, event);
					command.pos = target;
					if (command.type == mark::command::type::reset) {
						world = std::make_unique<mark::world>(m_resource_manager);
					} else {
						world->command(command);
					}
				}
			}
			mark::command guide;
			guide.type = mark::command::type::guide;
			guide.pos = target;
			world->command(guide);

			m_window.clear();
			m_buffer.clear({ 0, 0, 0, 0 });
			m_buffer2.clear();
			vbo.clear(sf::Color::White);
			occlusion_map.clear();
			auto sprites = world->tick(dt, mark::vector<double>(m_window.getSize()));
			for (const auto& layer : sprites) {
				if (layer.first < 0) {
					for (const auto& sprite : layer.second) {
						render(sprite, world->camera(), occlusion_map);
					}
				} else {
					for (const auto& sprite : layer.second) {
						render(sprite, world->camera(), m_buffer);
					}
				}
			}
			occlusion_map.display();
			sf::Sprite sprite1(occlusion_map.getTexture());
			sprite1.scale({ 512.f / 1920.f, 1.f / 1080.f });
			vbo.draw(sprite1, &occlusion_shader);
			vbo.display();
			m_buffer.display();
			m_buffer2.draw(sf::Sprite(occlusion_map.getTexture()), &bump_mapping);
			m_buffer2.draw(sf::Sprite(m_buffer.getTexture()));
			m_buffer2.draw(sf::Sprite(vbo.getTexture()));
			sf::Sprite shadows(vbo.getTexture());
			shadows.setScale({ 1920.f / 512.f, 1080.f / 1.f });;
			m_buffer2.draw(shadows, &shadows_shader);
			m_buffer2.display();
			m_window.draw(sf::Sprite(m_buffer2.getTexture()));
			m_window.display();
		}
	}
}

void mark::app::render(const mark::sprite& sprite, const mark::vector<double>& camera, sf::RenderTexture& buffer) {
	sf::Sprite tmp;
	const auto texture_size = static_cast<float>(sprite.image().getSize().y);
	const auto scale = sprite.size() / texture_size;
	tmp.setTexture(sprite.image());
	tmp.setTextureRect({ static_cast<int>(texture_size) * static_cast<int>(sprite.frame()), 0, static_cast<int>(texture_size), static_cast<int>(texture_size)  });
	tmp.setOrigin(texture_size / 2.f, texture_size / 2.f);
	tmp.scale(scale, scale);
	tmp.rotate(sprite.rotation());
	tmp.setColor(sprite.color());
	tmp.move(static_cast<float>(sprite.x() - camera.x + m_window.getSize().x / 2.0), static_cast<float>(sprite.y() - camera.y + m_window.getSize().y / 2.0));
	buffer.draw(tmp);
}