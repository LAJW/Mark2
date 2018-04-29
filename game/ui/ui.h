﻿#pragma once
#include "action_bar.h"
#include "event.h"
#include <command.h>
#include <mode_stack.h>
#include <stdafx.h>

namespace mark {
namespace ui {
class window;

class ui final
{
public:
	explicit ui(
		resource::manager& rm,
		mode_stack& stack,
		world_stack& world_stack);
	~ui();
	void update(update_context& context, vd resolution, vd mouse_pos_);
	[[nodiscard]] auto command(world& world, const command::any& command)
		-> bool;
	bool click(vi32 screen_pos, bool shift);
	bool hover(vi32 screen_pos);
	void show_ship_editor(unit::modular&);
	void hide_ship_editor();
	void tooltip(vi32 pos, const std::string& text);
	void tooltip(update_context&, const std::string& text, vd pos);
	void world_tooltip(update_context&, const std::string& text, vd pos);

	interface::item_ptr grabbed;
	vi32 grabbed_prev_pos;
	interface::container* grabbed_prev_parent = nullptr;
	std::vector<int8_t> grabbed_bind;
	// Release module held by UI
	void release();

private:
	auto command(world& world, const command::move& move) -> bool;
	void drop(world& world, vd relative);
	void drag(world& world, vd relative, bool shift);
	void container_ui(
		update_context& context,
		vd resolution,
		const unit::landing_pad& landing_pad,
		const unit::modular& ship);

	action_bar m_action_bar;

	std::string m_tooltip_text;
	vd m_tooltip_pso;

	const resource::image_ptr m_font;
	const resource::image_ptr m_tooltip_bg;
	const resource::image_ptr m_grid_bg;

	// Used to detect container change
	std::vector<ref<mark::module::cargo>> m_containers;

	std::vector<unique_ptr<window>> m_windows;

	// Used to detect stack state change
	mode m_mode = mode::world;

	resource::manager& m_rm;
	mode_stack& m_stack;
	world_stack& m_world_stack;
};
} // namespace ui
} // namespace mark
