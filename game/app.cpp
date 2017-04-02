#include <chrono>
#include "app.h"
#include <SFML/Graphics.hpp>
#include "resource_image.h"
#include "resource_manager.h"
#include "world.h"
#include "sprite.h"

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1280, 720), "MARK2") {
}

void mark::app::main() {
	mark::world world(m_resource_manager);

	auto last = std::chrono::system_clock::now();

	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;

		if (dt >= 1.0 / 60.0) {
			last = now;

			sf::Event event;
			while (m_window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					m_window.close();
			}


			m_window.clear();
			world.tick(dt);
			auto sprites = world.render();
			for (auto& sprite : sprites) {
				render(sprite);
			}
			m_window.display();
		}
	}
}

void mark::app::render(const mark::sprite& sprite) {
	sf::Sprite tmp;
	const auto texture_size = static_cast<float>(sprite.image().getSize().y);
	const auto scale = sprite.size() / texture_size;
	tmp.setTexture(sprite.image());
	tmp.scale(scale, scale);
	tmp.rotate(sprite.rotation());
	tmp.move(static_cast<float>(sprite.x()), static_cast<float>(sprite.y()));
	m_window.draw(tmp);
}