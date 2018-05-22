#include "ui.h"
#include "button.h"
#include "container.h"
#include "window.h"
#include <algorithm.h>
#include <interface/has_bindings.h>
#include <item/shard.h>
#include <module/cargo.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/landing_pad.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>
#include <world_stack.h>

static auto make_main_menu(mark::resource::manager& rm, mark::mode_stack& stack)
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>([&] {
		window::info _;
		_.pos = { 300, 300 };
		return _;
	}());
	auto play_button = std::make_unique<button>([&] {
		button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.title = "Solitary Traveller";
		_.relative = true;
		return _;
	}());
	play_button->on_click.insert([&](let&) {
		stack.push(mode::world);
		return true;
	});
	menu->insert(move(play_button));
	auto quit_button = std::make_unique<button>([&] {
		button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.title = "Abandon Expedition";
		_.relative = true;
		return _;
	}());
	quit_button->on_click.insert([&](let&) {
		stack.push(mode::prompt);
		return true;
	});
	menu->insert(move(quit_button));
	return menu;
}

static auto make_prompt(mark::resource::manager& rm, mark::mode_stack& stack)
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>([&] {
		window::info _;
		_.pos = { 300, 300 };
		return _;
	}());
	auto play_button = std::make_unique<button>([&] {
		button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.title = "Yes";
		_.relative = true;
		return _;
	}());
	play_button->on_click.insert([&](let&) {
		stack.clear();
		return true;
	});
	menu->insert(move(play_button));
	auto cancel_button = std::make_unique<button>([&] {
		button::info _;
		_.size = { 250, 50 };
		_.font = rm.image("font.png");
		_.title = "No";
		_.relative = true;
		return _;
	}());
	cancel_button->on_click.insert([&](let&) {
		stack.pop();
		return true;
	});
	menu->insert(std::move(cancel_button));
	return menu;
}

mark::ui::ui::ui(
	resource::manager& rm,
	mode_stack& stack,
	world_stack& world_stack)
	: m_world_stack(world_stack)
	, m_stack(stack)
	, m_action_bar(rm)
	, m_grid_bg(rm.image("grid-background.png"))
	, m_rm(rm)
	, m_tooltip(rm)
{
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
}

mark::ui::ui::~ui() = default;

