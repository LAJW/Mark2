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


mark::world::world(mark::resource::manager& resource_manager)
	:m_resource_manager(resource_manager), m_map(resource_manager) {

	auto vessel = std::make_shared<mark::unit::modular>(*this, mark::vector<double>(0.0, 0.0), 10.f);
	auto core = std::make_unique<mark::module::core>(m_resource_manager);
	vessel->attach(std::move(core), { -1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { 1, -1 });
	vessel->attach(std::make_unique<mark::module::cargo>(m_resource_manager), { -3, -1 });
	vessel->attach(std::make_unique<mark::module::shield_generator>(m_resource_manager), { -1, 1 });
	vessel->attach(std::make_unique<mark::module::turret>(m_resource_manager), { -3, -3 });
	vessel->attach(std::make_unique<mark::module::turret>(m_resource_manager), { 1, -3 });
	m_camera_target = vessel;
	m_units.push_back(vessel);
	m_units.push_back(std::make_shared<mark::unit::minion>(*this, mark::vector<double>(20, 0)));
	m_units.push_back(std::make_shared<mark::unit::minion>(*this, mark::vector<double>(-20, 0)));
}

auto mark::world::map() const -> const mark::map&{
	return m_map;
}

auto mark::world::render(mark::vector<double> screen_size) const -> std::vector<mark::sprite> {
	std::vector<mark::sprite> sprites = m_map.render(
		m_camera - screen_size / 2.0 - mark::vector<double>(64, 64),
		m_camera + screen_size / 2.0 + mark::vector<double>(64, 64)
	);

	for (auto& unit : m_units) {
		auto socket_sprites = unit->render();
		sprites.insert(
			sprites.end(),
			std::make_move_iterator(socket_sprites.begin()),
			std::make_move_iterator(socket_sprites.end())
		);
	}


	return sprites;
}

auto mark::world::resource_manager() -> mark::resource::manager& {
	return m_resource_manager;
}

void mark::world::tick(double dt) {
	if (dt > 0.1) {
		dt = 0.1;
	}
	mark::tick_context context;
	context.dt = dt;
	for (auto& unit : m_units) {
		unit->tick(context);
	}
	auto last = std::remove_if(m_units.begin(), m_units.end(), [](const std::shared_ptr<mark::unit::base>& base) {
		return base->dead();
	});
	m_units.erase(last, m_units.end());
	m_units.insert(
		m_units.end(),
		std::make_move_iterator(context.units.begin()),
		std::make_move_iterator(context.units.end())
	);
	const auto camera_target = m_camera_target.lock();
	if (camera_target) {
		m_camera = m_camera + (camera_target->pos() - m_camera) * 2.0 * dt;
	}
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

void mark::world::command(const mark::command& command) {
	auto& camera_target = m_camera_target.lock();
	if (camera_target) {
		camera_target->command(command);
	}
}
