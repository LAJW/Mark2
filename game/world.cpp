#include "world.h"
#include "resource_manager.h"
#include "unit_base.h"
#include "unit_modular.h"
#include "module_cargo.h"
#include "module_core.h"
#include "module_shield_generator.h"
#include "module_turret.h"
#include "sprite.h"
#include <random>
#include "unit_minion.h"
#include "command.h"
#include "tick_context.h"
#include "module_cannon.h"
#include "module_mortar.h"
#include "unit_landing_pad.h"

auto create_ship(mark::resource::manager& resource_manager, mark::world& world) {
	auto vessel = std::make_shared<mark::unit::modular>(world, mark::vector<double>(0.0, 0.0), 10.f);
	auto core = std::make_unique<mark::module::core>(resource_manager);
	vessel->attach(std::move(core), { -1, -1 });
	auto cargo1 = std::make_unique<mark::module::cargo>(resource_manager);
	cargo1->drop({ 0, 0 }, std::make_unique<mark::module::shield_generator>(resource_manager));
	vessel->attach(std::move(cargo1), { -1, 1 });
	vessel->attach(std::make_unique<mark::module::cargo>(resource_manager), { -1, -3 });
	vessel->attach(std::make_unique<mark::module::shield_generator>(resource_manager), { 1, -1 });
	vessel->attach(std::make_unique<mark::module::turret>(resource_manager), { -3, -3 });
	vessel->attach(std::make_unique<mark::module::turret>(resource_manager), { -3, 1 });
	vessel->attach(std::make_unique<mark::module::cannon>(resource_manager), { -1, 3 });
	vessel->attach(std::make_unique<mark::module::cannon>(resource_manager), { -1, -5 });
	vessel->attach(std::make_unique<mark::module::mortar>(resource_manager), { -3, -5 });
	vessel->attach(std::make_unique<mark::module::mortar>(resource_manager), { -3, 3 });
	return vessel;
}

mark::world::world(mark::resource::manager& resource_manager)
	:m_resource_manager(resource_manager), m_map(resource_manager) {
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 320.0)) {
				m_units.push_back(std::make_shared<mark::unit::landing_pad>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end;
			}
		}
	}
	end:;
	auto vessel = create_ship(resource_manager, *this);
	vessel->team(1);
	m_camera_target = vessel;
	m_units.push_back(vessel);
	// m_units.push_back(create_ship(resource_manager, *this));
	mark::command command;
	command.type = mark::command::type::ai;
	// m_units.back()->command(command);
	// m_units.push_back(std::make_shared<mark::unit::minion>(*this, mark::vector<double>(20, 0)));
	// m_units.push_back(std::make_shared<mark::unit::minion>(*this, mark::vector<double>(-20, 0)));
}

auto mark::world::map() const -> const mark::map&{
	return m_map;
}

auto mark::world::resource_manager() -> mark::resource::manager& {
	return m_resource_manager;
}

auto mark::world::tick(double dt, mark::vector<double> screen_size) -> std::map<int, std::vector<mark::sprite>> {
	if (dt > 0.1) {
		dt = 0.1;
	}
	mark::tick_context context;
	std::vector<mark::sprite> sprites = m_map.render(
		m_camera - screen_size / 2.0 - mark::vector<double>(64, 64),
		m_camera + screen_size / 2.0 + mark::vector<double>(64, 64)
	);
	context.sprites[0].insert(
		context.sprites[0].end(),
		std::make_move_iterator(sprites.begin()),
		std::make_move_iterator(sprites.end())
	);

	context.dt = dt;
	for (auto& unit : m_units) {
		unit->tick(context);
	}
	for (auto& particle : m_particles) {
		particle.tick(dt, context.sprites);
	}
	// Add/Remove units
	{
		auto last = std::remove_if(m_units.begin(), m_units.end(), [](const std::shared_ptr<mark::unit::base>& base) {
			return base->dead();
		});
		m_units.erase(last, m_units.end());
		m_units.insert(
			m_units.end(),
			std::make_move_iterator(context.units.begin()),
			std::make_move_iterator(context.units.end())
		);
	}
	// Add/remove particles
	{
		auto last = std::remove_if(
			m_particles.begin(),
			m_particles.end(),
			[](const mark::particle& particle) {
			return particle.dead();
		});
		m_particles.erase(last, m_particles.end());
		m_particles.insert(
			m_particles.end(),
			std::make_move_iterator(context.particles.begin()),
			std::make_move_iterator(context.particles.end())
		);
	}

	const auto camera_target = m_camera_target.lock();
	if (camera_target) {
		m_camera = m_camera + (camera_target->pos() - m_camera) * 2.0 * dt;
	}
	return context.sprites;
}

auto mark::world::find(mark::vector<double> pos, double radius) -> std::vector<std::shared_ptr<mark::unit::base>> {
	std::vector<std::shared_ptr<mark::unit::base>> out;
	for (auto& unit : m_units) {
		if (mark::length(unit->pos() - pos) < radius) {
			out.push_back(unit);
		}
	}
	return out;
}

auto mark::world::find(mark::vector<double> pos, double radius,
	const std::function<bool(const mark::unit::base&)>& pred)
	-> std::vector<std::shared_ptr<mark::unit::base>> {
	std::vector<std::shared_ptr<mark::unit::base>> out;
	for (auto& unit : m_units) {
		if (mark::length(unit->pos() - pos) < radius && pred(*unit)) {
			out.push_back(unit);
		}
	}
	return out;
}

auto mark::world::find_one(mark::vector<double> pos, double radius,
	const std::function<bool(const mark::unit::base&)>& pred)
	-> std::shared_ptr<mark::unit::base> {
	for (auto& unit : m_units) {
		if (mark::length(unit->pos() - pos) < radius && pred(*unit)) {
			return unit;
		}
	}
	return nullptr;
}

void mark::world::command(const mark::command& command) {
	auto& camera_target = m_camera_target.lock();
	if (camera_target) {
		camera_target->command(command);
	}
}