void mark::ui::ui::update(update_context& context, vd resolution, vd mouse_pos_)
{
	auto& world = m_world_stack.world();
	if (!m_stack.get().empty()) {
		if (m_stack.get().back() != m_mode) {
			m_mode = m_stack.get().back();
			// router
			if (m_mode == mode::main_menu) {
				m_windows.front() = make_main_menu(m_rm, m_stack);
			} else if (m_mode == mode::world) {
				m_windows.front() = std::make_unique<mark::ui::window>(
					mark::ui::window::info());
			} else if (m_mode == mode::prompt) {
				m_windows.front() = make_prompt(m_rm, m_stack);
			}
		}
	}
	m_action_bar.update(world, context, m_rm, resolution, mouse_pos_);
	let image_circle = m_rm.image("circle.png");
	for (let& window : m_windows) {
		window->update(context);
	}
	let mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
	// Display Mark Modular Logo
	if (m_mode == mode::main_menu) {
		context.sprites[100].push_back([&] {
			sprite _;
			_.centred = false;
			_.world = false;
			_.size = 256.f;
			_.pos = { 700., 300. };
			_.frame = std::numeric_limits<size_t>::max();
			_.image = m_rm.image("mark-modular.png");
			return _;
		}());
	}

	// Display landing pad UI
	if (auto landing_pad =
			std::dynamic_pointer_cast<unit::landing_pad>(world.target())) {
		if (let ship = landing_pad->ship()) {
			if (m_windows.size() == 2) {
				m_windows.push_back(std::make_unique<mark::ui::window>(
					mark::ui::window::info()));
				auto recycle_button = std::make_unique<button>([&] {
					button::info _;
					_.font = m_rm.image("font.png");
					_.pos = { 500, 500 };
					_.relative = false;
					_.size = { 150, 50 };
					_.title = "recycle";
					return _;
				}());
				recycle_button->on_click.insert([&](let&) {
					for (auto& slot : m_recycler_queue) {
						// TODO: Turn into a shard
						(void)detach(slot);
						if (let landing_pad =
								std::dynamic_pointer_cast<unit::landing_pad>(
									world.target())) {
							if (let ship = landing_pad->ship()) {
								// TODO: can_push
								(void)push(
									*ship, std::make_unique<item::shard>(m_rm));
							}
						}
					}
					m_recycler_queue.clear();
					return true;
				});
				m_windows.back()->insert(move(recycle_button));
				auto cancel_recycle_button = std::make_unique<button>([&] {
					button::info _;
					_.font = m_rm.image("font.png");
					_.pos = { 650, 500 };
					_.relative = false;
					_.size = { 150, 50 };
					_.title = "Cancel Recycling";
					return _;
				}());
				cancel_recycle_button->on_click.insert([&](let&) {
					m_recycler_queue.clear();
					return true;
				});
				m_windows.back()->insert(move(cancel_recycle_button));
			}

			this->container_ui(context, mouse_pos, *landing_pad, *ship);
			let containers = ship->containers();
			auto& window = m_windows[1];
			let[removed, added] =
				diff(window->children(), containers, [](let& a, let& b) {
					let& container =
						dynamic_cast<const mark::ui::container&>(*a.get());
					let item_count = count_if(b.get().items(), [](let& item) {
						return item.get() != nullptr;
					});
					return &container.cargo() == &b.get()
						&& container.children().size() == item_count;
				});
			for (let& it : removed) {
				window->children().erase(it);
			}
			for (let& pair : added) {
				auto& [it, container] = pair;
				auto container_window =
					std::make_unique<mark::ui::container>([&] {
						mark::ui::container::info _;
						_.rm = &m_rm;
						_.container = &container.get();
						_.ui = this;
						_.relative = true;
						return _;
					}());
				window->children().insert(it, move(container_window));
			}
		}
	} else {
		m_windows[1]->children().clear();
		if (m_windows.size() == 3) {
			m_windows.pop_back(); // Clear the UI
		}
		m_grabbed = {};
		m_recycler_queue.clear();
	}
	m_tooltip.update(context);
	for (let & [ i, slot ] : enumerate(m_recycler_queue)) {
		context.sprites[100].emplace_back([&] {
			let constexpr item_size = 4 * module::size;
			let& item = item_of(slot);
			let item_pos_x = static_cast<double>(i) * 1.5 * item_size;
			sprite _;
			_.image = item.thumbnail();
			_.pos = { resolution.x - item_size, item_pos_x };
			_.size = item_size;
			_.centred = false;
			_.world = false;
			return _;
		}());
	}
}

bool mark::ui::ui::click(vi32 screen_pos, bool shift)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	event.shift = shift;
	for (let& window : m_windows) {
		let handled = window->click(event);
		if (handled) {
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
	for (let& window : m_windows) {
		let handled = window->hover(event);
		if (handled) {
			return true;
		}
	}
	return false;
}

namespace mark {
static auto ship(mark::world& world) -> shared_ptr<unit::modular>
{
	let target = world.target();
	let landing_pad = std::dynamic_pointer_cast<unit::landing_pad>(target);
	return landing_pad ? landing_pad->ship() : nullptr;
}
} // namespace mark

bool mark::ui::ui::command(world& world, const mark::command::any& any)
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
		let ship = mark::ship(world);
		if (!ship) {
			return false;
		}
		if (grabbed()) {
			m_grabbed = {};
			return true;
		}
		let relative =
			(activate->pos - world.target()->pos()) / double(module::size);
		let pick_pos = floor(relative);
		ship->toggle_bind(activate->id, pick_pos);
		return true;
	}
	if (let move = std::get_if<command::move>(&any)) {
		return this->command(world, *move);
	}
	return false;
}

