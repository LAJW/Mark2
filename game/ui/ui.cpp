#include "ui.h"
#include <algorithm.h>
#include <interface/has_bindings.h>
#include <module/base.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
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

mark::ui::ui::ui(
	resource::manager& rm,
	random& random,
	mode_stack& stack,
	world_stack& world_stack)
	: m_action_bar(rm)
	, m_grid_bg(rm.image("grid-background.png"))
	, m_tooltip(rm)
	, m_rm(rm)
	, m_random(random)
	, m_stack(stack)
	, m_world_stack(world_stack)
{
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
}

mark::ui::ui::~ui() = default;

void mark::ui::ui::update(update_context& context, vd resolution, vd mouse_pos_)
{
	let resolution_i = vi32(resolution);
	auto& world = m_world_stack.world();
	if (!m_stack.get().empty() && m_stack.get().back() != m_mode) {
		m_mode = m_stack.get().back();
		// router
		if (m_mode == mode::main_menu) {
			m_windows.front() = make_main_menu(m_rm, m_stack);
		} else if (m_mode == mode::world) {
			m_windows.front() =
				std::make_unique<mark::ui::window>(mark::ui::window::info());
		} else if (m_mode == mode::prompt) {
			m_windows.front() = make_prompt(m_rm, m_stack);
		} else if (m_mode == mode::options) {
			m_windows.front() = make_options(m_rm, m_stack);
		}
	}
	if (m_stack.get().back() == mode::world) {
		m_action_bar.update(world, context, resolution, mouse_pos_);
		let image_circle = m_rm.image("circle.png");
		let mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
		// Display landing pad UI
		if (let modular = this->landed_modular()) {
			if (m_windows.size() == 2) {
				let inventory_size =
					mark::vu32(16 * 16, (16 * 4 + 32) * container_count);
				m_windows.push_back(std::make_unique<mark::ui::inventory>([&] {
					inventory::info _;
					_.modular = *modular;
					_.rm = m_rm;
					_.ui = *this;
					_.pos = { 50, 50 };
					_.size = inventory_size;
					return _;
				}()));
				m_windows.push_back(std::make_unique<mark::ui::recycler>([&] {
					recycler::info _;
					_.modular = *modular;
					_.rm = m_rm;
					_.tooltip = m_tooltip;
					_.pos = { resolution_i.x - 50 - 300, 50 };
					_.size = inventory_size;
					_.ui = *this;
					return _;
				}()));
			} else {
				this->recycler()->pos(vi32(resolution_i.x - 50 - 300, 50));
			}
			this->container_ui(context, mouse_pos, *modular);
		} else {
			m_windows[1]->clear();
			if (m_windows.size() == 4) {
				m_windows.pop_back(); // Clear recycler UI
				m_windows.pop_back(); // Clear inventory UI
			}
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
	for (let& window : m_windows) {
		window->update(context);
	}
	m_tooltip.update(context);
}

bool mark::ui::ui::click(vi32 screen_pos, bool shift)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	event.shift = shift;
	for (auto &window : m_windows) {
		let result = window->click(event);
		if (result.handled) {
			return true;
		}
	}
	return false;
}

bool mark::ui::ui::hover(vi32 screen_pos)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	event.shift = false;
	for (auto &window : m_windows) {
		let result = window->hover(event);
		if (result.handled) {
			return true;
		}
	}
	return false;
}

namespace mark {
static auto modular(mark::world& world) -> shared_ptr<unit::modular>
{
	let target = world.target();
	let landing_pad = std::dynamic_pointer_cast<unit::landing_pad>(target);
	return landing_pad ? landing_pad->ship() : nullptr;
}
} // namespace mark

bool mark::ui::ui::command(
	world& world,
	random& random,
	const mark::command::any& any)
{
	if (std::holds_alternative<command::cancel>(any)) {
		m_stack.pop();
		return true;
	}
	if (let guide = std::get_if<command::guide>(&any)) {
		return this->hover(guide->screen_pos);
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
		let modular = mark::modular(world);
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
	}
	if (let move = std::get_if<command::move>(&any)) {
		return this->command(world, random, *move);
	}
	return false;
}

auto mark::ui::ui::command(
	world& world,
	random& random,
	const mark::command::move& move)
	-> bool
{
	if (move.release) {
		return false;
	}
	if (this->click(move.screen_pos, move.shift)) {
		return true;
	}
	if (!modular(world)) {
		return false;
	}
	let relative = (move.to - world.target()->pos()) / double(module::size);
	let module_pos = round(relative);
	if (!(std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17)) {
		return true;
	}
	// modular drag&drop
	if (this->grabbed()) {
		this->drop(world, random, relative);
	} else {
		this->drag(world, relative, move.shift);
	}
	return true;
}

