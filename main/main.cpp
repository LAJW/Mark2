#include <iostream>
#include "stdafx.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "algorithm.h"
#include "command.h"
#include "keymap.h"
#include "renderer.h"
#include "resource_image.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "vector.h"
#include "world.h"


extern "C" {
	// Enable dedicated graphics for NVIDIA
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	// Enable dedicated graphics for AMD Radeon
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

mark::renderer::render_info tick(double dt, mark::resource::manager& rm, mark::world& world, mark::vector<double> resolution)
{
	mark::tick_context context(rm);
	context.dt = dt;
	world.tick(context, resolution);

	const auto unit_with_bindings = std::dynamic_pointer_cast<mark::ihas_bindings>(world.target());
	if (unit_with_bindings) {
		const auto grid = rm.image("grid-background.png");
		const auto bindings = unit_with_bindings->bindings();
		const auto icon_size = 64.0;
		for (const auto i : mark::enumerate(bindings.size())) {
			const auto& binding = bindings[i];
			const auto di = static_cast<double>(i);
			mark::sprite::info sprite;
			sprite.image = binding.thumbnail;
			sprite.size = icon_size;
			sprite.pos = world.camera() + mark::vector<double>(
				di * icon_size - icon_size * bindings.size() / 2,
				resolution.y / 2 - icon_size
				);
			if (binding.thumbnail) {
				context.sprites[101].emplace_back(sprite);
			}
			sprite.image = grid;
			context.sprites[100].emplace_back(sprite);
			{
				std::ostringstream os;
				os << static_cast<int>(i);
				mark::print(
					world.image_font,
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
				os << binding.total;
				mark::print(
					world.image_font,
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
	mark::renderer::render_info info;
	info.camera = world.camera();
	info.resolution = resolution;
	info.lights = std::move(context.lights);
	info.sprites = std::move(context.sprites);
	info.normals = std::move(context.normals);
	return info;
}

std::unique_ptr<mark::world> load_or_create_world(
	std::string filename, mark::resource::manager& resource_manager)
{
	try {
		const auto node = YAML::LoadFile(filename);
		return std::make_unique<mark::world>(resource_manager, node);
	} catch (std::exception&) {
		return std::make_unique<mark::world>(resource_manager);
	}

}

void save_world(const mark::world& world, std::string filename)
{
	YAML::Emitter out;
	world.serialize(out);
	std::ofstream state(filename);
	state << out.c_str();
}

namespace mark {
	int main(std::vector<std::string> args) {
		try {
			mark::resource::manager_impl resource_manager;
			sf::RenderWindow m_window(sf::VideoMode(1920, 1080), "MARK 2");
			const auto keymap = mark::keymap("options.yml");
			auto world = load_or_create_world("state.yml", resource_manager);
			mark::renderer renderer({ 1920, 1080 }, 512);
			auto last = std::chrono::system_clock::now();

			while (m_window.isOpen()) {
				const auto now = std::chrono::system_clock::now();
				const auto dt = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - last).count()) / 1000000.0;
				const auto target = world->camera() + mark::vector<double>(sf::Mouse::getPosition(m_window)) - mark::vector<double>(m_window.getSize()) / 2.0;

				sf::Event event;
				while (m_window.pollEvent(event)) {
					if (event.type == sf::Event::Resized) {
						m_window.setView(sf::View(sf::FloatRect(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height))));
					} if (event.type == sf::Event::Closed) {
						m_window.close();
					} else {
						auto command = keymap.translate(event);
						command.pos = target;
						if (command.type == mark::command::type::reset) {
							world = std::make_unique<mark::world>(resource_manager);
						} else {
							world->command(command);
						}
					}
				}

				if (dt >= 1.0 / 60.0) {
					last = now;

					mark::command guide;
					guide.type = mark::command::type::guide;
					guide.pos = target;
					world->command(guide);
					mark::tick_context context(resource_manager);
					const auto resolution = mark::vector<double>(m_window.getSize());
					const auto info = tick(dt, resource_manager, *world, resolution);
					m_window.draw(renderer.render(info));
					m_window.display();
				}
			}

			save_world(*world, "state.yml");

			return 0;
		} catch (std::exception& error) {
			std::cout << "ERROR: " << error.what() << std::endl;
			std::cin.get();
			return 1;
		}
	}
}

int main(const int argc, const char* argv[]) {
	return mark::main({ argv, argv + argc });
}