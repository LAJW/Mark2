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
	auto menu = std::make_unique<window>(vector<int>{ 300, 300 });
	{
		button::info play_button;
		play_button.size = { 250, 50 };
		play_button.parent = menu.get();
		play_button.font = rm.image("font.png");
		play_button.title = "Solitary Traveller";
		auto button = std::make_unique<mark::ui::button>(play_button);
		button->m_relative = true;
		button->on_click.insert([&](let&) {
			stack.push(mode::world);
			return true;
		});
		menu->insert(std::move(button));
	}
	{
		button::info quit_button;
		quit_button.size = { 250, 50 };
		quit_button.parent = menu.get();
		quit_button.font = rm.image("font.png");
		quit_button.title = "Abandon Expedition";
		auto button = std::make_unique<mark::ui::button>(quit_button);
		button->m_relative = true;
		button->on_click.insert([&](let&) {
			stack.push(mode::prompt);
			return true;
		});
		menu->insert(std::move(button));
	}
	return menu;
}

static auto make_prompt(mark::resource::manager& rm, mark::mode_stack& stack)
{
	using namespace mark;
	using namespace ui;
	auto menu = std::make_unique<window>(vector<int>{ 300, 300 });
	{
		button::info play_button;
		play_button.size = { 250, 50 };
		play_button.parent = menu.get();
		play_button.font = rm.image("font.png");
		play_button.title = "Yes";
		auto button = std::make_unique<mark::ui::button>(play_button);
		button->m_relative = true;
		button->on_click.insert([&](let&) {
			stack.clear();
			return true;
		});
		menu->insert(std::move(button));
	}
	{
		button::info quit_button;
		quit_button.size = { 250, 50 };
		quit_button.parent = menu.get();
		quit_button.font = rm.image("font.png");
		quit_button.title = "No";
		auto button = std::make_unique<mark::ui::button>(quit_button);
		button->m_relative = true;
		button->on_click.insert([&](let&) {
			stack.pop();
			return true;
		});
		menu->insert(std::move(button));
	}
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
	m_windows.push_back(std::make_unique<mark::ui::window>());
	m_windows.push_back(std::make_unique<mark::ui::window>());
}

mark::ui::ui::~ui() = default;

void mark::ui::ui::update(
	update_context& context,
	vector<double> resolution,
	vector<double> mouse_pos_)
{
	auto& world = m_world_stack.world();
	if (!m_stack.get().empty()) {
		if (m_stack.get().back() != m_mode) {
			m_mode = m_stack.get().back();
			// router
			if (m_mode == mode::main_menu) {
				m_windows.front() = make_main_menu(m_rm, m_stack);
			} else if (m_mode == mode::world) {
				m_windows.front() = std::make_unique<mark::ui::window>();
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
						return _;
					}());
				container_window->m_relative = true;
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

bool mark::ui::ui::click(vector<int> screen_pos)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	for (let& window : m_windows) {
		let handled = window->click(event);
		if (handled) {
			return true;
		}
	}
	return false;
}

bool mark::ui::ui::hover(vector<int> screen_pos)
{
	mark::ui::event event;
	event.absolute_cursor = screen_pos;
	event.cursor = screen_pos;
	for (let& window : m_windows) {
		let handled = window->hover(event);
		if (handled) {
			return true;
		}
	}
	m_tooltip_text = "";
	return false;
}

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
				return this->click(move->screen_pos);
			}
			return false;
		}
	}

	auto landing_pad =
		std::dynamic_pointer_cast<unit::landing_pad>(world.target());
	if (!landing_pad) {
		return false;
	}
	auto ship = landing_pad->ship();
	if (!ship) {
		return false;
	}
	if (let activate = std::get_if<command::activate>(&any)) {
		if (grabbed) {
			this->release();
			return true;
		} else {
			let relative =
				(activate->pos - landing_pad->pos()) / double(module::size);
			let pick_pos = floor(relative);
			ship->toggle_bind(activate->id, pick_pos);
		}
		return true;
	} else if (let move = std::get_if<command::move>(&any)) {
		let shift = move->shift;
		if (move->release) {
			return false;
		}
		if (this->click(move->screen_pos)) {
			return true;
		}
		let relative = (move->to - landing_pad->pos()) / double(module::size);
		let module_pos = round(relative);
		let pick_pos = floor(relative);
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship drag&drop
			if (grabbed) {
				// module's top-left corner
				let drop_pos = module_pos - vector<int>(grabbed->size()) / 2;
				if (error::code::success == ship->attach(drop_pos, grabbed)) {
					for (let& bind : this->grabbed_bind) {
						ship->toggle_bind(bind, drop_pos);
					}
					grabbed_bind.clear();
				} else if (let module = ship->module_at(drop_pos)) {
					let result =
						grabbed->use_on(m_rm, world.blueprints(), *module);
					if (result.error == error::code::success
						&& result.consumed) {
						grabbed.reset();
					}
				}
			} else {
				grabbed_bind = ship->binding(pick_pos);
				if (let module = ship->module_at(pick_pos)) {
					grabbed_prev_pos = module->grid_pos();
					grabbed = ship->detach(pick_pos);
					if (grabbed) {
						grabbed_prev_parent = ship.get();
						if (shift) {
							if (push(*ship, grabbed) != error::code::success) {
								Expects(
									!ship->attach(grabbed_prev_pos, grabbed));
							}
						}
					} else {
						grabbed_bind.clear();
					}
				}
			}
		}
		return true;
	}
	return false;
}

