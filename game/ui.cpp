#include "stdafx.h"
#include "ui.h"
#include "algorithm.h"
#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_landing_pad.h"
#include "unit_modular.h"
#include "world.h"
#include "ihas_bindings.h"
#include "ui_window.h"
#include "ui_button.h"
#include "ui_container.h"

mark::ui::ui::ui(mark::resource::manager& rm)
	: m_font(rm.image("font.png"))
	, m_tooltip_bg(rm.image("floor.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_hotbar_bg(rm.image("hotbar-background.png"))
	, m_hotbar_overlay(rm.image("hotbar-overlay.png"))
	, m_rm(rm) { }

mark::ui::ui::~ui() = default;

void mark::ui::ui::tick(
	mark::world& world,
	mark::tick_context& context,
	mark::resource::manager& rm,
	mark::vector<double> resolution,
	mark::vector<double> mouse_pos_)
{
	for (const auto& window : m_windows) {
		window->tick(context);
	}
	const auto mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
	// Display Hotbar
	{
		mark::sprite::info bg_sprite;
		bg_sprite.image = m_hotbar_bg;
		bg_sprite.pos.x = resolution.x / 2. - 23. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 85;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[0].emplace_back(bg_sprite);
	}
	{
		mark::sprite::info bg_sprite;
		bg_sprite.image = m_hotbar_overlay;
		bg_sprite.pos.x = resolution.x / 2. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 64;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[2].emplace_back(bg_sprite);
	}

	if (const auto unit
		= std::dynamic_pointer_cast<const mark::ihas_bindings>(world.target())) {
		const auto icon_size = 64.0;
		for (const auto pair : mark::enumerate(unit->bindings())) {
			const auto [i, binding] = pair;
			const auto di = static_cast<double>(i);
			const auto x = resolution.x / 2. - 64. * 5.5 + 64.0 * i;
			const auto y = resolution.y - 64;;
			if (binding.thumbnail) {
				mark::sprite::info sprite;
				sprite.image = binding.thumbnail;
				sprite.pos.x = x;
				sprite.pos.y = y;
				sprite.image = binding.thumbnail;
				sprite.size = 64.f;
				context.ui_sprites[1].emplace_back(sprite);
			}
			{
				std::ostringstream os;
				os << static_cast<int>(i);
				mark::print(
					m_font,
					context.ui_sprites[1],
					mark::vector<double>(x + 32.f, y + 8.f),
					{ 300 - 14.f, 300 - 14.f },
					14.f,
					sf::Color::White,
					os.str()
				);
			}
			{
				std::ostringstream os;
				os << binding.total;
				mark::print(
					m_font,
					context.ui_sprites[1],
					mark::vector<double>(x + 32.f, y + 32.f),
					{ 300 - 14.f, 300 - 14.f },
					14.f,
					sf::Color::White,
					os.str()
				);
			}
		}
	}
	// Display landing pad UI
	if (auto landing_pad
		= std::dynamic_pointer_cast<mark::unit::landing_pad>(
			world.target())) {
		if (auto& ship = landing_pad->ship()) {
			this->container_ui(
				world, context, resolution, mouse_pos_, mouse_pos,
				*landing_pad, *ship);
			if (m_windows.empty()
				|| ship->containers().size() != m_container_count) {
				m_container_count = ship->containers().size();
				this->hide_ship_editor();
				this->show_ship_editor(*ship);
			}
		}
	} else {
		this->hide_ship_editor();
	}
	if (!m_tooltip_text.empty()) {
		this->tooltip(context, m_tooltip_text, m_tooltip_pso);
	}
}

bool mark::ui::ui::click(mark::vector<int> screen_pos)
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

bool mark::ui::ui::hover(mark::vector<int> screen_pos)
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

void mark::ui::ui::command(world& world, const mark::command &command)
{
	if (auto landing_pad
		= std::dynamic_pointer_cast<mark::unit::landing_pad>(
			world.target())) {
		if (command.type == mark::command::type::move && !command.release) {
			if (auto ship = landing_pad->ship()) {
				const auto relative = (command.pos - landing_pad->pos()) / 16.0;
				const auto module_pos = mark::round(relative);
				const auto pick_pos = mark::floor(relative);
				if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
					// ship drag&drop
					if (grabbed) {
						const auto drop_pos = module_pos - mark::vector<int>(grabbed->size()) / 2; // module's top-left corner
						try {
							ship->attach(grabbed, drop_pos);
						} catch (const mark::exception&) { /* no-op */ }
					} else {
						grabbed = ship->detach(pick_pos);
					}
				}
			}
		} else if (!command.release) {
			if (command.type >= command::type::ability_0
				&& command.type <= command::type::ability_9
				|| command.type == command::type::shoot) {
				if (auto ship = landing_pad->ship()) {
					const auto relative = (command.pos - landing_pad->pos()) / 16.0;
					const auto pick_pos = mark::floor(relative);
					ship->toggle_bind(command.type, pick_pos);
				}
			}
		}
	}
}

void mark::ui::ui::tooltip(
	tick_context& context,
	const std::string& text,
	mark::vector<double> screen_pos)
{
	mark::sprite::info info;
	info.image = m_tooltip_bg;
	info.pos = screen_pos;
	info.size = 300.f;
	context.ui_sprites[10].emplace_back(info);

	mark::print(
		m_font,
		context.ui_sprites[10],
		screen_pos + mark::vector<double>(7.f, 7.f),
		{ 300 - 14.f, 300 - 14.f },
		14.f,
		sf::Color::White,
		text
	);

}

void mark::ui::ui::world_tooltip(
	tick_context& context,
	const std::string& text,
	mark::vector<double> pos)
{
	mark::sprite::info info;
	info.image = m_tooltip_bg;
	info.pos = pos + mark::vector<double>(150, 150),
	info.size = 300.f;
	context.sprites[100].emplace_back(info);

	mark::print(
		m_font,
		context.sprites[100],
		pos + mark::vector<double>(7.f, 7.f),
		{ 300 - 14.f, 300 - 14.f },
		14.f,
		sf::Color::White,
		text
	);

}

void mark::ui::ui::container_ui(
	const mark::world& world,
	mark::tick_context& context,
	mark::vector<double> resolution,
	mark::vector<double> screen_pos,
	mark::vector<double> mouse_pos,
	const mark::unit::landing_pad& landing_pad,
	const mark::unit::modular& ship)
{
	const auto surface = mark::range<mark::vector<int>>(
		{ -20, -20 },
		{ 20, 20 });
	const auto relative = (mouse_pos - landing_pad.pos()) / double(mark::module::size);
	const auto module_pos = mark::round(relative);
	if (grabbed) {
		for (const auto offset : surface) {
			mark::sprite::info info;
			info.image = m_grid_bg;
			info.pos = landing_pad.pos() + mark::vector<double>(offset) * 16.0 + mark::vector<double>(8.0, 8.0);
			info.size = 16.f;
			const auto alpha = std::max(1.0 - mark::length(info.pos - mouse_pos) / 320.0, 0.0) * 255.0;
			if (ship.p_reserved(vector<int8_t>(offset.x, offset.y))) {
				info.color = sf::Color(255, 0, 0, static_cast<uint8_t>(alpha));
			} else {
				info.color = sf::Color(0, 255, 255, static_cast<uint8_t>(alpha));
			}
			context.sprites[1].emplace_back(info);
		}
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			const auto size = static_cast<float>(std::max(
				grabbed->size().x,
				grabbed->size().y)) * 16.f;
			const auto drop_pos = module_pos - mark::vector<int>(grabbed->size()) / 2; // module's top-left corner
			const auto color = ship.can_attach(*grabbed, drop_pos) ? sf::Color::Green : sf::Color::Red;
			mark::sprite::info info;
			info.image = grabbed->thumbnail();
			info.pos = mark::vector<double>(module_pos * 16) + landing_pad.pos();
			info.size = size;
			info.color = color;
			context.sprites[100].emplace_back(info);
		} else {
			const auto size = static_cast<float>(std::max(grabbed->size().x, grabbed->size().y)) * 16.f;
			mark::sprite::info info;
			info.image = grabbed->thumbnail();
			info.pos = mouse_pos;
			info.size = size;
			context.sprites[100].emplace_back(info);
		}
	}

	// Display tooltips
	const auto pick_pos = mark::floor(relative);
	if (!grabbed) {
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship

			const auto module = ship.module(pick_pos);
			if (module) {
				const auto description = module->describe();
				const auto module_pos = module->pos();
				const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
				const auto tooltip_pos = module_pos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;

				this->world_tooltip(context, description, tooltip_pos);
			}
		}
	}
}

void mark::ui::ui::show_ship_editor(mark::unit::modular& modular)
{
	auto window_ptr = std::make_unique<mark::ui::window>();
	auto& window = *window_ptr;
	int top = 0;
	for (auto& container : modular.containers()) {
		mark::ui::container::info info;
		info.rm = &m_rm;
		info.container = &container.get();
		info.pos = { 0, top };
		info.ui = this;
		window.insert(std::make_unique<mark::ui::container>(info));
		top += 24 * container.get().interior_size().y;
	}
	m_windows.push_back(std::move(window_ptr));
}

void mark::ui::ui::hide_ship_editor()
{
	m_windows.clear();
}

void mark::ui::ui::tooltip(mark::vector<int> pos, const std::string& str)
{
	m_tooltip_text = str;
	m_tooltip_pso = mark::vector<double>(pos);
}