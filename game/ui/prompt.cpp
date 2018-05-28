#include "prompt.h"
#include <resource_manager.h>
#include <mode_stack.h>
#include <ui/chunky_button.h>
#include <ui/window.h>

auto mark::ui::make_prompt(mark::resource::manager& rm, mark::mode_stack& stack)
	-> unique_ptr<window>
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>([&] {
		window::info _;
		_.pos = { 250, 300 };
		return _;
	}());
	auto play_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-red-button.png");
		_.text = "Yes";
		_.relative = true;
		return _;
	}());
	play_button->on_click.insert([&](let&) {
		stack.clear();
		return true;
	});
	menu->insert(move(play_button));
	auto cancel_button = std::make_unique<chunky_button>([&] {
		chunky_button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.background = rm.image("chunky-blue-button.png");
		_.text = "No";
		_.relative = true;
		return _;
	}());
	cancel_button->on_click.insert([&](let&) {
		stack.pop();
		return true;
	});
	menu->insert(std::move(cancel_button));
	return menu;
}
