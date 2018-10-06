#include "main_menu.h"
#include <exception.h>
#include <mode_stack.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/push_ui_state.h>
#include <ui/chunky_button.h>
#include <ui/label.h>
#include <ui/window.h>

namespace mark {
namespace ui {
auto mark::ui::make_main_menu(mark::resource::manager& rm) -> unique_ptr<window>
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
		_.on_click = [&](const event&) {
			return make_handler_result<action::push_ui_state>(mode::world);
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
			return make_handler_result<action::push_ui_state>(mode::options);
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
			return make_handler_result<action::push_ui_state>(mode::prompt);
		};
		return _;
	}()))));
	return menu;
}
} // namespace ui
} // namespace mark
