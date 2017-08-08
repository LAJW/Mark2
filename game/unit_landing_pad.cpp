
#include "stdafx.h"
#include "unit_landing_pad.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "module_shield_generator.h"
#include "command.h"
#include "module_cargo.h"
#include "exception.h"

mark::unit::landing_pad::landing_pad(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_image(world.resource_manager().image("landing-pad.png")) {
}

void mark::unit::landing_pad::tick(mark::tick_context& context) {
	mark::sprite::info info;
	info.image = m_image;
	info.pos = pos();
	info.size = 320.f;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);
	const auto image = m_world.resource_manager().image("grid-background.png");
	auto ship = m_ship.lock();
	if (ship) {
		for (int x = -10; x < 10; x++) {
			for (int y = -10; y < 10; y++) {
				mark::sprite::info info;
				info.image = image;
				info.pos = pos() + mark::vector<double>(x, y) * 32.0;
				info.size = 32.f;
				context.sprites[100].emplace_back(info);
			}
		}
		double top = 0.0;
		for (auto& cargo_ref : ship->containers()) {
			auto& cargo = cargo_ref.get();
			auto pos = mark::vector<double>(this->pos().x + 320.0, this->pos().y - 320.0 + top);
			cargo.render_contents(pos, context);
			top += cargo.interior_size().y * 16.0 + 32.0;
		}
		if (m_grabbed) {
			const auto size = static_cast<float>(std::max(m_grabbed->size().x, m_grabbed->size().y)) * 16.f;
			mark::sprite::info info;
			info.image = m_grabbed->thumbnail();
			info.pos = m_mousepos;
			info.size = size;
			context.sprites[100].emplace_back(info);
		}

		// display tooltips
		const auto relative = (m_mousepos - pos()) / 16.0;
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

					mark::sprite::info info;
					info.image = m_world.resource_manager().image("wall.png");
					info.pos = tooltip_pos + mark::vector<double>(150, 150),
					info.size = 300.f;
					context.sprites[100].emplace_back(info);

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
					const auto relative = m_mousepos - pos() + mark::vector<double>(-320 + 8, -top + 320 + 8);
					if (relative.y >= 0 && relative.y < size.y * 16) {
						const auto pick_pos = mark::floor(relative / 16.0);
						const auto module = cargo.module(pick_pos);
						if (module) {
							const auto description = module->describe();
							const auto module_size = mark::vector<double>(module->size()) * static_cast<double>(mark::module::size);
							const auto tooltip_pos = m_mousepos + mark::vector<double>(module_size.x, -module_size.y) / 2.0;

							mark::sprite::info info;
							info.image = m_world.resource_manager().image("wall.png");
							info.pos = tooltip_pos + mark::vector<double>(150, 150),
							info.size = 300.f;
							context.sprites[100].emplace_back(info);

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
		auto ship_ptr = std::dynamic_pointer_cast<mark::unit::modular>(m_world.find_one(pos(), 500.0, [ship](const mark::unit::base& unit) {
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
		move.pos = pos();
		modular->command(move);
		mark::command look;
		look.type = mark::command::type::guide;
		look.pos = pos() + mark::vector<double>(1, 0);
		modular->command(look);
	}
}

void mark::unit::landing_pad::command(const mark::command & command) {
	if (command.type == mark::command::type::activate && !command.release) {
		auto ship = m_ship.lock();
		if (ship) {
			ship->activate(this->shared_from_this());
			m_ship.reset();
		}
	} else if (command.type == mark::command::type::guide) {
		m_mousepos = command.pos;
	} else if (command.type == mark::command::type::move && !command.release) {
		auto ship = m_ship.lock();
		if (ship) {
			const auto relative = (command.pos - pos()) / 16.0;
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
					const auto relative = command.pos - pos() + mark::vector<double>(-320 + 8, -top + 320 + 8);
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
	} else if (!command.release) {
		if (command.type >= command::type::ability_0
			&& command.type <= command::type::ability_9
			|| command.type == command::type::shoot) {
			auto ship = m_ship.lock();
			if (ship) {
				const auto relative = (command.pos - pos()) / 16.0;
				const auto pick_pos = mark::floor(relative);
				ship->toggle_bind(command.type, pick_pos);
			}
		}
	}
}

auto mark::unit::landing_pad::collide(const mark::segment_t &) -> std::pair<mark::idamageable *, mark::vector<double>> {
	return { nullptr, { NAN, NAN } };
}

// Serialize / Deserialize

mark::unit::landing_pad::landing_pad(
	mark::world& world,
	const YAML::Node& node):
	mark::unit::base(world, node),
	m_image(world.resource_manager().image("landing-pad.png")) {

	if (node["grabbed"]) {
		m_grabbed = mark::module::deserialize(world.resource_manager(), node["grabbed"]);
	}
}

void mark::unit::landing_pad::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::landing_pad::type_name;
	this->serialize_base(out);
	const auto ship = m_ship.lock();
	if (ship) {
		out << Key << "ship_id" << Value << ship->id();
	}
	if (m_grabbed) {
		out << Key << "grabbed" << Value;
		m_grabbed->serialize(out);
	}
	out << EndMap;
}

void mark::unit::landing_pad::resolve_ref(
	const YAML::Node& node,
	const std::unordered_map<uint64_t, std::weak_ptr<mark::unit::base>>& units) {
	if (node["ship_id"]) {
		const auto ship_id = node["ship_id"].as<uint64_t>();
		m_ship = std::dynamic_pointer_cast<mark::unit::modular>(units.at(ship_id).lock());
	}
}
