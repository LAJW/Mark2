#include "ui.h"
#include <algorithm.h>
#include <algorithm/match.h>
#include <interface/has_bindings.h>
#include <module/base.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/drop_into_modular.h>
#include <ui/action/grab_from_modular.h>
#include <ui/action/quick_detach.h>
#include <ui/action/set_tooltip.h>
#include <ui/action/use_grabbed_item.h>
#include <ui/impl/ui.h>
#include <ui/inventory.h>
#include <ui/main_menu.h>
#include <ui/options.h>
#include <ui/prompt.h>
#include <ui/recycler.h>
#include <ui/window.h>
#include <unit/landing_pad.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>
#include <world_stack.h>

let container_count = 8;

namespace mark {
namespace ui {

ui::ui(
	resource::manager& rm,
	random& random,
	mode_stack& stack,
	world_stack& world_stack)
	: m_action_bar(rm)
	, m_root(std::make_unique<window>())
	, m_grid_bg(rm.image("grid-background.png"))
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

/// Create game overlay inventory menu with cargo container management and the
/// recycler
[[nodiscard]] static std::unique_ptr<mark::ui::window> make_inventory_menu(
	const ui& ui,
	resource::manager& rm,
	const unit::modular& modular,
	const vi32 resolution)
{
	auto inventory = std::make_unique<window>();
	let inventory_size = vu32(16 * 16, (16 * 4 + 32) * container_count);
	Expects(
		success(inventory->append(std::make_unique<mark::ui::inventory>([&] {
			inventory::info _;
			_.modular = modular;
			_.rm = rm;
			_.ui = ui;
			_.pos = { 50, 50 };
			_.size = inventory_size;
			return _;
		}()))));
	Expects(success(inventory->append(std::make_unique<mark::ui::recycler>([&] {
		recycler::info _;
		_.rm = rm;
		_.pos = { resolution.x - 50 - 300, 50 };
		_.size = inventory_size;
		_.ui = ui;
		return _;
	}()))));
	return inventory;
}

const ui::recycler_queue_type& ui::recycler_queue() const
{
	return m_recycler_queue;
}

[[nodiscard]] static std::unique_ptr<window> route(
	const mode mode,
	const ui& ui,
	resource::manager& rm,
	optional<const unit::modular&> modular,
	const vi32 resolution)
{
	switch (mode) {
	case mode::main_menu:
		return make_main_menu(rm);
	case mode::world:
		if (modular) {
			return make_inventory_menu(ui, rm, *modular, resolution);
		}
		break;
	case mode::prompt:
		return make_prompt(rm);
	case mode::options:
		return make_options(rm);
	}
	return std::make_unique<window>();
}

void ui::update_recycler_position(const vi32 resolution)
{
	if (let recycler = this->recycler()) {
		// TODO: Allow aligning windows to the right/bottom of the
		// screen
		this->recycler()->pos({ resolution.x - 50 - 300, 50 });
	}
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

[[nodiscard]] static vd
screen_to_world(vd camera_pos, vd resolution, vd screen_pos)
{
	return camera_pos + screen_pos - resolution / 2.;
}

void ui::update(update_context& context, vd resolution, vd mouse_pos)
{
	if (state_changed()) {
		update_state();
		m_root = route(
			m_mode,
			*this,
			m_rm,
			this->landed_modular(),
			vi32(resolution));
	}
	if (m_stack.get().back() == mode::world) {
		auto& world = m_world_stack.world();
		let world_mouse_pos =
			screen_to_world(world.camera(), resolution, mouse_pos);
		m_action_bar.update(world, context, resolution, world_mouse_pos);
		if (let modular = this->landed_modular()) {
			this->container_ui(ref(context), mouse_pos, *modular);
		}
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
[[nodiscard]] static event root_event(vi32 screen_pos, bool shift)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	event.shift = shift;
	return event;
}

/// Calculate tooltip's world position for a module
[[nodiscard]] static vd tooltip_pos(const mark::module::base& module)
{
	let module_size = vd(module.size()) * static_cast<double>(module::size);
	return module.pos() + vd(module_size.x, -module_size.y) / 2.0;
}

[[nodiscard]] static optional<handler_result>
modular_tooltip(vd world_pos, const unit::modular& modular)
{
	let pick_pos = impl::pick_pos(world_pos - modular.pos());
	if (let module = modular.module_at(pick_pos)) {
		return make_handler_result<action::set_tooltip>(
			tooltip_pos(*module), &*module, module->describe());
	}
	return {};
}

handler_result ui::ui::hover(vi32 screen_pos, vd world_pos)
{
	if (let modular = landed_modular()) {
		if (!grabbed() && !m_stack.paused()) {
			if (auto action = modular_tooltip(world_pos, *modular)) {
				return { std::move(*action) };
			}
		}
	}
	return m_root->hover(root_event(screen_pos, false));
}

bool ui::ui::command(const world& world, const command::any& any)
{
	return match(
		any,
		[&](const command::cancel&) {
			m_stack.pop();
			return true;
		},
		[&](const command::guide& guide) {
			return execute(hover(guide.screen_pos, guide.pos));
		},
		[&](const command::move& move) {
			if (move.release) {
				return false;
			}
			return execute(click(world, move.screen_pos, move.to, move.shift));
		},
		[&](const command::activate& activate) {
			if (grabbed()) {
				m_grabbed = {};
				return true;
			}
			if (let modular = this->landed_modular()) {
				let pick_pos = floor(
					(activate.pos - world.target()->pos())
					/ static_cast<double>(module::size));
				modular->toggle_bind(activate.id, pick_pos);
				return true;
			}
			return false;
		},
		[&](const auto&) { return false; });
}

[[nodiscard]] static bool
inside_modular_grid(vi32 module_pos, vu32 umodule_size)
{
	let half_size = gsl::narrow<int>(unit::modular::max_size / 2);
	let module_size = vi32(umodule_size);
	return module_pos.x >= -half_size + module_size.x / 2
		&& module_pos.x <= half_size
		&& module_pos.y >= -half_size + module_size.y / 2
		&& module_pos.y <= half_size;
}

handler_result ui::click(
	const world& world,
	const vi32 screen_pos,
	const vd world_pos,
	const bool shift)
{
	if (auto actions = m_root->click(root_event(screen_pos, shift))) {
		return actions;
	} else if (!landed_modular()) {
		return {};
	}
	let relative = world_pos - world.target()->pos();
	if (!inside_modular_grid(round(relative / double(module::size)), {})) {
		return {};
	}
	return this->grabbed() ? this->drop(relative) : this->drag(relative, shift);
}

handler_result ui::drop(const vd relative) const
{
	Expects(grabbed());
	let modular = this->landed_modular();
	let drop_pos = impl::drop_pos(relative, grabbed()->size());
	if (modular->can_attach(drop_pos, *grabbed())) {
		auto bindings = (&m_grabbed.container() == &*modular)
			? modular->binding(m_grabbed.pos())
			: std::vector<int8_t>();
		return make_handler_result<action::drop_into_modular>(
			drop_pos, move(bindings));
	} else if (modular->module_at(drop_pos)) {
		return make_handler_result<action::use_grabbed_item>(
			drop_pos, m_world_stack.blueprints());
	}
	return handled();
}

handler_result ui::drag(const vd relative, const bool shift) const
{
	Expects(!grabbed());
	let pick_pos = floor(relative / static_cast<double>(module::size));
	let modular = this->landed_modular();
	Expects(modular);
	if (!modular->module_at(pick_pos)) {
		return {};
	} else if (shift) {
		return make_handler_result<action::quick_detach>(pick_pos);
	} else if (modular->can_detach(pick_pos)) {
		return make_handler_result<action::grab_from_modular>(pick_pos);
	} else {
		return {};
	}
}

static std::vector<bool> make_available_map(
	const mark::interface::item& item,
	const mark::unit::modular& modular)
{
	using namespace mark;
	let grid_size = gsl::narrow<int>(unit::modular::max_size);
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	std::vector<bool> available(grid_size * grid_size, false);
	for (let top_left : surface) {
		if (modular.can_attach(top_left, item)) {
			for (let relative : range(item.size())) {
				let pos = top_left + vi32(grid_size / 2, grid_size / 2)
					+ vi32(relative);
				if (pos.x < grid_size && pos.y < grid_size) {
					available[pos.x + pos.y * grid_size] = true;
				}
			}
		}
	}
	return available;
}

[[nodiscard]] static sprite draw_grabbed_over_modular(
	const interface::item& grabbed,
	const unit::modular& modular,
	const vi32& drop_pos)
{
	sprite _;
	_.image = grabbed.thumbnail();
	_.pos = modular.pos() + vd(drop_pos) * 16.
		+ vd(grabbed.size().x / 2., grabbed.size().y / 2.) * 16.;
	_.size = static_cast<float>(std::max(grabbed.size().x, grabbed.size().y))
		* module::size;
	_.color = modular.can_attach(drop_pos, grabbed) ? sf::Color::Green
													: sf::Color::Red;
	_.centred = true;
	return _;
}

[[nodiscard]] static sprite
draw_grabbed_over_void(const interface::item& grabbed, const vd& mouse_pos)
{
	sprite _;
	_.image = grabbed.thumbnail();
	_.pos = mouse_pos;
	_.size = static_cast<float>(std::max(grabbed.size().x, grabbed.size().y))
		* module::size;
	return _;
}

[[nodiscard]] static sprite draw_grabbed(
	const interface::item& grabbed,
	const unit::modular& modular,
	const vd& mouse_pos)
{
	let drop_pos = impl::drop_pos(mouse_pos - modular.pos(), grabbed.size());
	return inside_modular_grid(drop_pos, grabbed.size())
		? draw_grabbed_over_modular(grabbed, modular, drop_pos)
		: draw_grabbed_over_void(grabbed, mouse_pos);
}

void draw_grid_background(
	ref<std::vector<renderable>> sprites,
	const interface::item& grabbed,
	const mark::unit::modular& modular,
	const resource::image_ptr& tile_image)
{
	let available = make_available_map(grabbed, modular);
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	for (let offset : surface) {
		if (available
				[offset.x + grid_size / 2
				 + (offset.y + grid_size / 2) * grid_size]) {
			sprites->push_back([&] {
				sprite _;
				_.image = tile_image;
				_.pos = modular.pos() + vd(offset) * double(module::size)
					+ vd(module::size, module::size) / 2.;
				_.size = module::size;
				_.color = { 0, 255, 255, 255 };
				return _;
			}());
		}
	}
}

void ui::container_ui(
	ref<update_context> context,
	const vd mouse_pos,
	const unit::modular& modular)
{
	if (let grabbed = this->grabbed()) {
		draw_grid_background(
			ref(context->sprites[1]), *grabbed, modular, m_grid_bg);
		context->sprites[105].push_back(
			draw_grabbed(*grabbed, modular, mouse_pos));
	}
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
