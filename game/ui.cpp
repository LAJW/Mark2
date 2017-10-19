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
#include "interface_has_bindings.h"
#include "ui_window.h"
#include "ui_button.h"
#include "ui_container.h"

mark::ui::ui::ui(resource::manager& rm)
	: m_font(rm.image("font.png"))
	, m_tooltip_bg(rm.image("floor.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_hotbar_bg(rm.image("hotbar-background.png"))
	, m_hotbar_overlay(rm.image("hotbar-overlay.png"))
	, m_rm(rm) { }

mark::ui::ui::~ui() = default;

void mark::ui::ui::tick(
	world& world,
	tick_context& context,
	resource::manager& rm,
	vector<double> resolution,
	vector<double> mouse_pos_)
{
	const auto image_circle = rm.image("circle.png");
	const auto image_ray = rm.image("ray.png");
	for (const auto& window : m_windows) {
		window->tick(context);
	}
	const auto mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
	// Display Hotbar
	{
		sprite bg_sprite;
		bg_sprite.image = m_hotbar_bg;
		bg_sprite.pos.x = resolution.x / 2. - 23. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 85;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[0].emplace_back(bg_sprite);
	}
	{
		sprite bg_sprite;
		bg_sprite.image = m_hotbar_overlay;
		bg_sprite.pos.x = resolution.x / 2. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 64;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		context.ui_sprites[2].emplace_back(bg_sprite);
	}

	if (const auto unit
		= std::dynamic_pointer_cast<const interface::has_bindings>(world.target())) {
		const auto icon_size = 64.0;
		const auto bindings = unit->bindings();
		for (size_t i = 0; i < bindings.size(); ++i) {
			const auto binding = bindings[i];
			// const auto&[i, binding] = pair;
			const auto di = static_cast<double>(i);
			const auto x = resolution.x / 2. - 64. * 5.5 + 64.0 * i;
			const auto y = resolution.y - 64;;

			// Highlight bindings
			const auto center = resolution / 2.0;
			if (std::dynamic_pointer_cast<const unit::landing_pad>(unit)) {
				for (const auto& module : binding.modules) {
					const auto pos = module.get().grid_pos();
					const auto module_pos = center
						+ vector<double>(pos) * static_cast<double>(module::size);
					const auto module_size
						= vector<double>(module.get().size())
							* static_cast<double>(module::size);
					if (!(x <= mouse_pos_.x && mouse_pos_.x < x + 64.
						&& y <= mouse_pos_.y && mouse_pos_.y < y + 64.)
						&& !(module_pos.x <= mouse_pos_.x && mouse_pos_.x < module_pos.x + module_size.x
							&& module_pos.y <= mouse_pos_.y && mouse_pos_.y < module_pos.y + module_size.y))
						continue;
					sprite circle_sprite;
					const auto module_pos_ = module_pos + vector<double>(8, 8);
					circle_sprite.pos = module_pos_;
					circle_sprite.image = image_circle;
					circle_sprite.frame = std::numeric_limits<size_t>::max();
					context.ui_sprites[1].push_back(circle_sprite);
					const auto diff = vector<double>(x + 32, y + 32) - module_pos_;
					const auto dir = normalize(diff);
					const auto dist = length(diff);
					for (auto s = 0.; s < dist; s += 16.) {
						sprite line_sprite;
						line_sprite.pos = module_pos_ + dir * s + vector<double>(16, 16);
						line_sprite.image = image_ray;
						line_sprite.frame = std::numeric_limits<size_t>::max();
						line_sprite.rotation = static_cast<float>(atan(dir));
						context.ui_sprites[1].push_back(line_sprite);
					}
				}
			}

			if (binding.thumbnail) {
				mark::sprite sprite;
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
				print(
					m_font,
					context.ui_sprites[1],
					vector<double>(x + 32.f, y + 8.f),
					{ 300 - 14.f, 300 - 14.f },
					14.f,
					sf::Color::White,
					os.str()
				);
			}
			{
				std::ostringstream os;
				os << binding.total;
				print(
					m_font,
					context.ui_sprites[1],
					vector<double>(x + 32.f, y + 32.f),
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
		= std::dynamic_pointer_cast<unit::landing_pad>(
			world.target())) {
		if (const auto ship = landing_pad->ship()) {
			this->container_ui(context, mouse_pos, *landing_pad, *ship);
			if (m_windows.empty()
				|| ship->containers().size() != m_container_count
				|| m_redraw_ui) {
				m_redraw_ui = false;
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

void mark::ui::ui::command(world& world, const mark::command &command)
{
	auto landing_pad = std::dynamic_pointer_cast<unit::landing_pad>(world.target());
	if (!landing_pad) {
		return;
	}
	auto ship = landing_pad->ship();
	if (!ship) {
		return;
	}
	if (command.type == command::type::shoot && !command.release && grabbed) {
		this->release();
	} else if (command.type == command::type::move && !command.release) {
		const auto relative = (command.pos - landing_pad->pos()) / 16.0;
		const auto module_pos = round(relative);
		const auto pick_pos = floor(relative);
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			// ship drag&drop
			if (grabbed) {
				// module's top-left corner
				const auto drop_pos = module_pos
					- vector<int>(grabbed->size()) / 2;
				(void)ship->attach(drop_pos, grabbed);
				for (const auto& bind : this->grabbed_bind) {
					ship->toggle_bind(bind, drop_pos);
				}
				grabbed_bind.clear();
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
	} else if (!command.release) {
		if (command.type >= command::type::ability_0
			&& command.type <= command::type::ability_9
			|| command.type == command::type::shoot) {
			const auto relative = (command.pos - landing_pad->pos()) / 16.0;
			const auto pick_pos = floor(relative);
			ship->toggle_bind(command.type, pick_pos);
		}
	}
}

void mark::ui::ui::tooltip(
	tick_context& context,
	const std::string& text,
	vector<double> screen_pos)
{
	sprite info;
	info.image = m_tooltip_bg;
	info.pos = screen_pos;
	info.size = 300.f;
	context.ui_sprites[10].emplace_back(info);

	print(
		m_font,
		context.ui_sprites[10],
		screen_pos + vector<double>(7.f, 7.f),
		{ 300 - 14.f, 300 - 14.f },
		14.f,
		sf::Color::White,
		text
	);

}

void mark::ui::ui::world_tooltip(
	tick_context& context,
	const std::string& text,
	vector<double> pos)
{
	sprite info;
	info.image = m_tooltip_bg;
	info.pos = pos + vector<double>(150, 150),
	info.size = 300.f;
	context.sprites[100].emplace_back(info);

	print(
		m_font,
		context.sprites[100],
		pos + vector<double>(7.f, 7.f),
		{ 300 - 14.f, 300 - 14.f },
		14.f,
		sf::Color::White,
		text
	);

}

static std::vector<bool> make_available_map(
	const mark::module::base& module,
	const mark::unit::modular& ship)
{
	using namespace mark;
	const auto surface = range<vector<int>>(
		{ -20, -20 },
		{ 20, 20 });
	std::vector<bool> available(40 * 40, false);
	for (const auto top_left : surface) {
		if (ship.can_attach(top_left, module)) {
			for (const auto relative : range(module.size())) {
				const auto pos = top_left
					+ vector<int>(20, 20)
					+ vector<int>(relative);
				if (pos.x < 40 && pos.y < 40) {
					available[pos.x + pos.y * 40] = true;
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
		m_redraw_ui = true;
	}
}

void mark::ui::ui::container_ui(
	tick_context& context,
	vector<double> mouse_pos,
	const unit::landing_pad& landing_pad,
	const unit::modular& ship)
{
	const auto surface = range<vector<int>>(
		{ -20, -20 },
		{ 20, 20 });
	const auto relative = (mouse_pos - landing_pad.pos()) / double(module::size);
	const auto module_pos = round(relative);
	if (grabbed) {
		const auto available = make_available_map(*grabbed, ship);
		for (const auto offset : surface) {
			if (available[offset.x + 20 + (offset.y + 20) * 40]) {
				sprite info;
				info.image = m_grid_bg;
				info.pos = landing_pad.pos() + vector<double>(offset) * 16.0 + vector<double>(8.0, 8.0);
				info.size = 16.f;
				info.color = sf::Color(0, 255, 255, 255);
				context.sprites[1].emplace_back(info);
			}
		}
		if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
			const auto size = static_cast<float>(std::max(
				grabbed->size().x,
				grabbed->size().y)) * 16.f;
			const auto drop_pos = module_pos - vector<int>(grabbed->size()) / 2; // module's top-left corner
			const auto color = ship.can_attach(drop_pos, *grabbed) ? sf::Color::Green : sf::Color::Red;
			sprite info;
			info.image = grabbed->thumbnail();
			info.pos = vector<double>(module_pos * 16) + landing_pad.pos();
			info.size = size;
			info.color = color;
			context.sprites[100].emplace_back(info);
		} else {
			const auto size = static_cast<float>(std::max(grabbed->size().x, grabbed->size().y)) * 16.f;
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

void mark::ui::ui::show_ship_editor(unit::modular& modular)
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
	m_tooltip_pso = vector<double>(pos);
}