void mark::ui::ui::drop(world& world, random& random, vd relative)
{
	Expects(grabbed());
	let module_pos = round(relative);
	let modular = mark::modular(world);
	// module's top-left corner
	let drop_pos = module_pos - vi32(grabbed()->size()) / 2;
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
			grabbed()->use_on(random, world.blueprints(), *module);
		if (error == error::code::success && consumed) {
			detach(m_grabbed);
		}
	}
}

void mark::ui::ui::drag(world& world, vd relative, bool shift)
{
	Expects(!grabbed());
	let pick_pos = floor(relative);
	let modular = mark::modular(world);
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

void mark::ui::ui::container_ui(
	update_context& context,
	vd mouse_pos,
	const unit::modular& modular)
{
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	let relative = (mouse_pos - modular.pos()) / double(module::size);
	let module_pos = round(relative);
	if (grabbed()) {
		let available = make_available_map(*grabbed(), modular);
		for (let offset : surface) {
			if (available
					[offset.x + grid_size / 2
					 + (offset.y + grid_size / 2) * grid_size]) {
				context.sprites[1].push_back([&] {
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
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			let size = static_cast<float>(
						   std::max(grabbed()->size().x, grabbed()->size().y))
				* module::size;
			let drop_pos = module_pos
				- vi32(grabbed()->size()) / 2; // module's top-left corner
			let color = modular.can_attach(drop_pos, *grabbed())
				? sf::Color::Green
				: sf::Color::Red;
			context.sprites[100].emplace_back([&] {
				sprite _;
				_.image = grabbed()->thumbnail();
				_.pos = vd(module_pos) * double(module::size) + modular.pos();
				_.size = size;
				_.color = color;
				return _;
			}());
		} else {
			let size = static_cast<float>(
						   std::max(grabbed()->size().x, grabbed()->size().y))
				* module::size;
			context.sprites[105].emplace_back([&] {
				sprite _;
				_.image = grabbed()->thumbnail();
				_.pos = mouse_pos;
				_.size = size;
				return _;
			}());
		}
	}

	// Display tooltips
	let pick_pos = floor(relative);
	if (!grabbed()) {
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// modular

			let module = modular.module_at(pick_pos);
			if (module) {
				let description = module->describe();
				let module_size =
					vd(module->size()) * static_cast<double>(module::size);
				let tooltip_pos =
					module->pos() + vd(module_size.x, -module_size.y) / 2.0;
				m_tooltip.set(tooltip_pos, &*module, description);
			}
		}
	}
}

void mark::ui::ui::tooltip(
	std::variant<vd, vi32> pos,
	const void* object_id,
	const std::string& str)
{
	m_tooltip.set(pos, &object_id, str);
}

auto mark::ui::ui::grabbed() noexcept -> interface::item*
{
	return !m_grabbed.empty() ? &item_of(m_grabbed) : nullptr;
}

void mark::ui::ui::drag(interface::container& container, vi32 pos) noexcept
{
	m_grabbed = { container, pos };
}

auto mark::ui::ui::drop() noexcept -> interface::item_ptr
{
	return detach(m_grabbed);
}

void mark::ui::ui::recycle(interface::container& container, vi32 pos) noexcept
{
	if (let recycler = this->recycler()) {
		recycler->recycle(container, pos);
	}
}

auto mark::ui::ui::landed_modular() noexcept -> mark::unit::modular*
{
	let landing_pad = std::dynamic_pointer_cast<mark::unit::landing_pad>(
		m_world_stack.world().target());
	if (!landing_pad) {
		return nullptr;
	}
	return dynamic_cast<mark::unit::modular*>(landing_pad->ship().get());
}

auto mark::ui::ui::in_recycler(const mark::interface::item& item) const noexcept
	-> bool
{
	if (let recycler = this->recycler()) {
		return recycler->has(item);
	}
	return false;
}

auto mark::ui::ui::recycler() noexcept -> optional<mark::ui::recycler&>
{
	if (m_windows.size() == 4) {
		return dynamic_cast<mark::ui::recycler&>(*m_windows.back());
	}
	return {};
}

auto mark::ui::ui::recycler() const noexcept
	-> optional<const mark::ui::recycler&>
{
	if (m_windows.size() == 4) {
		return dynamic_cast<mark::ui::recycler&>(*m_windows.back());
	}
	return {};
}
