#include <algorithm>
#include "unit_landing_pad.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "module_shield_generator.h"
#include "command.h"
#include "module_cargo.h"

mark::unit::landing_pad::landing_pad(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_image(world.resource_manager().image("landing-pad.png")) {
}

void mark::unit::landing_pad::tick(mark::tick_context& context) {
	context.sprites[0].push_back(mark::sprite(m_image, m_pos, 320.f, 0.f));
	const auto image = m_world.resource_manager().image("grid-background.png");
	auto ship = m_ship.lock();
	if (ship) {
		for (int x = -10; x < 10; x++) {
			for (int y = -10; y < 10; y++) {
				const auto pos = m_pos + mark::vector<double>(x, y) * 32.0;
				context.sprites[0].push_back(mark::sprite(image, pos, 32.f));
			}
		}
		double top = 0.0;
		for (auto& cargo_ref : ship->containers()) {
			auto& cargo = cargo_ref.get();
			auto pos = mark::vector<double>(m_pos.x + 320.0, m_pos.y - 320.0 + top);
			cargo.render_contents(pos, context);
			top += cargo.interior_size().y * 16.0 + 32.0;
		}
		if (m_grabbed) {
			const auto size = static_cast<float>(std::max(m_grabbed->size().x, m_grabbed->size().y)) * 16.f;
			context.sprites[0].push_back(mark::sprite(m_grabbed->thumbnail(), m_mousepos, size));
		}

		// display tooltips
		const auto relative = (m_mousepos - m_pos) / 16.0;
		const auto module_pos = mark::round(relative);
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
					context.sprites[100].emplace_back(
						m_world.resource_manager().image("wall.png"),
						tooltip_pos + mark::vector<double>(150, 150),
						300.f);
					mark::print(
						m_world.resource_manager().image("font.png"),
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
					const auto relative = m_mousepos - m_pos + mark::vector<double>(-320 + 8, -top + 320 + 8);
					if (relative.y >= 0 && relative.y < size.y * 16) {
						const auto pick_pos = mark::floor(relative / 16.0);
						const auto module = cargo.module(pick_pos);
						if (module) {
							const auto description = module->describe();
							const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
							const auto tooltip_pos = m_mousepos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;
							context.sprites[100].emplace_back(
								m_world.resource_manager().image("wall.png"),
								tooltip_pos + mark::vector<double>(150, 150),
								300.f);
							mark::print(
								m_world.resource_manager().image("font.png"),
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

void mark::unit::landing_pad::dock(mark::unit::modular* ship) {
	if (ship) {
		auto ship_ptr = std::dynamic_pointer_cast<mark::unit::modular>(m_world.find_one(m_pos, 500.0, [ship](const mark::unit::base& unit) {
			return &unit == ship;
		}));
		m_ship = ship_ptr;
	} else {
		m_ship.reset();
	}
}

void mark::unit::landing_pad::activate(const std::shared_ptr<mark::unit::base>& by) {
	auto modular = std::dynamic_pointer_cast<mark::unit::modular>(by);
	if (modular) {
		m_ship = modular;
		m_world.target(this->shared_from_this());
		mark::command move;
		move.type = mark::command::type::move;
		move.pos = m_pos;
		modular->command(move);
		mark::command look;
		look.type = mark::command::type::guide;
		look.pos = m_pos + mark::vector<double>(1, 0);
		modular->command(look);
	}
}

void mark::unit::landing_pad::command(const mark::command & command) {
	if (command.type == mark::command::type::activate) {
		auto ship = m_ship.lock();
		if (ship) {
			ship->activate(this->shared_from_this());
			m_ship.reset();
		}
	} else if (command.type == mark::command::type::guide) {
		m_mousepos = command.pos;
	} else if (command.type == mark::command::type::move) {
		auto ship = m_ship.lock();
		if (ship) {
			const auto relative = (command.pos - m_pos) / 16.0;
			const auto module_pos = mark::round(relative);
			const auto pick_pos = mark::floor(relative);
			if (std::abs(module_pos.x) <= 17 && std::abs(module_pos.y) <= 17) {
				// ship drag&drop
				if (m_grabbed) {
					const auto drop_pos = module_pos - mark::vector<int>(m_grabbed->size()) / 2; // module's top-left corner
					if (ship->can_attach(m_grabbed, drop_pos)) {
						ship->attach(std::move(m_grabbed), drop_pos);
					}
				} else {
					m_grabbed = ship->detach(pick_pos);
				}
			} else if (std::abs(relative.y) < 320.0 && relative.x < 320.0 + 16.0 * 16.0) {
				// cargo drag&drop
				double top = 0.0;
				for (auto& cargo_ref : ship->containers()) {
					auto& cargo = cargo_ref.get();
					const auto size = cargo.interior_size();
					const auto relative = command.pos - m_pos + mark::vector<double>(-320 + 8, -top + 320 + 8);
					if (relative.y >= 0 && relative.y < size.y * 16) {
						if (m_grabbed) {
							const auto drop_pos = mark::round(relative / 16.0 - mark::vector<double>(m_grabbed->size()) / 2.0);
							if (cargo.can_drop(drop_pos, m_grabbed)) {
								cargo.drop(drop_pos, std::move(m_grabbed));
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
	}
}

auto mark::unit::landing_pad::collides(mark::vector<double> pos, float radius) const -> bool {
	return false;
}
