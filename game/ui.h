﻿#pragma once
#include "stdafx.h"
#include "command.h"
#include "action_bar.h"
#include "ui_event.h"
#include "mode_stack.h"

namespace mark {
namespace ui {
class window;

class ui final {
public:
	explicit ui(resource::manager& rm, mode_stack& stack, world_stack& world_stack);
	~ui();
	void tick(
		tick_context& context,
		resource::manager& rm,
		vector<double> resolution,
		vector<double> mouse_pos_);
	[[nodiscard]] auto command(world& world, const command::any& command) -> bool;
	bool click(vector<int> screen_pos);
	bool hover(vector<int> screen_pos);
	void show_ship_editor(unit::modular&);
	void hide_ship_editor();
	void tooltip(vector<int> pos, const std::string& text);
	void tooltip(
		tick_context&,
		const std::string& text,
		vector<double> pos);
	void world_tooltip(
		tick_context&,
		const std::string& text,
		vector<double> pos);

	std::unique_ptr<module::base> grabbed;
	vector<int> grabbed_prev_pos;
	interface::container* grabbed_prev_parent = nullptr;
	std::vector<int8_t> grabbed_bind;
	// Release module held by UI
	void release();
private:
	void container_ui(
		tick_context& context,
		vector<double> resolution,
		const unit::landing_pad& landing_pad,
		const unit::modular& ship);

	action_bar m_action_bar;

	std::string m_tooltip_text;
	vector<double> m_tooltip_pso;

	const std::shared_ptr<const resource::image> m_font;
	const std::shared_ptr<const resource::image> m_tooltip_bg;
	const std::shared_ptr<const resource::image> m_grid_bg;

	// Used to detect container change
	std::vector<std::reference_wrapper<mark::module::cargo>> m_containers;

	std::vector<std::unique_ptr<window>> m_windows;

	resource::manager& m_rm;
	mode_stack& m_stack;
	world_stack& m_world_stack;
};

}
}

