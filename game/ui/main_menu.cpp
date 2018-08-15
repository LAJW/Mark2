#include "main_menu.h"
#include <mode_stack.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/chunky_button.h>
#include <ui/label.h>
#include <ui/window.h>
#include <exception.h>

auto mark::ui::make_main_menu(
	mark::resource::manager& rm,
	mark::mode_stack& stack) -> unique_ptr<window>
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>([&] {
		window::info _;
		_.pos = { 300, 300 };
		return _;
	}());
	Ensures(success(menu->append(std::make_unique<label>([&] {
		label::info _;
		_.pos = { 0, -100 };
		_.size = { 600, 300 };
		_.font = rm.image("font.png");
		_.text = "Main Menu";
		_.font_size = 46;
		return _;
	}()))));
	Ensures(success(menu->append(std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 300, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.text = "Solitary Traveller";
		_.relative = true;
		_.on_click = [&](let&) {
			stack.push(mode::world);
			return true;
		};
		return _;
	}()))));
	Ensures(success(menu->append(std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 300, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.text = "Configuration of preferences";
		_.relative = true;
		_.on_click = [&](let&) {
			stack.push(mode::options);
			return true;
		};
		return _;
	}()))));
	Ensures(success(menu->append(std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 300, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.text = "Abandon Expedition";
		_.relative = true;
		_.on_click = [&](let&) {
			stack.push(mode::prompt);
			return true;
		};
		return _;
	}()))));
	return menu;
}
