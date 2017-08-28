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

void ui(
	mark::tick_context& context,
	mark::resource::manager& rm,
	std::shared_ptr<mark::ihas_bindings> unit_with_bindings,
	mark::vector<double> camera,
	mark::vector<double> resolution)
{
	{
		mark::sprite::info bg_sprite;
		bg_sprite.image = rm.image("hotbar-background.png");
		bg_sprite.pos.x = resolution.x / 2. - 23. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 85;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[0].emplace_back(bg_sprite);
	}
	{
		mark::sprite::info bg_sprite;
		bg_sprite.image = rm.image("hotbar-overlay.png");
		bg_sprite.pos.x = resolution.x / 2. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 64;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[2].emplace_back(bg_sprite);
	}

	const auto grid = rm.image("grid-background.png");
	const auto bindings = unit_with_bindings->bindings();
	const auto icon_size = 64.0;
	for (const auto i : mark::enumerate(bindings.size())) {
		const auto& binding = bindings[i];
		const auto di = static_cast<double>(i);
		const auto x = resolution.x / 2. - 64. * 5.5 + 64.0 * i;
		const auto y = resolution.y - 64;;
		if (binding.thumbnail) {
			mark::sprite::info sprite;
			sprite.image = binding.thumbnail;
			sprite.pos.x = x;
			sprite.pos.y = y;
			sprite.image = binding.thumbnail;
			sprite.size = 64.f;
			context.ui_sprites[1].emplace_back(sprite);
		}
		{
			std::ostringstream os;
			os << static_cast<int>(i);
			mark::print(
				rm.image("font.png"),
				context.ui_sprites[1],
				mark::vector<double>(x + 32.f, y + 8.f),
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
				rm.image("font.png"),
				context.ui_sprites[1],
				mark::vector<double>(x + 32.f, y + 32.f),
				{ 300 - 14.f, 300 - 14.f },
				14.f,
				sf::Color::White,
				os.str()
			);
		}
	}
}

mark::renderer::render_info tick(
	double dt,
	mark::resource::manager& rm,
	mark::world& world,
	mark::vector<double> resolution)
{
	mark::tick_context context(rm);
	context.dt = dt;
	world.tick(context, resolution);
	if (const auto unit_with_bindings =
			std::dynamic_pointer_cast<mark::ihas_bindings>(world.target())) {
		ui(context, rm, unit_with_bindings, world.camera(), resolution);
	}

	mark::renderer::render_info info;
	info.camera = world.camera();
	info.resolution = resolution;
	info.lights = std::move(context.lights);
	info.sprites = std::move(context.sprites);
	info.ui_sprites = std::move(context.ui_sprites);
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

struct on_event_info {
	sf::Event event;
	mark::vector<double> window_res;
	mark::vector<double> mouse_pos;
};

struct on_tick_info {
	double dt;
	mark::vector<double> window_res;
	mark::vector<double> mouse_pos;
};

void event_loop(
	std::string window_title,
	mark::vector<unsigned> res,
	std::function<void(on_event_info)> on_event,
	std::function<mark::renderer::render_info(on_tick_info)> on_tick)
{
	assert(on_event);
	assert(on_tick);
	sf::RenderWindow m_window(sf::VideoMode(res.x, res.y), window_title);
	mark::renderer renderer(res, 512);

	auto last = std::chrono::system_clock::now();
	while (m_window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(
			std::chrono::duration_cast<std::chrono::microseconds>(now - last)
				.count())
			/ 1000000.0;

		sf::Event event;
		while (m_window.pollEvent(event)) {
			if (event.type == sf::Event::Resized) {
				const auto width = static_cast<float>(event.size.width);
				const auto height = static_cast<float>(event.size.height);
				const auto view = sf::View(sf::FloatRect(0, 0, width, height));
				m_window.setView(view);
			} if (event.type == sf::Event::Closed) {
				m_window.close();
			} else {
				on_event_info info;
				info.mouse_pos = mark::vector<double>(sf::Mouse::getPosition(m_window));
				info.window_res = mark::vector<double>(m_window.getSize());
				info.event = event;
				on_event(info);
			}
		}

		if (dt >= 1.0 / 60.0) {
			last = now;

			on_tick_info info;
			info.dt = dt;
			info.mouse_pos = mark::vector<double>(sf::Mouse::getPosition(m_window));
			info.window_res = mark::vector<double>(m_window.getSize());
			m_window.draw(renderer.render(on_tick(info)));
			m_window.display();
		}
	}

}

namespace mark {
	int main(std::vector<std::string> args);
}

int mark::main(std::vector<std::string> args)
{
	try {
		mark::resource::manager_impl rm;
		const auto keymap = mark::keymap("options.yml");
		auto world = load_or_create_world("state.yml", rm);
		const auto on_event = [&](const on_event_info& info) {
			const auto mouse_pos = info.mouse_pos;
			const auto window_res = info.window_res;
			const auto target = world->camera() + mouse_pos - window_res / 2.;
			auto command = keymap.translate(info.event);
			command.pos = target;
			if (command.type == mark::command::type::reset) {
				world = std::make_unique<mark::world>(rm);
			} else {
				world->command(command);
			}
		};
		const auto on_tick = [&](const on_tick_info& info) {
			const auto mouse_pos = info.mouse_pos;
			const auto window_res = info.window_res;
			const auto target = world->camera() + mouse_pos - window_res / 2.;
			mark::command guide;
			guide.type = mark::command::type::guide;
			guide.pos = target;
			world->command(guide);
			mark::tick_context context(rm);
			return tick(info.dt, rm, *world, window_res);
		};
		event_loop("MARK 2", { 1920, 1080 }, on_event, on_tick);
		save_world(*world, "state.yml");
		return 0;
	} catch (std::exception& error) {
		std::cout << "ERROR: " << error.what() << std::endl;
		std::cin.get();
		return 1;
	}
}

int main(const int argc, const char* argv[]) {
	return mark::main({ argv, argv + argc });
}