void mark::ui::ui::tooltip(
	update_context& context,
	const std::string& text,
	vector<double> screen_pos)
{
	sprite info;
	info.image = m_tooltip_bg;
	info.pos = screen_pos;
	info.size = tooltip_size;
	info.world = false;
	info.centred = false;
	info.color = { 50, 50, 50, 200 };
	context.sprites[110].emplace_back(info);

	update_context::text_info text_info;
	text_info.font = m_font;
	text_info.layer = 110;
	text_info.pos = screen_pos + vector<double>(tooltip_margin, tooltip_margin);
	text_info.box = { tooltip_size - tooltip_margin * 2.f,
					  tooltip_size - tooltip_margin * 2.f };
	text_info.size = font_size;
	text_info.text = text;
	context.render(text_info);
}

void mark::ui::ui::world_tooltip(
	update_context& context,
	const std::string& text,
	vector<double> pos)
{
	sprite info;
	info.image = m_tooltip_bg;
	info.pos = pos + vector<double>(150, 150), info.size = tooltip_size;
	info.color = { 50, 50, 50, 200 };
	context.sprites[100].emplace_back(info);

	update_context::text_info text_info;
	text_info.font = m_font;
	text_info.layer = 100;
	text_info.pos = pos + vector<double>(tooltip_margin, tooltip_margin);
	text_info.box = { tooltip_size - tooltip_margin,
					  tooltip_size - tooltip_margin };
	text_info.size = font_size;
	text_info.text = text;
	text_info.world = true;
	text_info.centred = true;
	context.render(text_info);
}

static std::vector<bool> make_available_map(
	const mark::interface::item& item,
	const mark::unit::modular& ship)
{
	using namespace mark;
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vector<int>>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	std::vector<bool> available(grid_size * grid_size, false);
	for (let top_left : surface) {
		if (ship.can_attach(top_left, item)) {
			for (let relative : range(item.size())) {
				let pos = top_left + vector<int>(grid_size / 2, grid_size / 2)
					+ vector<int>(relative);
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
	vector<double> mouse_pos,
	const unit::landing_pad& landing_pad,
	const unit::modular& ship)
{
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vector<int>>(
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
				sprite info;
				info.image = m_grid_bg;
				info.pos = landing_pad.pos()
					+ vector<double>(offset) * double(module::size)
					+ vector<double>(module::size, module::size) / 2.;
				info.size = module::size;
				info.color = sf::Color(0, 255, 255, 255);
				context.sprites[1].emplace_back(info);
			}
		}
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			let size = static_cast<float>(
						   std::max(grabbed->size().x, grabbed->size().y))
				* module::size;
			let drop_pos = module_pos
				- vector<int>(grabbed->size()) / 2; // module's top-left corner
			let color = ship.can_attach(drop_pos, *grabbed) ? sf::Color::Green
															: sf::Color::Red;
			sprite info;
			info.image = grabbed->thumbnail();
			info.pos = vector<double>(module_pos) * double(module::size)
				+ landing_pad.pos();
			info.size = size;
			info.color = color;
			context.sprites[100].emplace_back(info);
		} else {
			let size = static_cast<float>(
						   std::max(grabbed->size().x, grabbed->size().y))
				* module::size;
			sprite info;
			info.image = grabbed->thumbnail();
			info.pos = mouse_pos;
			info.size = size;
			context.sprites[100].emplace_back(info);
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
				let module_size = vector<double>(module->size())
					* static_cast<double>(module::size);
				let tooltip_pos = module->pos()
					+ vector<double>(module_size.x, -module_size.y) / 2.0;

				this->world_tooltip(context, description, tooltip_pos);
			}
		}
	}
}

void mark::ui::ui::show_ship_editor(unit::modular&) {}

void mark::ui::ui::hide_ship_editor() {}

void mark::ui::ui::tooltip(mark::vector<int> pos, const std::string& str)
{
	m_tooltip_text = str;
	m_tooltip_pso = vector<double>(pos);
}
