#include "ui.h"
#include <algorithm.h>
#include <algorithm/match.h>
#include <interface/has_bindings.h>
#include <module/base.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <ui/action/set_tooltip.h>
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

static auto modular(ref<world> world) -> shared_ptr<unit::modular>
{
	let target = world->target();
	let landing_pad = std::dynamic_pointer_cast<unit::landing_pad>(target);
	return landing_pad ? landing_pad->ship() : nullptr;
}

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

void ui::update(update_context& context, vd resolution, vd mouse_pos_)
{
	let resolution_i = vi32(resolution);
	auto& world = m_world_stack.world();
	if (!m_stack.get().empty() && m_stack.get().back() != m_mode) {
		m_mode = m_stack.get().back();
		// router
		if (m_mode == mode::main_menu) {
			m_root = make_main_menu(m_rm);
		} else if (m_mode == mode::world) {
			m_root = std::make_unique<window>();
		} else if (m_mode == mode::prompt) {
			m_root = make_prompt(m_rm);
		} else if (m_mode == mode::options) {
			m_root = make_options(m_rm);
		}
	}
	if (m_stack.get().back() == mode::world) {
		m_action_bar.update(world, context, resolution, mouse_pos_);
		let image_circle = m_rm.image("circle.png");
		let mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
		// Display landing pad UI
		if (let modular = this->landed_modular()) {
			if (let recycler = this->recycler()) {
				// TODO: Allow aligning windows to the right/bottom of the
				// screen
				this->recycler()->pos({ resolution_i.x - 50 - 300, 50 });
			} else {
				m_root = std::make_unique<window>();
				let inventory_size =
					vu32(16 * 16, (16 * 4 + 32) * container_count);
				Ensures(success(m_root->append(std::make_unique<inventory>([&] {
					inventory::info _;
					_.modular = *modular;
					_.rm = m_rm;
					_.ui = *this;
					_.pos = { 50, 50 };
					_.size = inventory_size;
					return _;
				}()))));
				Ensures(success(
					m_root->append(std::make_unique<mark::ui::recycler>([&] {
						recycler::info _;
						_.rm = m_rm;
						_.pos = { resolution_i.x - 50 - 300, 50 };
						_.size = inventory_size;
						_.ui = *this;
						_.queue = m_queue;
						return _;
					}()))));
			}
			this->container_ui(ref(context), mouse_pos, *modular);
		} else {
			m_root->clear();
			m_grabbed = {};
		}
	}
	if (m_stack.get().back() == mode::main_menu) {
		context.sprites[100].push_back([&] {
			sprite _;
			_.centred = false;
			_.size = 256.f;
			_.pos = vi32(700, 300);
			_.frame = std::numeric_limits<size_t>::max();
			_.image = m_rm.image("mark-modular.png");
			return _;
		}());
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
	execute_info.queue = m_queue;
	execute_info.grabbed = m_grabbed;
	action.execute(execute_info);
}

bool ui::execute(const handler_result& actions)
{
	if (actions) {
		for (let &action : *actions) {
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

bool ui::ui::command(world& world, random& random, const command::any& any)
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
			return execute(click(
				ref(world), ref(random), move.screen_pos, move.to, move.shift));
		},
		[&](const command::activate& activate) {
			if (grabbed()) {
				m_grabbed = {};
				return true;
			}
			if (let modular = mark::ui::modular(ref(world))) {
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
	ref<world> world,
	ref<random> random,
	const vi32 screen_pos,
	const vd world_pos,
	const bool shift)
{
	if (auto actions = m_root->click(root_event(screen_pos, shift))) {
		return actions;
	} else if (!modular(ref(world))) {
		return {};
	}
	let relative = world_pos - world->target()->pos();
	if (!inside_modular_grid(round(relative / double(module::size)), {})) {
		return {};
	}
	if (this->grabbed()) {
		this->drop(ref(world), ref(random), relative);
	} else {
		this->drag(ref(world), relative, shift);
	}
	return handled();
}

void ui::drop(ref<world> world, ref<random> random, const vd relative)
{
	Expects(grabbed());
	let modular = mark::ui::modular(ref(world));
	// module's top-left corner
	let drop_pos = impl::drop_pos(relative, grabbed()->size());
	if (modular->can_attach(drop_pos, *grabbed())) {
		let grabbed_bind = modular->binding(m_grabbed.pos());
		Expects(!modular->attach(drop_pos, detach(m_grabbed)));
		for (let& bind : grabbed_bind) {
			modular->toggle_bind(bind, drop_pos);
		}
		return;
	}
	if (let module = modular->module_at(drop_pos)) {
		let[error, consumed] =
			item_of(m_grabbed).use_on(*random, world->blueprints(), *module);
		if (error == error::code::success && consumed) {
			detach(m_grabbed);
		}
	}
}

void ui::drag(ref<world> world, const vd relative, const bool shift)
{
	Expects(!grabbed());
	let pick_pos = floor(relative / static_cast<double>(module::size));
	let modular = mark::ui::modular(ref(world));
	let pos = modular->pos_at(pick_pos);
	if (!pos) {
		return;
	}
	if (shift) {
		if (auto detached = modular->detach(pick_pos)) {
			if (failure(push(*modular, move(detached)))) {
				// It should be possible to reattach a module, if it was already
				// attached
				Expects(success(modular->attach(*pos, move(detached))));
			}
		}
	} else if (modular->can_detach(pick_pos)) {
		m_grabbed = { *modular, pick_pos };
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

auto ui::landed_modular() noexcept -> mark::unit::modular*
{
	let landing_pad = std::dynamic_pointer_cast<mark::unit::landing_pad>(
		m_world_stack.world().target());
	if (!landing_pad) {
		return nullptr;
	}
	return dynamic_cast<mark::unit::modular*>(landing_pad->ship().get());
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
