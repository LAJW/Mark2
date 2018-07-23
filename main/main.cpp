#include "algorithm.h"
#include "command.h"
#include "hid.h"
#include "renderer.h"
#include "resource_image.h"
#include "resource_manager.h"
#include "sprite.h"
#include "stdafx.h"
#include "ui/ui.h"
#include "unit/modular.h"
#include "update_context.h"
#include "world.h"
#include "world_stack.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <fstream>
#include <iostream>

namespace mark {
constexpr let window_title = "mark 2";
constexpr let save_path = "state.yml";
constexpr let options_path = "options.yml";
constexpr let blueprints_path = "blueprints";
}; // namespace mark

#ifdef WIN32

extern "C"
{
	// Enable dedicated graphics for NVIDIA
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	// Enable dedicated graphics for AMD Radeon
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#endif

#if !WIN32 && __has_include(<filesystem>)
#include <filesystem>
using namespace std::filesystem;
#else
#include <experimental/filesystem>
using namespace std::experimental;
#endif

void save_world(const mark::world& world, std::string filename)
{
	YAML::Emitter out;
	world.serialize(out);
	std::ofstream state(filename);
	state << out.c_str();
}

struct on_event_info
{
	sf::Event event;
	mark::vector<double> window_res;
	mark::vector<double> mouse_pos;
};

struct on_update_info
{
	double dt;
	mark::vector<double> window_res;
	mark::vector<double> mouse_pos;
};

struct event_loop_info
{
	std::string window_title;
	mark::vector<unsigned> res;
	std::function<void(on_event_info)> on_event;
	std::function<mark::renderer::render_info(on_update_info)> on_update;
	mark::mode_stack* stack;
};

void event_loop(event_loop_info& info)
{
	Expects(info.on_event);
	Expects(info.on_update);
	Expects(info.stack);
	sf::RenderWindow window({ info.res.x, info.res.y }, info.window_title);
	mark::renderer renderer(info.res);
	let& on_update = info.on_update;
	let& on_event = info.on_event;

	auto last = std::chrono::system_clock::now();
	while (window.isOpen()) {
		let now = std::chrono::system_clock::now();
		let delta_time =
			std::chrono::duration_cast<std::chrono::microseconds>(now - last);
		let dt = static_cast<double>(delta_time.count()) / 1000000.0;

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Resized) {
				let width = static_cast<float>(event.size.width);
				let height = static_cast<float>(event.size.height);
				window.setView(sf::View({ 0.f, 0.f, width, height }));
			}
			if (event.type == sf::Event::Closed) {
				window.close();
			} else {
				on_event([&] {
					on_event_info _;
					_.mouse_pos = mark::vd(sf::Mouse::getPosition(window));
					_.window_res = mark::vd(window.getSize());
					_.event = event;
					return _;
				}());
			}
		}
		if (info.stack->get().empty()) {
			break;
			window.close();
		}

		if (dt >= 1.0 / 60.0) {
			last = now;
			on_event([&] {
				on_event_info _;
				_.mouse_pos = mark::vd(sf::Mouse::getPosition(window));
				_.window_res = mark::vd(window.getSize());
				_.event.type = sf::Event::MouseMoved;
				return _;
			}());
			window.draw(renderer.render(on_update([&] {
				on_update_info _;
				_.dt = dt;
				_.mouse_pos = mark::vd(sf::Mouse::getPosition(window));
				_.window_res = mark::vd(window.getSize());
				return _;
			}())));
			window.display();
		}
	}
}

// Get map of all ship and module blueprints
static auto blueprints()
{
	std::unordered_map<std::string, YAML::Node> result;
	for (let& dir : filesystem::directory_iterator(mark::blueprints_path)) {
		let path = dir.path().string();
		let basename = dir.path().stem().string();
		result[basename] = YAML::LoadFile(path);
		result[basename]["blueprint_id"] = basename;
	}
	return result;
}

namespace mark {
void main(std::vector<std::string> args);
}

void mark::main(std::vector<std::string> args)
{
	mode_stack stack;
	resource::manager_impl rm;
	random_impl random;
	mark::world_stack world_stack(
		YAML::LoadFile(mark::save_path), rm, random, blueprints());
	mark::ui::ui ui(rm, random, stack, world_stack);
	let options = YAML::LoadFile(options_path);
	mark::hid hid(options["keybindings"]);
	event_loop_info event_loop_info;
	event_loop_info.window_title = window_title;
	event_loop_info.stack = &stack;
	event_loop_info.res = { 1920, 1080 };
	event_loop_info.on_event = [&](const on_event_info& info) {
		hid.handle(info.event);
		auto& world = world_stack.world();
		let target = world.camera() + info.mouse_pos - info.window_res / 2.;
		for (let command : hid.commands(round(info.mouse_pos), target)) {
			if (!ui.command(world, random, command) && !stack.paused()) {
				world.command(command);
			}
		}
	};
	event_loop_info.on_update = [&](const on_update_info& info) {
		auto& world = world_stack.world();
		let resolution = info.window_res;
		mark::update_context context(rm, random);
		context.dt = info.dt;
		if (!stack.paused()) {
			world.update(context, resolution);
		}
		ui.update(context, resolution, info.mouse_pos);

		mark::renderer::render_info render_info;
		render_info.camera = world.camera();
		render_info.resolution = resolution;
		render_info.lights = std::move(context.lights);
		render_info.sprites = std::move(context.sprites);
		render_info.normals = std::move(context.normals);
		return render_info;
	};
	event_loop(event_loop_info);
	save_world(world_stack.world(), save_path);
}

int main(const int argc, const char* argv[])
{
		mark::main({ argv, argv + argc });
		return 0;
}
