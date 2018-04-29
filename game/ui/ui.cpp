#include "ui.h"
#include "button.h"
#include "container.h"
#include "window.h"
#include <algorithm.h>
#include <interface/has_bindings.h>
#include <module/cargo.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/landing_pad.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>
#include <world_stack.h>

constexpr let tooltip_size = 300.f;
constexpr let tooltip_margin = 7.f;
constexpr let font_size = 14.f;

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
	, m_font(rm.image("font.png"))
	, m_tooltip_bg(rm.image("white.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_rm(rm)
{
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
	m_windows.push_back(
		std::make_unique<mark::ui::window>(mark::ui::window::info()));
}

mark::ui::ui::~ui() = default;

void mark::ui::ui::update(
	update_context& context,
	vd resolution,
	vd mouse_pos_)
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

	// Display landing pad UI
	if (auto landing_pad =
			std::dynamic_pointer_cast<unit::landing_pad>(world.target())) {
		if (let ship = landing_pad->ship()) {
			this->container_ui(context, mouse_pos, *landing_pad, *ship);
			let containers = ship->containers();
			auto& window = m_windows.back();
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
		m_windows.back()->children().clear();
	}
	if (!m_tooltip_text.empty()) {
		this->tooltip(context, m_tooltip_text, m_tooltip_pso);
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
	m_tooltip_text = "";
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
		if (grabbed) {
			this->release();
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
	if (this->grabbed) {
		this->drop(world, relative);
	} else {
		this->drag(world, relative, move.shift);
	}
	return true;
}

void mark::ui::ui::drop(world& world, vd relative)
{
	Expects(grabbed);
	let module_pos = round(relative);
	let ship = mark::ship(world);
	// module's top-left corner
	let drop_pos = module_pos - vi32(grabbed->size()) / 2;
	if (ship->attach(drop_pos, grabbed) == error::code::success) {
		for (let& bind : this->grabbed_bind) {
			ship->toggle_bind(bind, drop_pos);
		}
		grabbed_bind.clear();
		return;
	}
	if (let module = ship->module_at(drop_pos)) {
		let[error, consumed] =
			grabbed->use_on(m_rm, world.blueprints(), *module);
		if (error == error::code::success && consumed) {
			grabbed.reset();
		}
	}
}

void mark::ui::ui::drag(world& world, vd relative, bool shift)
{
	Expects(!grabbed);
	let pick_pos = floor(relative);
	let ship = mark::ship(world);
	grabbed_bind = ship->binding(pick_pos);
	let pos = ship->pos_at(pick_pos);
	if (!pos) {
		return;
	}
	grabbed_prev_pos = *pos;
	let module = ship->module_at(pick_pos);
	Expects(module);
	if (!shift) {
		if (grabbed = ship->detach(pick_pos)) {
			grabbed_prev_parent = ship.get();
		}
		return;
	}
	auto detached = ship->detach(pick_pos);
	if (!detached) {
		return;
	}
	if (error::code::success != push(*ship, detached)) {
		// It should be possible to reattach a module, if it was already
		// attached
		Expects(!ship->attach(grabbed_prev_pos, detached));
	}
}

let constexpr tooltip_layer = 110ull;

void mark::ui::ui::tooltip(
	update_context& context,
	const std::string& text,
	vd screen_pos)
{
	context.sprites[tooltip_layer].emplace_back([&] {
		sprite _;
		_.image = m_tooltip_bg;
		_.pos = screen_pos;
		_.size = tooltip_size;
		_.world = false;
		_.centred = false;
		_.color = { 50, 50, 50, 200 };
		return _;
	}());

	context.render([&] {
		update_context::text_info _;
		_.font = m_font;
		_.layer = tooltip_layer;
		_.pos = screen_pos + vd(tooltip_margin, tooltip_margin);
		_.box = { tooltip_size - tooltip_margin * 2.f,
				  tooltip_size - tooltip_margin * 2.f };
		_.size = font_size;
		_.text = text;
		return _;
	}());
}

void mark::ui::ui::world_tooltip(
	update_context& context,
	const std::string& text,
	vd pos)
{
	context.sprites[tooltip_layer].emplace_back([&] {
		sprite _;
		_.image = m_tooltip_bg;
		_.pos = pos + vd(150, 150), _.size = tooltip_size;
		_.color = { 50, 50, 50, 200 };
		return _;
	}());
	context.render([&] {
		update_context::text_info _;
		_.font = m_font;
		_.layer = 100;
		_.pos = pos + vd(tooltip_margin, tooltip_margin);
		_.box = { tooltip_size - tooltip_margin,
				  tooltip_size - tooltip_margin };
		_.size = font_size;
		_.text = text;
		_.world = true;
		_.centred = true;
		return _;
	}());
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

void mark::ui::ui::release()
{
	if (grabbed && grabbed_prev_parent) {
		Expects(!grabbed_prev_parent->attach(grabbed_prev_pos, grabbed));
		if (let ship = dynamic_cast<unit::modular*>(grabbed_prev_parent)) {
			for (let& bind : this->grabbed_bind) {
				ship->toggle_bind(bind, grabbed_prev_pos);
			}
		}
		grabbed_bind.clear();
	}
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
	if (grabbed) {
		let available = make_available_map(*grabbed, ship);
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
						   std::max(grabbed->size().x, grabbed->size().y))
				* module::size;
			let drop_pos = module_pos
				- vi32(grabbed->size()) / 2; // module's top-left corner
			let color = ship.can_attach(drop_pos, *grabbed) ? sf::Color::Green
															: sf::Color::Red;
			context.sprites[100].emplace_back([&] {
				sprite _;
				_.image = grabbed->thumbnail();
				_.pos = vd(module_pos) * double(module::size)
					+ landing_pad.pos();
				_.size = size;
				_.color = color;
				return _;
			}());
		} else {
			let size = static_cast<float>(
						   std::max(grabbed->size().x, grabbed->size().y))
				* module::size;
			context.sprites[100].emplace_back([&] {
				sprite _;
				_.image = grabbed->thumbnail();
				_.pos = mouse_pos;
				_.size = size;
				return _;
			}());
		}
	}

	// Display tooltips
	let pick_pos = floor(relative);
	if (!grabbed) {
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship

			let module = ship.module_at(pick_pos);
			if (module) {
				let description = module->describe();
				let module_size = vd(module->size())
					* static_cast<double>(module::size);
				let tooltip_pos = module->pos()
					+ vd(module_size.x, -module_size.y) / 2.0;

				this->world_tooltip(context, description, tooltip_pos);
			}
		}
	}
}

void mark::ui::ui::show_ship_editor(unit::modular&) {}

void mark::ui::ui::hide_ship_editor() {}

void mark::ui::ui::tooltip(mark::vi32 pos, const std::string& str)
{
	m_tooltip_text = str;
	m_tooltip_pso = vd(pos);
}
