﻿#pragma once
#include <command.h>
#include <mode_stack.h>
#include <slot.h>
#include <stdafx.h>
#include <ui/action_bar.h>
#include <ui/event.h>
#include <ui/tooltip.h>

namespace mark {
namespace ui {
class window;
class recycler;

class ui final
{
public:
	ui(resource::manager& rm, mode_stack& stack, world_stack& world_stack);
	/// Default destructor defined in the source file for enabling PIMPL
	~ui();
	/// Update UI state, render frames, etc.
	void update(update_context& context, vd resolution, vd mouse_pos_);
	/// Handle all events
	[[nodiscard]] auto command(world& world, const command::any& command)
		-> bool;
	/// Show tooltip at a specified screen position with supplied text
	void tooltip(vi32 pos, const std::string& text);
	/// Show tooltip at a specified world position with supplied text
	void tooltip(vd pos, const std::string& text);
	/// Get a grabbed item
	auto grabbed() noexcept -> interface::item*;
	/// Grab an item
	void drag(interface::container&, vi32 pos) noexcept;
	/// Drop grabbed item
	auto drop() noexcept -> interface::item_ptr;
	void recycle(interface::container&, vi32 pos) noexcept;
	/// Returns a modular, if a modular is present in the landing pad
	auto landed_modular() noexcept -> mark::unit::modular*;
	/// Returns true if module is present in the recycler
	auto in_recycler(const mark::interface::item& item) const noexcept -> bool;

private:
	/// Handler for the click event
	[[nodiscard]] auto click(vi32 screen_pos, bool shift) -> bool;
	/// Handler for the mouse over event
	[[nodiscard]] auto hover(vi32 screen_pos) -> bool;
	/// Process the move command
	auto command(world& world, const command::move& move) -> bool;
	/// Process the "drag" user command
	void drop(world& world, vd relative);
	/// Process the "drop" user command
	void drag(world& world, vd relative, bool shift);
	void container_ui(
		update_context& context,
		vd resolution,
		const unit::modular& modular);
	auto recycler() noexcept -> optional<mark::ui::recycler&>;
	auto recycler() const noexcept -> optional<const mark::ui::recycler&>;

	action_bar m_action_bar;

	const resource::image_ptr m_grid_bg;

	// Used to detect container change
	std::vector<ref<mark::module::cargo>> m_containers;

	std::vector<unique_ptr<window>> m_windows;

	// Used to detect stack state change
	mode m_mode = mode::world;
	mark::ui::tooltip m_tooltip;

	resource::manager& m_rm;
	mode_stack& m_stack;
	world_stack& m_world_stack;

	slot m_grabbed;
};
} // namespace ui
} // namespace mark
