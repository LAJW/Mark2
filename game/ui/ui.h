#pragma once
#include <array2d.h>
#include <command.h>
#include <interface/ui.h>
#include <mode_stack.h>
#include <ref.h>
#include <slot.h>
#include <stdafx.h>
#include <ui/action_bar.h>
#include <ui/event.h>
#include <ui/tooltip.h>

namespace mark {
namespace ui {
class window;
class recycler;

class ui final : public interface::ui
{
public:
	ui(resource::manager& rm,
	   random& random,
	   mode_stack& stack,
	   world_stack& world_stack);
	/// Default destructor defined in the source file for enabling PIMPL
	~ui();
	/// Update UI state, render frames, etc.
	void update(update_context& context, vd resolution, vd mouse_pos_);
	/// Handle all events
	[[nodiscard]] bool
	command(world& world, random& random, const command::any& command);
	/// Get a grabbed item
	[[nodiscard]] optional<const interface::item&> grabbed() const
		noexcept override;
	/// Returns a modular, if a modular is present in the landing pad
	[[nodiscard]] mark::unit::modular* landed_modular() noexcept;
	/// Returns true if module is present in the recycler
	[[nodiscard]] bool in_recycler(const mark::interface::item& item) const
		noexcept override;

private:
	using dispatch_callback =
		std::function<handler_result(const event&, window&)>;
	[[nodiscard]] bool
	dispatch(vi32 screen_pos, bool shift, dispatch_callback proc);
	/// Handler for the click event
	[[nodiscard]] auto click(vi32 screen_pos, bool shift) -> bool;
	/// Handler for the mouse over event
	[[nodiscard]] auto hover(vi32 screen_pos, vd world_pos) -> bool;
	/// Process the move command
	[[nodiscard]] bool
	command(ref<world> world, ref<random> random, const command::move& move);
	/// Process the "drag" user command
	void drop(ref<world> world, ref<random> random, vd relative);
	/// Process the "drop" user command
	void drag(ref<world> world, vd relative, bool shift);
	/// Process container UI frame
	void container_ui(
		ref<update_context> context,
		vd resolution,
		const unit::modular& modular);
	/// Get the recycler
	[[nodiscard]] optional<mark::ui::recycler&> recycler() noexcept;
	[[nodiscard]] optional<const mark::ui::recycler&> recycler() const noexcept;

	action_bar m_action_bar;

	// Root UI component
	unique_ptr<window> m_root;

	const resource::image_ptr m_grid_bg;

	// Used to detect container change
	std::vector<std::reference_wrapper<mark::module::cargo>> m_containers;

	// Used to detect stack state change
	mode m_mode = mode::world;
	mark::ui::tooltip m_tooltip;

	resource::manager& m_rm;
	random& m_random;
	mode_stack& m_stack;
	world_stack& m_world_stack;
	using queue_type = array2d<mark::slot, 16, 32>;
	slot m_grabbed;
	queue_type m_queue;
};
} // namespace ui
} // namespace mark
