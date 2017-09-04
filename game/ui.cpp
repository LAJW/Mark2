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

mark::ui::ui(mark::resource::manager& rm)
	: m_font(rm.image("font.png"))
	, m_tooltip_bg(rm.image("floor.png"))
	, m_grid_bg(rm.image("grid-background.png"))
	, m_hotbar_bg(rm.image("hotbar-background.png"))
	, m_hotbar_overlay(rm.image("hotbar-overlay.png")) { }

mark::ui::~ui() = default;

void mark::ui::tick(
	const mark::world& world,
	mark::tick_context& context,
	mark::resource::manager& rm,
	mark::vector<double> resolution,
	mark::vector<double> mouse_pos_)
{
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

	const auto grid = m_grid_bg;
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
	const auto surface = mark::enumerate<mark::vector<int>>(
		{ -20, -20 },
		{ 20, 20 });
	if (const auto landing_pad
		= std::dynamic_pointer_cast<const mark::unit::landing_pad>(
			world.target())) {
		if (const auto ship = landing_pad->ship()) {
			const auto relative = (mouse_pos - landing_pad->pos()) / double(mark::module::size);
			const auto module_pos = mark::round(relative);
			if (m_grabbed) {
				for (const auto offset : surface) {
					mark::sprite::info info;
					info.image = grid;
					info.pos = landing_pad->pos() + mark::vector<double>(offset) * 16.0 - mark::vector<double>(8.0, 8.0);
					info.size = 16.f;
					const auto alpha = std::max(1.0 - mark::length(info.pos - mouse_pos) / 320.0, 0.0) * 255.0;
					info.color = sf::Color(255, 255, 255, static_cast<uint8_t>(alpha));
					context.sprites[1].emplace_back(info);
				}
			}
			double top = 0.0;
			for (auto& cargo_ref : ship->containers()) {
				auto& cargo = cargo_ref.get();
				auto pos = mark::vector<double>(landing_pad->pos().x + 320.0, landing_pad->pos().y - 320.0 + top);
				cargo.render_contents(pos, context);
				top += cargo.interior_size().y * 16.0 + 32.0;
			}
			if (m_grabbed) {
				if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
					const auto size = static_cast<float>(std::max(m_grabbed->size().x, m_grabbed->size().y)) * 16.f;
					const auto drop_pos = module_pos - mark::vector<int>(m_grabbed->size()) / 2; // module's top-left corner
					const auto color = ship->can_attach(*m_grabbed, drop_pos) ? sf::Color::Green : sf::Color::Red;
					mark::sprite::info info;
					info.image = m_grabbed->thumbnail();
					info.pos = mark::vector<double>(module_pos * 16) + landing_pad->pos();
					info.size = size;
					info.color = color;
					context.sprites[100].emplace_back(info);
				} else {
					const auto size = static_cast<float>(std::max(m_grabbed->size().x, m_grabbed->size().y)) * 16.f;
					mark::sprite::info info;
					info.image = m_grabbed->thumbnail();
					info.pos = mouse_pos;
					info.size = size;
					context.sprites[100].emplace_back(info);
				}
			}

			// Display tooltips
			const auto pick_pos = mark::floor(relative);
			if (!m_grabbed) {
				if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
					// ship

					const auto module = ship->module(pick_pos);
					if (module) {
						const auto description = module->describe();
						const auto module_pos = module->pos();
						const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
						const auto tooltip_pos = module_pos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;

						this->tooltip(context, description, tooltip_pos);
					}
				} else if (std::abs(relative.y) < 320.0 && relative.x < 320.0 + 16.0 * 16.0) {
					double top = 0.0;
					for (auto& cargo_ref : ship->containers()) {
						auto& cargo = cargo_ref.get();
						const auto size = cargo.interior_size();
						const auto relative = mouse_pos - landing_pad->pos() + mark::vector<double>(-320 + 8, -top + 320 + 8);
						if (relative.y >= 0 && relative.y < size.y * 16) {
							const auto pick_pos = mark::floor(relative / 16.0);
							const auto module = cargo.module(pick_pos);
							if (module) {
								const auto description = module->describe();
								const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
								const auto tooltip_pos = mouse_pos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;

								this->tooltip(context, description, tooltip_pos);
							}
							break;
						}
						top += size.y * 16.0 + 32.0;
					}
				}
			}
		}
	}
}

void mark::ui::command(world& world, const mark::command &command)
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
					if (m_grabbed) {
						const auto drop_pos = module_pos - mark::vector<int>(m_grabbed->size()) / 2; // module's top-left corner
						try {
							ship->attach(m_grabbed, drop_pos);
						} catch (const mark::exception&) { /* no-op */ }
					} else {
						m_grabbed = ship->detach(pick_pos);
					}
				} else if (std::abs(relative.y) < 320.0 && relative.x < 320.0 + 16.0 * 16.0) {
					// cargo drag&drop
					double top = 0.0;
					for (auto& cargo_ref : ship->containers()) {
						auto& cargo = cargo_ref.get();
						const auto size = cargo.interior_size();
						const auto relative = command.pos - landing_pad->pos() + mark::vector<double>(-320 + 8, -top + 320 + 8);
						if (relative.y >= 0 && relative.y < size.y * 16) {
							if (m_grabbed) {
								const auto drop_pos = mark::round(relative / 16.0 - mark::vector<double>(m_grabbed->size()) / 2.0);
								if (cargo.drop(drop_pos, m_grabbed)
									== mark::error::code::success) {
									break;
								}
							} else {
								const auto pick_pos = mark::floor(relative / 16.0);
								m_grabbed = cargo.pick(pick_pos);
							}
							break;
						}
						top += size.y * 16.0 + 32.0;
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

void mark::ui::tooltip(
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
