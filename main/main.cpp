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

struct event_loop_info {
	std::string window_title;
	mark::vector<unsigned> res;
	std::function<void(on_event_info)> on_event;
	std::function<mark::renderer::render_info(on_tick_info)> on_tick;
	mark::mode_stack* stack;
};

void event_loop(event_loop_info& info)
{
	assert(info.on_event);
	assert(info.on_tick);
	assert(info.stack);
	sf::RenderWindow window({ info.res.x, info.res.y }, info.window_title);
	mark::renderer renderer(info.res);
	const auto& on_tick = info.on_tick;
	const auto& on_event = info.on_event;

	auto last = std::chrono::system_clock::now();
	while (window.isOpen()) {
		const auto now = std::chrono::system_clock::now();
		const auto dt = static_cast<double>(
			std::chrono::duration_cast<std::chrono::microseconds>(now - last)
				.count())
			/ 1000000.0;

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Resized) {
				const auto width = static_cast<float>(event.size.width);
				const auto height = static_cast<float>(event.size.height);
				window.setView(sf::View({ 0.f, 0.f, width, height }));
			} if (event.type == sf::Event::Closed) {
				window.close();
			} else {
				on_event_info info;
				info.mouse_pos = mark::vector<double>(sf::Mouse::getPosition(window));
				info.window_res = mark::vector<double>(window.getSize());
				info.event = event;
				on_event(info);
			}
		}
		if (info.stack->get().empty()) {
			break;
			window.close();
		}

		if (dt >= 1.0 / 60.0) {
			last = now;

			on_tick_info info;
			info.dt = dt;
			info.mouse_pos = mark::vector<double>(sf::Mouse::getPosition(window));
			info.window_res = mark::vector<double>(window.getSize());
			window.draw(renderer.render(on_tick(info)));
			window.display();
		}
	}
}

namespace mark {
	void main(std::vector<std::string> args);
}

void mark::main(std::vector<std::string> args)
{
	mode_stack stack;
	mark::resource::manager_impl rm;
	std::unordered_map<std::string, YAML::Node> templates;
	templates["ship"] = YAML::LoadFile("ship.yml");
	mark::world_stack world_stack(YAML::LoadFile("state.yml"), rm, templates);
	mark::ui::ui ui(rm, stack, world_stack);
	const auto options = YAML::LoadFile("options.yml");
	mark::hid hid(options["keybindings"]);
	event_loop_info event_loop_info;
	event_loop_info.window_title = "mark 2";
	event_loop_info.stack = &stack;
	event_loop_info.res = { 1920, 1080 };
	event_loop_info.on_event = [&](const on_event_info& info) {
		hid.handle(info.event);
		auto& world = world_stack.world();
		const auto target = world.camera()
			+ info.mouse_pos - info.window_res / 2.;
		for (const auto command : hid.commands(round(info.mouse_pos), target)) {
			if (!ui.command(world, command)
				&& !stack.paused()) {
				world.command(command);
			}
		}
	};
	event_loop_info.on_tick = [&](const on_tick_info& info) {
		auto& world = world_stack.world();
		const auto resolution = info.window_res;
		mark::tick_context context(rm);
		context.dt = info.dt;
		if (!stack.paused()) {
			world.tick(context, resolution);
		}
		ui.tick(context, rm, resolution, info.mouse_pos);

		mark::renderer::render_info render_info;
		render_info.camera = world.camera();
		render_info.resolution = resolution;
		render_info.lights = std::move(context.lights);
		render_info.sprites = std::move(context.sprites);
		render_info.normals = std::move(context.normals);
		return render_info;
	};
	event_loop(event_loop_info);
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