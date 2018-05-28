#include "main_menu.h"
#include <stdafx.h>
#include <resource_manager.h>
#include <mode_stack.h>
#include <ui/chunky_button.h>
#include <ui/window.h>
#include <sprite.h>

auto mark::ui::make_main_menu(mark::resource::manager& rm, mark::mode_stack& stack)
-> unique_ptr<window>
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>([&] {
		window::info _;
		_.pos = { 300, 300 };
		return _;
	}());
	auto play_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.text = "Solitary Traveller";
		_.relative = true;
		return _;
	}());
	play_button->on_click.insert([&](let&) {
		stack.push(mode::world);
		return true;
	});
	menu->insert(move(play_button));
	auto quit_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.text = "Abandon Expedition";
		_.relative = true;
		return _;
	}());
	quit_button->on_click.insert([&](let&) {
		stack.push(mode::prompt);
		return true;
	});
	menu->insert(move(quit_button));
	return menu;
}
