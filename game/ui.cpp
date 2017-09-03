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
		const auto bindings = unit->bindings();
		for (const auto i : mark::enumerate(bindings.size())) {
			const auto& binding = bindings[i];
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
			if (landing_pad->grabbed()) {
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
			if (const auto grabbed = landing_pad->grabbed()) {
				if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
					const auto size = static_cast<float>(std::max(grabbed->size().x, grabbed->size().y)) * 16.f;
					const auto drop_pos = module_pos - mark::vector<int>(grabbed->size()) / 2; // module's top-left corner
					const auto color = ship->can_attach(*grabbed, drop_pos) ? sf::Color::Green : sf::Color::Red;
					mark::sprite::info info;
					info.image = grabbed->thumbnail();
					info.pos = mark::vector<double>(module_pos * 16) + landing_pad->pos();
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
			if (!landing_pad->grabbed()) {
				if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
					// ship

					const auto module = ship->module(pick_pos);
					if (module) {
						const auto description = module->describe();
						const auto module_pos = module->pos();
						const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
						const auto tooltip_pos = module_pos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;

						mark::sprite::info info;
						info.image = m_tooltip_bg;
						info.pos = tooltip_pos + mark::vector<double>(150, 150),
						info.size = 300.f;
						context.sprites[100].emplace_back(info);

						mark::print(
							m_font,
							context.sprites[100],
							tooltip_pos + mark::vector<double>(7.f, 7.f),
							{ 300 - 14.f, 300 - 14.f },
							14.f,
							sf::Color::White,
							description
						);
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

								mark::sprite::info info;
								info.image = m_tooltip_bg;
								info.pos = tooltip_pos + mark::vector<double>(150, 150),
								info.size = 300.f;
								context.sprites[100].emplace_back(info);

								mark::print(
									m_font,
									context.sprites[100],
									tooltip_pos + mark::vector<double>(7.f, 7.f),
									{ 300 - 14.f, 300 - 14.f },
									14.f,
									sf::Color::White,
									description
								);
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
