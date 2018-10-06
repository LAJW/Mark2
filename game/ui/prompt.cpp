#include "prompt.h"
#include <exception.h>
#include <mode_stack.h>
#include <resource/manager.h>
#include <ui/action/clear_ui_state.h>
#include <ui/action/pop_ui_state.h>
#include <ui/chunky_button.h>
#include <ui/label.h>
#include <ui/window.h>

auto mark::ui::make_prompt(mark::resource::manager& rm) -> unique_ptr<window>
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
		_.text = "Quit to Desktop";
		_.font_size = 46;
		return _;
	}()))));
	auto play_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.text = "Yes";
		_.relative = true;
		_.on_click = [&](const event&) {
			return make_handler_result<action::clear_ui_state>();
		};
		return _;
	}());
	Ensures(success(menu->append(move(play_button))));
	auto cancel_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.text = "No";
		_.relative = true;
		_.on_click = [&](const event&) {
			return make_handler_result<action::pop_ui_state>();
		};
		return _;
	}());
	Ensures(success(menu->append(std::move(cancel_button))));
	return menu;
}
