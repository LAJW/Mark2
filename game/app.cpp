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
#include "unit_modular.h"
#include "yaml-cpp\yaml.h"
#include <fstream>

mark::app::app(const int argc, const char* argv[])
	:app({ argv, argv + argc }) {}

mark::app::app(std::vector<std::string> arguments) 
	:m_window(sf::VideoMode(1920, 1080), "MARK2") {
}

void mark::app::main() {


	const auto keymap = mark::keymap("options.yml");

	std::unique_ptr<mark::world> world;
	try {
		const auto node = YAML::LoadFile("state.yml");
		world = std::make_unique<mark::world>(m_resource_manager, node);
	} catch (std::exception&) {
		world = std::make_unique<mark::world>(m_resource_manager);
	}

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
					m_window.setView(sf::View(sf::FloatRect(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height))));
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

			const auto ship = std::dynamic_pointer_cast<mark::unit::modular>(world->target());
			if (ship) {
				const auto grid = m_resource_manager.image("grid-background.png");
				const auto list = ship->bound_status();
				const auto resolution = m_window.getSize();
				const auto icon_size = 64.0;
				for (uint8_t i = 0; i < list.size(); i++) {
					const auto di = static_cast<double>(i);
					const auto& status = list[i];
					mark::sprite::info sprite;
					sprite.image = status.thumbnail;
					sprite.size = icon_size;
					sprite.pos = world->camera() + mark::vector<double>(
						di * icon_size - icon_size * list.size() / 2,
						resolution.y / 2 - icon_size
					);
					if (status.thumbnail) {
						context.sprites[101].emplace_back(sprite);
					}
					sprite.image = grid;
					context.sprites[100].emplace_back(sprite);
					{
						std::ostringstream os;
						os << static_cast<int>(i);
						mark::print(
							world->image_font,
							context.sprites[102],
							sprite.pos + mark::vector<double>(-24.f, 7.f),
							{ 300 - 14.f, 300 - 14.f },
							14.f,
							sf::Color::White,
							os.str()
						);
					}
					{
						std::ostringstream os;
						os << status.total;
						mark::print(
							world->image_font,
							context.sprites[102],
							sprite.pos + mark::vector<double>(16.f, 7.f),
							{ 300 - 14.f, 300 - 14.f },
							14.f,
							sf::Color::White,
							os.str()
						);
					}
				}
			}
			renderer.render(context, world->camera(), resolution, m_window);
		}
	}

	YAML::Emitter out;
	world->serialize(out);
	std::ofstream state("state.yml");
	state << out.c_str();
}
