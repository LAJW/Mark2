#include "ui.h"
#include <algorithm.h>
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
}

bool ui::dispatch(vi32 screen_pos, bool shift, dispatch_callback proc)
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
	event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	event.shift = shift;
	let result = proc(event, *m_root);
	for (auto& action : result.actions) {
		action->execute(execute_info);
	}
	return result.handled;
}

bool ui::ui::click(vi32 screen_pos, bool shift)
{
	return dispatch(screen_pos, shift, [&](const event& event, window& window) {
		return window.click(event);
	});
}

bool ui::ui::hover(vi32 screen_pos, vd world_pos)
{
	return dispatch(screen_pos, false, [&](const event& event, window& window) {
		// Display tooltips
		let modular = landed_modular();
		if (modular && !grabbed()) {
			let pick_pos = impl::pick_pos(world_pos - modular->pos());
			if (std::abs(pick_pos.x) <= 17 && std::abs(pick_pos.y) <= 17) {
				if (let module = modular->module_at(pick_pos)) {
					let description = module->describe();
					let module_size =
						vd(module->size()) * static_cast<double>(module::size);
					let tooltip_pos =
						module->pos() + vd(module_size.x, -module_size.y) / 2.0;
					return handler_result::make(
						std::make_unique<action::set_tooltip>(
							tooltip_pos, &*module, description));
				}
			}
		}
		return window.hover(event);
	});
}

bool ui::ui::command(world& world, random& random, const command::any& any)
{
	if (std::holds_alternative<command::cancel>(any)) {
		m_stack.pop();
		return true;
	}
	if (let guide = std::get_if<command::guide>(&any)) {
		return this->hover(guide->screen_pos, guide->pos);
	}
	if (this->m_stack.paused()) {
		if (auto move = std::get_if<command::move>(&any)) {
			if (!move->release) {
				return this->click(move->screen_pos, move->shift);
			}
			return false;
		}
	}
	if (let activate = std::get_if<command::activate>(&any)) {
		let modular = mark::ui::modular(ref(world));
		if (!modular) {
			return false;
		}
		if (grabbed()) {
			m_grabbed = {};
			return true;
		}
		let relative =
			(activate->pos - world.target()->pos()) / double(module::size);
		let pick_pos = floor(relative);
		modular->toggle_bind(activate->id, pick_pos);
		return true;
	} else if (let move = std::get_if<command::move>(&any)) {
		return this->command(ref(world), ref(random), *move);
	} else {
		return false;
	}
}

auto ui::command(
	ref<world> world,
	ref<random> random,
	const mark::command::move& move) -> bool
{
	if (move.release) {
		return false;
	}
	if (this->click(move.screen_pos, move.shift)) {
		return true;
	}
	if (!modular(ref(world))) {
		return false;
	}
	let relative = (move.to - world->target()->pos()) / double(module::size);
	let module_pos = round(relative);
	if (!(std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17)) {
		return true;
	}
	// modular drag&drop
	if (this->grabbed()) {
		this->drop(ref(world), ref(random), move.to - world->target()->pos());
	} else {
		this->drag(ref(world), relative, move.shift);
	}
	return true;
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
	let pick_pos = floor(relative);
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
	} else {
		if (modular->can_detach(pick_pos)) {
			m_grabbed = { *modular, pick_pos };
		}
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

void ui::container_ui(
	ref<update_context> context,
	const vd mouse_pos,
	const unit::modular& modular)
{
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	if (grabbed()) {
		let available = make_available_map(*grabbed(), modular);
		for (let offset : surface) {
			if (available
					[offset.x + grid_size / 2
					 + (offset.y + grid_size / 2) * grid_size]) {
				context->sprites[1].push_back([&] {
					sprite _;
					_.image = m_grid_bg;
					_.pos = modular.pos() + vd(offset) * double(module::size)
						+ vd(module::size, module::size) / 2.;
					_.size = module::size;
					_.color = { 0, 255, 255, 255 };
					return _;
				}());
			}
		}
		let drop_pos =
			impl::drop_pos(mouse_pos - modular.pos(), grabbed()->size());
		if (std::abs(drop_pos.x) <= 17 && std::abs(drop_pos.y) <= 17) {
			let size = static_cast<float>(
						   std::max(grabbed()->size().x, grabbed()->size().y))
				* module::size;
			let color = modular.can_attach(drop_pos, *grabbed())
				? sf::Color::Green
				: sf::Color::Red;
			context->sprites[100].emplace_back([&] {
				sprite _;
				_.image = grabbed()->thumbnail();
				_.pos = modular.pos() + vd(drop_pos) * 16.;
				_.size = size;
				_.color = color;
				_.centred = false;
				return _;
			}());
		} else {
			let size = static_cast<float>(
						   std::max(grabbed()->size().x, grabbed()->size().y))
				* module::size;
			context->sprites[105].emplace_back([&] {
				sprite _;
				_.image = grabbed()->thumbnail();
				_.pos = mouse_pos;
				_.size = size;
				return _;
			}());
		}
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
