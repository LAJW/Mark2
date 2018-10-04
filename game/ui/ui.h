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
	using recycler_queue_type = array2d<mark::slot, 16, 32>;

	ui(resource::manager& rm,
	   random& random,
	   mode_stack& stack,
	   world_stack& world_stack);
	/// Default destructor defined in the source file for enabling PIMPL
	~ui();
	/// Update UI state, render frames, etc.
	void update(update_context& context, vd resolution, vd mouse_pos_);
	/// Handle all events
	[[nodiscard]] bool command(const command::any& command);
	/// Get a grabbed item
	[[nodiscard]] optional<const interface::item&> grabbed() const
		noexcept override;
	/// Returns a modular, if a modular is present in the landing pad
	[[nodiscard]] optional<mark::unit::modular&> landed_modular() noexcept;
	[[nodiscard]] optional<const mark::unit::modular&> landed_modular() const
		noexcept;
	/// Returns true if module is present in the recycler
	[[nodiscard]] bool in_recycler(const mark::interface::item& item) const
		noexcept override;
	[[nodiscard]] const recycler_queue_type& recycler_queue() const;
	[[nodiscard]] const slot& grabbed_raw() const;
	[[nodiscard]] const std::unordered_map<std::string, YAML::Node>&
	blueprints() const;

private:
	void execute(action::base& action);
	[[nodiscard]] bool execute(const handler_result&);
	using dispatch_callback =
		std::function<handler_result(const event&, window&)>;
	/// Get the recycler
	[[nodiscard]] optional<mark::ui::recycler&> recycler() noexcept;
	[[nodiscard]] optional<const mark::ui::recycler&> recycler() const noexcept;
	void render_logo(ref<update_context> context) const;
	void update_state();
	[[nodiscard]] bool state_changed() const;

	action_bar m_action_bar;

	// Root UI component
	unique_ptr<window> m_root;

	// Used to detect container change
	std::vector<std::reference_wrapper<mark::module::cargo>> m_containers;

	// Used to detect stack state change
	mode m_mode = mode::world;
	mark::ui::tooltip m_tooltip;

	resource::manager& m_rm;
	random& m_random;
	mode_stack& m_stack;
	world_stack& m_world_stack;
	slot m_grabbed;
	recycler_queue_type m_recycler_queue;
	const unit::base* m_prev_world_target = nullptr;
};
} // namespace ui
} // namespace mark
