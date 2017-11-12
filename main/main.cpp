#include <iostream>
#include "stdafx.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>

#include "algorithm.h"
#include "command.h"
#include "hid.h"
#include "renderer.h"
#include "resource_image.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "world_stack.h"
#include "world.h"
#include "ui.h"


extern "C" {
	// Enable dedicated graphics for NVIDIA
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	// Enable dedicated graphics for AMD Radeon
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


mark::renderer::render_info tick(
	double dt,
	mark::ui::ui& ui,
	mark::resource::manager& rm,
	mark::world& world,
	mark::vector<double> resolution,
	mark::vector<double> mouse_pos)
{
	mark::tick_context context(rm);
	context.dt = dt;
	if (false) {
		world.tick(context, resolution);
	}
	ui.tick(world, context, rm, resolution, mouse_pos);

	mark::renderer::render_info info;
	info.camera = world.camera();
	info.resolution = resolution;
	info.lights = std::move(context.lights);
	info.sprites = std::move(context.sprites);
	info.normals = std::move(context.normals);
	return info;
}

void save_world(const mark::world& world, std::string filename)
{
	YAML::Emitter out;
	world.serialise(out);
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
	mark::renderer renderer(res);

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
	void main(std::vector<std::string> args);
}

void mark::main(std::vector<std::string> args)
{
	mark::resource::manager_impl rm;
	mark::ui::ui ui(rm);
	const auto options = YAML::LoadFile("options.yml");
	mark::hid hid(options["keybindings"]);
	std::unordered_map<std::string, YAML::Node> templates;
	templates["ship"] = YAML::LoadFile("ship.yml");
	mark::world_stack world_stack(YAML::LoadFile("state.yml"), rm, templates);
	const auto on_event = [&](const on_event_info& info) {
		if (info.event.type == sf::Event::MouseMoved) {
			ui.hover(round(info.mouse_pos));
		}
		if (info.event.mouseButton.button == sf::Mouse::Button::Left
			&& info.event.type == sf::Event::MouseButtonPressed) {
			if (ui.click(round(info.mouse_pos))) {
				return;
			}
		}
		hid.handle(info.event);
	};
	bool moving = false;
	const auto on_tick = [&](const on_tick_info& info) {
		const auto target = world_stack.world().camera()
			+ info.mouse_pos - info.window_res / 2.;
		for (const auto command : hid.commands(target)) {
			ui.command(world_stack.world(), command);
			if (const auto move = std::get_if<command::move>(&command)) {
				moving = !move->release;
			}
			if (const auto move = std::get_if<command::guide>(&command)) {
				if (moving) {
					world_stack.world().command(command::move{ move->pos, true });
				}
			}
			world_stack.world().command(command);
		}
		mark::tick_context context(rm);
		return tick(
			info.dt, ui, rm, world_stack.world(), info.window_res, info.mouse_pos);
	};
	event_loop("MARK 2", { 1920, 1080 }, on_event, on_tick);
	ui.release();
	save_world(world_stack.world(), "state.yml");
}

int main(const int argc, const char* argv[]) {
	try {
		mark::main({ argv, argv + argc });
		return 0;
	} catch (std::exception& error) {
		std::cout << "ERROR: " << error.what() << std::endl;
		std::cin.get();
		return 1;
	}
}