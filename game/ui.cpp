#include "stdafx.h"
#include "ui.h"
#include "algorithm.h"
#include "interface_has_bindings.h"
#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "ui_window.h"
#include "ui_button.h"
#include "ui_container.h"
#include "unit_landing_pad.h"
#include "unit_modular.h"
#include "world.h"
#include "world_stack.h"

constexpr const auto tooltip_size = 300.f;
constexpr const auto tooltip_margin = 7.f;
constexpr const auto font_size = 14.f;

static auto make_main_menu(mark::resource::manager& rm, mark::mode_stack& stack)
{
	using namespace mark;
	using namespace mark::ui;
	auto menu = std::make_unique<window>(vector<int>{ 300, 300 });
	{
		button::info play_button;
		play_button.size = { 250, 50 };
		play_button.parent = menu.get();
		play_button.font = rm.image("font.png");
		play_button.title = "Solitary Traveller";
		auto button = std::make_unique<mark::ui::button>(play_button);
		button->m_relative = true;
		button->on_click.insert([&](const auto&) {
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
		button->on_click.insert([&](const auto&) {
			stack.clear();
			return true;
		});
		menu->insert(std::move(button));
	}
	return menu;
}

mark::ui::ui::ui(resource::manager& rm, mode_stack& stack, world_stack& world_stack)
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

void mark::ui::ui::tick(
	tick_context& context,
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
			}
		}
	}
	m_action_bar.tick(world, context, m_rm, resolution, mouse_pos_);
	const auto image_circle = m_rm.image("circle.png");
	for (const auto& window : m_windows) {
		window->tick(context);
	}
	const auto mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;

	// Display landing pad UI
	if (auto landing_pad
		= std::dynamic_pointer_cast<unit::landing_pad>(
			world.target())) {
		if (const auto ship = landing_pad->ship()) {
			this->container_ui(context, mouse_pos, *landing_pad, *ship);
			const auto containers = ship->containers();
			auto& window = m_windows.back();
			const auto[removed, added] = diff(
				window->children(), containers, [](const auto& a, const auto& b) {
				return &dynamic_cast<const mark::ui::container*>(a.get())->cargo() == &b.get();
			});
			for (const auto& it : removed) {
				window->children().erase(it);
			}
			for (const auto& pair : added) {
				auto&[it, container] = pair;
				mark::ui::container::info info;
				info.rm = &m_rm;
				info.container = &container.get();
				info.ui = this;
				window->children().insert(it, std::make_unique<mark::ui::container>(info));
				window->children().back()->m_relative = true;
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
	for (const auto& window : m_windows) {
		const auto handled = window->click(event);
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
	for (const auto& window : m_windows) {
		const auto handled = window->hover(event);
		if (handled) {
			return true;
		}
	}
	m_tooltip_text = "";
	return false;
}

bool mark::ui::ui::command(world& world, const mark::command::any &any)
{
	if (std::holds_alternative<command::cancel>(any)) {
		m_stack.pop();
		return true;
	}
	if (const auto guide = std::get_if<command::guide>(&any)) {
		return this->hover(guide->screen_pos);
	}
	if (const auto move = std::get_if<command::move>(&any)) {
		return this->click(move->screen_pos);
	}

	auto landing_pad = std::dynamic_pointer_cast<unit::landing_pad>(world.target());
	if (!landing_pad) {
		return false;
	}
	auto ship = landing_pad->ship();
	if (!ship) {
		return false;
	}
	if (const auto activate = std::get_if<command::activate>(&any)) {
		if (grabbed) {
			this->release();
			return true;
		} else {
			const auto relative = (activate->pos - landing_pad->pos()) / double(module::size);
			const auto pick_pos = floor(relative);
			ship->toggle_bind(activate->id, pick_pos);
		}
		return true;
	} else if (const auto move = std::get_if<command::move>(&any)) {
		if (move->release) {
			return false;
		}
		const auto relative = (move->to - landing_pad->pos()) / double(module::size);
		const auto module_pos = round(relative);
		const auto pick_pos = floor(relative);
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship drag&drop
			if (grabbed) {
				// module's top-left corner
				const auto drop_pos = module_pos
					- vector<int>(grabbed->size()) / 2;
				if (error::code::success == ship->attach(drop_pos, grabbed)) {
					for (const auto& bind : this->grabbed_bind) {
						ship->toggle_bind(bind, drop_pos);
					}
					grabbed_bind.clear();
				}
			} else {
				grabbed_bind = ship->binding(pick_pos);
				grabbed = ship->detach(pick_pos);
				if (grabbed) {
					grabbed_prev_pos = grabbed->grid_pos();
					grabbed_prev_parent = ship.get();
				} else {
					grabbed_bind.clear();
				}
			}
		}
		return true;
	}
	return false;
}

void mark::ui::ui::tooltip(
	tick_context& context,
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

	tick_context::text_info text_info;
	text_info.font = m_font;
	text_info.layer = 110;
	text_info.pos = screen_pos + vector<double>(tooltip_margin, tooltip_margin);
	text_info.box = {
		tooltip_size - tooltip_margin * 2.f,
		tooltip_size - tooltip_margin * 2.f };
	text_info.size = font_size;
	text_info.text = text;
	context.render(text_info);

}

void mark::ui::ui::world_tooltip(
	tick_context& context,
	const std::string& text,
	vector<double> pos)
{
	sprite info;
	info.image = m_tooltip_bg;
	info.pos = pos + vector<double>(150, 150),
	info.size = tooltip_size;
	info.color = { 50, 50, 50, 200 };
	context.sprites[100].emplace_back(info);

	tick_context::text_info text_info;
	text_info.font = m_font;
	text_info.layer = 100;
	text_info.pos = pos + vector<double>(tooltip_margin, tooltip_margin);
	text_info.box = {
		tooltip_size - tooltip_margin,
		tooltip_size - tooltip_margin };
	text_info.size = font_size;
	text_info.text = text;
	text_info.world = true;
	text_info.centred = true;
	context.render(text_info);
}

static std::vector<bool> make_available_map(
	const mark::module::base& module,
	const mark::unit::modular& ship)
{
	using namespace mark;
	constexpr const auto grid_size = unit::modular::max_size;
	const auto surface = range<vector<int>>(
		{ -int(grid_size) / 2, -int(grid_size) /2 },
		{ grid_size / 2, grid_size / 2 });
	std::vector<bool> available(grid_size * grid_size, false);
	for (const auto top_left : surface) {
		if (ship.can_attach(top_left, module)) {
			for (const auto relative : range(module.size())) {
				const auto pos = top_left
					+ vector<int>(grid_size / 2, grid_size / 2)
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
		(void)grabbed_prev_parent->attach(grabbed_prev_pos, grabbed);
		if (const auto ship = dynamic_cast<unit::modular*>(grabbed_prev_parent)) {
			for (const auto& bind : this->grabbed_bind) {
				ship->toggle_bind(bind, grabbed_prev_pos);
			}
		}
		grabbed_bind.clear();
	}
}

void mark::ui::ui::container_ui(
	tick_context& context,
	vector<double> mouse_pos,
	const unit::landing_pad& landing_pad,
	const unit::modular& ship)
{
	constexpr const auto grid_size = unit::modular::max_size;
	const auto surface = range<vector<int>>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	const auto relative = (mouse_pos - landing_pad.pos()) / double(module::size);
	const auto module_pos = round(relative);
	if (grabbed) {
		const auto available = make_available_map(*grabbed, ship);
		for (const auto offset : surface) {
			if (available[offset.x + grid_size / 2 + (offset.y + grid_size / 2) * grid_size]) {
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
			const auto size = static_cast<float>(std::max(
				grabbed->size().x,
				grabbed->size().y)) * module::size;
			const auto drop_pos = module_pos - vector<int>(grabbed->size()) / 2; // module's top-left corner
			const auto color = ship.can_attach(drop_pos, *grabbed) ? sf::Color::Green : sf::Color::Red;
			sprite info;
			info.image = grabbed->thumbnail();
			info.pos = vector<double>(module_pos) * double(module::size) + landing_pad.pos();
			info.size = size;
			info.color = color;
			context.sprites[100].emplace_back(info);
		} else {
			const auto size = static_cast<float>(std::max(grabbed->size().x, grabbed->size().y)) * module::size;
			sprite info;
			info.image = grabbed->thumbnail();
			info.pos = mouse_pos;
			info.size = size;
			context.sprites[100].emplace_back(info);
		}
	}

	// Display tooltips
	const auto pick_pos = floor(relative);
	if (!grabbed) {
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship

			const auto module = ship.at(pick_pos);
			if (module) {
				const auto description = module->describe();
				const auto module_size = vector<double>(module->size()) * static_cast<double>(module::size);
				const auto tooltip_pos = module->pos() + vector<double>(module_size.x, -module_size.y) / 2.0;

				this->world_tooltip(context, description, tooltip_pos);
			}
		}
	}
}

void mark::ui::ui::show_ship_editor(unit::modular&)
{ }

void mark::ui::ui::hide_ship_editor()
{ }

void mark::ui::ui::tooltip(mark::vector<int> pos, const std::string& str)
{
	m_tooltip_text = str;
	m_tooltip_pso = vector<double>(pos);
}