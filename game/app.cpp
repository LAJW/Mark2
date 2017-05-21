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
#include "tick_context.h"
#include "keymap.h"
#include "renderer.h"

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
}

void mark::app::main() {
	const auto keymap = mark::keymap("options.yml");
	auto world = std::make_unique<mark::world>(m_resource_manager);
	mark::renderer renderer({ 1920, 1080 }, 512);

	auto last = std::chrono::system_clock::now();

	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;

		if (dt >= 1.0 / 60.0) {
			last = now;
			sf::Event event;

			const auto target = world->camera() + mark::vector<double>(sf::Mouse::getPosition(m_window)) - mark::vector<double>(m_window.getSize()) / 2.0;
			while (m_window.pollEvent(event)) {
				if (event.type == sf::Event::Resized) {
					m_window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
				} if (event.type == sf::Event::Closed) {
					m_window.close();
				} else {
					auto command = keymap.translate(event);
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

			mark::tick_context context(m_resource_manager);
			context.dt = dt;
			const auto resolution = mark::vector<double>(m_window.getSize());
			world->tick(context, resolution);
			renderer.render(context, world->camera(), resolution, m_window);
		}
	}
}
