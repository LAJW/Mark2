#include "ui.h"
#include <algorithm.h>
#include <algorithm/match.h>
#include <interface/has_bindings.h>
#include <module/base.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/impl/ui.h>
#include <ui/main_menu.h>
#include <ui/options.h>
#include <ui/prompt.h>
#include <ui/recycler.h>
#include <ui/ship_editor.h>
#include <ui/window.h>
#include <unit/landing_pad.h>
#include <unit/modular.h>
#include <update_context.h>
#include <vector.h>
#include <world.h>
#include <world_stack.h>

namespace mark {
namespace ui {

ui::ui(
	resource::manager& rm,
	random& random,
	mode_stack& stack,
	world_stack& world_stack)
	: m_action_bar(rm)
	, m_root(std::make_unique<window>())
	, m_tooltip(rm)
	, m_rm(rm)
	, m_random(random)
	, m_stack(stack)
	, m_world_stack(world_stack)
{}

ui::~ui() = default;

void ui::render_logo(ref<update_context> context) const
{
	context->sprites[100].push_back([&] {
		sprite _;
		_.centred = false;
		_.size = 256.f;
		_.pos = vi32(700, 300);
		_.frame = std::numeric_limits<size_t>::max();
		_.image = m_rm.image("mark-modular.png");
		return _;
	}());
}

const std::unordered_map<std::string, YAML::Node>& ui::blueprints() const
{
	return m_world_stack.blueprints();
}

const slot& ui::grabbed_raw() const { return m_grabbed; }

const ui::recycler_queue_type& ui::recycler_queue() const
{
	return m_recycler_queue;
}

[[nodiscard]] static std::unique_ptr<window> route(
	const mode mode,
	const ui& ui,
	resource::manager& rm,
	const vi32 resolution)
{
	switch (mode) {
	case mode::main_menu:
		return make_main_menu(rm);
	case mode::world:
		if (let modular = ui.landed_modular()) {
			return std::make_unique<ship_editor>([&] {
				ship_editor::info info;
				info.ui = ui;
				info.modular = modular;
				info.resource_manager = rm;
				info.resolution = resolution;
				info.relative = false;
				return info;
			}());
		}
		break;
	case mode::prompt:
		return make_prompt(rm);
	case mode::options:
		return make_options(rm);
	}
	return std::make_unique<window>();
}

bool ui::state_changed() const
{
	let& world = m_world_stack.world();
	let mode_changed = !m_stack.get().empty() && m_stack.get().back() != m_mode;
	let world_target_changed = &*world.target() != m_prev_world_target;
	return mode_changed || world_target_changed;
}

void ui::update_state()
{
	auto& world = m_world_stack.world();
	m_prev_world_target = &*world.target();
	if (!m_stack.get().empty()) {
		m_mode = m_stack.get().back();
	}
}

void ui::update(update_context& context, vd resolution, vd mouse_pos)
{
	if (state_changed()) {
		update_state();
		m_root = route(m_mode, *this, m_rm, vi32(resolution));
	}
	if (m_stack.get().back() == mode::world) {
		auto& world = m_world_stack.world();
		m_action_bar.update(world, context, resolution, mouse_pos);
	}
	if (m_stack.get().back() == mode::main_menu) {
		render_logo(ref(context));
	}
	m_root->update(context);
	m_tooltip.update(context);
}

void ui::execute(action::base& action)
{
	action::base::execute_info execute_info;
	execute_info.mode_stack = m_stack;
	// Can't inline into if-statement, the lifetime has to be extended
	let landed_modular = this->landed_modular();
	if (landed_modular) {
		execute_info.modular = *landed_modular;
	}
	execute_info.tooltip = m_tooltip;
	execute_info.queue = m_recycler_queue;
	execute_info.grabbed = m_grabbed;
	execute_info.random = m_random;
	action.execute(execute_info);
}

bool ui::execute(const handler_result& actions)
{
	if (actions) {
		for (let& action : *actions) {
			this->execute(*action);
		}
	}
	return actions.has_value();
}

/// Create an event for the root component
[[nodiscard]] static event root_event(const command::move& move)
{
	mark::ui::event event;
	event.absolute_cursor = move.screen_pos;
	event.cursor = move.screen_pos;
	event.world_cursor = move.to;
	event.shift = move.shift;
	return event;
}

[[nodiscard]] static event root_event(const command::guide& guide)
{
	mark::ui::event event;
	event.absolute_cursor = guide.screen_pos;
	event.cursor = guide.screen_pos;
	event.world_cursor = guide.pos;
	event.shift = false;
	return event;
}

bool ui::ui::command(const world&, const command::any& any)
{
	return match(
		any,
		[&](const command::cancel&) {
			m_stack.pop();
			return true;
		},
		[&](const command::guide& guide) {
			return execute(m_root->hover(root_event(guide)));
		},
		[&](const command::move& move) {
			if (move.release) {
				return false;
			}
			if (auto actions = m_root->click(root_event(move))) {
				return execute(std::move(*actions));
			}
			return false;
		},
		[&](const command::activate& activate) {
			if (grabbed()) {
				m_grabbed = {};
				return true;
			}
			if (let modular = this->landed_modular()) {
				modular->toggle_bind(
					activate.id, impl::pick_pos(activate.pos - modular->pos()));
				return true;
			}
			return false;
		},
		[&](const auto&) { return false; });
}

auto ui::grabbed() const noexcept -> optional<const interface::item&>
{
	if (m_grabbed.empty()) {
		return {};
	}
	return item_of(m_grabbed);
}

template <
	typename world_stack_type,
	typename modular_type =
		add_const_if_t<unit::modular, std::is_const_v<world_stack_type>>>
optional<modular_type&> landed_modular(world_stack_type& world_stack)
{
	let landing_pad = std::dynamic_pointer_cast<mark::unit::landing_pad>(
		world_stack.world().target());
	if (!landing_pad) {
		return {};
	}
	if (let modular = dynamic_cast<modular_type*>(landing_pad->ship().get())) {
		return *modular;
	}
	return {};
}

auto ui::landed_modular() noexcept -> optional<mark::unit::modular&>
{
	return mark::ui::landed_modular(m_world_stack);
}

auto ui::landed_modular() const noexcept -> optional<const mark::unit::modular&>
{
	return mark::ui::landed_modular(m_world_stack);
}

auto ui::in_recycler(const mark::interface::item& item) const noexcept -> bool
{
	if (let recycler = this->recycler()) {
		return recycler->has(item);
	}
	return false;
}

template <typename T>
[[nodiscard]] auto get_recycler(T& root)
{
	using value_type = add_const_if_t<mark::ui::recycler, std::is_const_v<T>>&;
	using return_type = optional<value_type>;
	let children = root.children();
	if (children.size() == 2) {
		return return_type(dynamic_cast<value_type>(children.back().get()));
	}
	return return_type();
}

optional<mark::ui::recycler&> ui::recycler() noexcept
{
	return get_recycler(*m_root);
}

optional<const mark::ui::recycler&> ui::recycler() const noexcept
{
	return get_recycler(*m_root);
}

} // namespace ui
} // namespace mark