auto mark::ui::ui::command(world& world, const mark::command::move& move)
	-> bool
{
	if (move.release) {
		return false;
	}
	if (this->click(move.screen_pos, move.shift)) {
		return true;
	}
	if (!ship(world)) {
		return false;
	}
	let relative = (move.to - world.target()->pos()) / double(module::size);
	let module_pos = round(relative);
	if (!(std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17)) {
		return true;
	}
	// ship drag&drop
	if (this->grabbed()) {
		this->drop(world, relative);
	} else {
		this->drag(world, relative, move.shift);
	}
	return true;
}

void mark::ui::ui::drop(world& world, vd relative)
{
	Expects(grabbed());
	let module_pos = round(relative);
	let ship = mark::ship(world);
	// module's top-left corner
	let drop_pos = module_pos - vi32(grabbed()->size()) / 2;
	if (ship->can_attach(drop_pos, *grabbed())) {
		let grabbed_bind = ship->binding(m_grabbed.pos());
		Expects(!ship->attach(drop_pos, detach(m_grabbed)));
		for (let& bind : grabbed_bind) {
			ship->toggle_bind(bind, drop_pos);
		}
		return;
	}
	if (let module = ship->module_at(drop_pos)) {
		let[error, consumed] =
			grabbed()->use_on(m_rm, world.blueprints(), *module);
		if (error == error::code::success && consumed) {
			detach(m_grabbed);
		}
	}
}

void mark::ui::ui::drag(world& world, vd relative, bool shift)
{
	Expects(!grabbed());
	let pick_pos = floor(relative);
	let ship = mark::ship(world);
	let pos = ship->pos_at(pick_pos);
	if (!pos) {
		return;
	}
	let module = ship->module_at(pick_pos);
	Expects(module);
	if (!shift) {
		if (ship->can_detach(pick_pos)) {
			m_grabbed = { *ship, pick_pos };
		}
		return;
	}
	auto detached = ship->detach(pick_pos);
	if (!detached) {
		return;
	}
	if (error::code::success != push(*ship, move(detached))) {
		// It should be possible to reattach a module, if it was already
		// attached
		Expects(!ship->attach(pick_pos, move(detached)));
	}
}

static std::vector<bool> make_available_map(
	const mark::interface::item& item,
	const mark::unit::modular& ship)
{
	using namespace mark;
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	std::vector<bool> available(grid_size * grid_size, false);
	for (let top_left : surface) {
		if (ship.can_attach(top_left, item)) {
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
	const unit::landing_pad& landing_pad,
	const unit::modular& ship)
{
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	let relative = (mouse_pos - landing_pad.pos()) / double(module::size);
	let module_pos = round(relative);
	if (grabbed()) {
		let available = make_available_map(*grabbed(), ship);
		for (let offset : surface) {
			if (available
					[offset.x + grid_size / 2
					 + (offset.y + grid_size / 2) * grid_size]) {
				context.sprites[1].push_back([&] {
					sprite _;
					_.image = m_grid_bg;
					_.pos = landing_pad.pos()
						+ vd(offset) * double(module::size)
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
			let color = ship.can_attach(drop_pos, *grabbed()) ? sf::Color::Green
															  : sf::Color::Red;
			context.sprites[100].emplace_back([&] {
				sprite _;
				_.image = grabbed()->thumbnail();
				_.pos =
					vd(module_pos) * double(module::size) + landing_pad.pos();
				_.size = size;
				_.color = color;
				return _;
			}());
		} else {
			let size = static_cast<float>(
						   std::max(grabbed()->size().x, grabbed()->size().y))
				* module::size;
			context.sprites[100].emplace_back([&] {
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
			// ship

			let module = ship.module_at(pick_pos);
			if (module) {
				let description = module->describe();
				let module_size =
					vd(module->size()) * static_cast<double>(module::size);
				let tooltip_pos =
					module->pos() + vd(module_size.x, -module_size.y) / 2.0;
				m_tooltip.set(tooltip_pos, description);
			}
		}
	}
}

void mark::ui::ui::tooltip(vi32 pos, const std::string& str)
{
	m_tooltip.set(pos, str);
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
	if (all_of(m_recycler_queue, [&](let& slot) {
			return slot != mark::slot{ container, pos };
		})) {
		m_recycler_queue.emplace_back(container, pos);
	}
}
