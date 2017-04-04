#include <chrono>
#include "app.h"
#include <SFML/Graphics.hpp>
#include "resource_image.h"
#include "resource_manager.h"
#include "world.h"
#include "sprite.h"
#include "vector.h"
#include <iostream>

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
}

void mark::app::main() {
	mark::world world(m_resource_manager);

	auto last = std::chrono::system_clock::now();


	mark::vector<float> direction;
	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;

		if (dt >= 1.0 / 60.0) {
			last = now;
			sf::Event event;

			while (m_window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					m_window.close();
				}
				if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == 22) { // W
						direction.y = -1;
					} else if (event.key.code == 18) { // S
						direction.y = 1;
					} else if (event.key.code == 0) { // A
						direction.x = -1;
					} else if (event.key.code == 3) { // D
						direction.x = 1;
					}
				}
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == 22) { // W
						direction.y = 0;
					} else if (event.key.code == 18) { // S
						direction.y = 0;
					} else if (event.key.code == 0) { // A
						direction.x = 0;
					} else if (event.key.code == 3) { // D
						direction.x = 0;
					}
				}
			}

			auto length = mark::length(direction);
			auto direction2 = direction;
			if (length) {
				direction2 /= length;
			}
			world.direction(direction2);

			m_window.clear();
			world.tick(dt);
			auto sprites = world.render();
			for (auto& sprite : sprites) {
				render(sprite, world.camera());
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
	tmp.scale(scale, scale);
	tmp.rotate(sprite.rotation());
	tmp.move(static_cast<float>(sprite.x() - camera.x + m_window.getSize().x / 2.0), static_cast<float>(sprite.y() - camera.y + m_window.getSize().y / 2.0));
	m_window.draw(tmp);
}