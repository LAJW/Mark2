#include <chrono>
#include "app.h"
#include <SFML/Graphics.hpp>
#include "resource_image.h"
#include "resource_manager.h"
#include "world.h"
#include "sprite.h"
#include "vector.h"
#include <iostream>
#include "yaml-cpp\yaml.h"
#include "command.h"

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
}

void mark::app::main() {
	auto world = std::make_unique<mark::world>(m_resource_manager);

	auto last = std::chrono::system_clock::now();
	bool moving = false;
	bool shooting = false;

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
				} else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == 0) {
					world->command(mark::command{ mark::command::type::move, target });
					moving = true;
				} else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == 0) {
					moving = false;
				} else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == 1) {
					shooting = true;
				} else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == 1) {
					shooting = false;
				} else if (event.type == sf::Event::KeyPressed && event.key.code == 17) {
					world = std::make_unique<mark::world>(m_resource_manager);
				} else if (event.type == sf::Event::MouseMoved) {
					world->command(mark::command{ mark::command::type::guide, target });
					if (moving) {
						world->command(mark::command{ mark::command::type::move, target });
					}
				} else if (event.type == sf::Event::KeyPressed && event.key.code == 5) {
					world->command(mark::command{ mark::command::type::activate, mark::vector<double>() });
				}
			}

			if (shooting) {
				world->command(mark::command{ mark::command::type::shoot, target });
			}

			m_window.clear();
			auto sprites = world->tick(dt, mark::vector<double>(m_window.getSize()));
			for (const auto& layer : sprites) {
				for (const auto& sprite : layer.second) {
					render(sprite, world->camera());
				}
			}
			m_window.display();
		}
	}
}

void mark::app::render(const mark::sprite& sprite, const mark::vector<double>& camera) {
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
	m_window.draw(tmp);
}