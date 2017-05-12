#include <algorithm>
#include "world.h"
#include "resource_manager.h"
#include "unit_base.h"
#include "unit_modular.h"
#include "module_cargo.h"
#include "module_core.h"
#include "module_shield_generator.h"
#include "module_turret.h"
#include "sprite.h"
#include "unit_minion.h"
#include "command.h"
#include "tick_context.h"
#include "module_cannon.h"
#include "unit_landing_pad.h"
#include "module_energy_generator.h"
#include "module_battery.h"
#include "module_flamethrower.h"
#include "module_engine.h"
#include <assert.h>

auto make_turret(mark::resource::manager& resource_manager) {
	mark::module::turret::info info;
	info.resource_manager = &resource_manager;
	info.seek_radius = 0.f;
	info.velocity = 1000.f;
	info.projectile_angular_velocity = 30.f;
	info.guided = true;
	info.rate_of_fire = 10.f;
	info.angular_velocity = 360.f;
	info.cone_curve = mark::curve::linear;
	info.cone = 10.f;
	info.projectile_count = 1;
	info.heat_per_shot = 2.f;
	return std::make_unique<mark::module::turret>(info);
}

auto make_mortar(mark::resource::manager& resource_manager) {
	mark::module::turret::info info;
	info.resource_manager = &resource_manager;
	info.seek_radius = 500.f;
	info.projectile_count = 3;
	info.velocity = 500.f;
	info.projectile_angular_velocity = 30.f;
	info.guided = false;
	info.rate_of_fire = .3f;
	info.angular_velocity = 0.f;
	info.cone = 30.f;
	info.heat_per_shot = 30.f;
	return std::make_unique<mark::module::turret>(info);
}

auto create_ship(mark::resource::manager& resource_manager, mark::world& world) {
	auto vessel = std::make_shared<mark::unit::modular>(world, mark::vector<double>(0.0, 0.0), 10.f);
	auto core = std::make_unique<mark::module::core>(resource_manager);
	vessel->attach(std::move(core), { -1, -1 });
	auto cargo1 = std::make_unique<mark::module::cargo>(resource_manager);
	cargo1->drop({ 0, 0 }, std::make_unique<mark::module::shield_generator>(resource_manager));
	vessel->attach(std::move(cargo1), { -1, 1 });
	vessel->attach(std::make_unique<mark::module::cargo>(resource_manager), { -1, -3 });
	vessel->attach(std::make_unique<mark::module::shield_generator>(resource_manager), { 1, -1 });
	vessel->attach(std::make_unique<mark::module::flamethrower>(resource_manager), { 3, -3 });
	vessel->attach(std::make_unique<mark::module::flamethrower>(resource_manager), { 3, 1 });
	vessel->attach(make_turret(resource_manager), { -3, -3 });
	vessel->attach(make_turret(resource_manager), { -3, 1 });
	vessel->attach(std::make_unique<mark::module::cannon>(resource_manager), { -1, 3 });
	vessel->attach(std::make_unique<mark::module::cannon>(resource_manager), { -1, -5 });
	vessel->attach(make_mortar(resource_manager), { -3, -5 });
	vessel->attach(make_mortar(resource_manager), { -3, 3 });
	vessel->attach(std::make_unique<mark::module::energy_generator>(resource_manager), { -3, -1 });
	vessel->attach(std::make_unique<mark::module::battery>(resource_manager), { -5, -1 });
	vessel->attach(std::make_unique<mark::module::engine>(resource_manager), { -7, -3 });
	vessel->attach(std::make_unique<mark::module::engine>(resource_manager), { -7, 1 });
	return vessel;
}

mark::world::world(mark::resource::manager& resource_manager):
	m_resource_manager(resource_manager),
	m_map(mark::map::make_cavern(resource_manager)) {
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<mark::unit::landing_pad>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end;
			}
		}
	}
	end:;
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			const auto pos = mark::vector<double>(32 * (x - 500), 32 * (y - 500));
			if (m_map.traversable(pos, 64.0) && this->find(pos, 320.0).empty()) {
				m_units.push_back(std::make_shared<mark::unit::minion>(*this, pos));
			}
		}
	}
	auto vessel = create_ship(resource_manager, *this);
	vessel->team(1);
	m_camera_target = vessel;
	m_units.push_back(vessel);
	// m_units.push_back(create_ship(resource_manager, *this));
	mark::command command;
	command.type = mark::command::type::ai;
	// m_units.back()->command(command);
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
	mark::tick_context context(m_resource_manager);
	std::vector<mark::sprite> sprites = m_map.render(
		m_camera - screen_size / 2.0 - mark::vector<double>(64, 64),
		m_camera + screen_size / 2.0 + mark::vector<double>(64, 64)
	);
	context.sprites[-1].insert(
		context.sprites[-1].end(),
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
		auto last = std::remove_if(m_units.begin(), m_units.end(), [&context](std::shared_ptr<mark::unit::base>& unit) {
			const auto dead = unit->dead();
			if (dead) {
				unit->on_death(context);
			}
			return dead;
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

void mark::world::target(const std::shared_ptr<mark::unit::base>& target) {
	m_camera_target = target;
}

auto mark::world::target() -> std::shared_ptr<mark::unit::base> {
	return m_camera_target.lock();
}

auto mark::world::target() const -> std::shared_ptr<const mark::unit::base> {
	return m_camera_target.lock();
}

auto mark::world::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable *, mark::vector<double>> {
	auto min = m_map.collide(ray);
	double min_length = !std::isnan(min.x)
		? mark::length(ray.first - min)
		: INFINITY;
	mark::idamageable* damageable = nullptr;
	for (auto& unit : m_units) {
		auto result = unit->collide(ray);
		if (result.first) {
			const auto length = mark::length(ray.first - result.second);
			if (length < min_length) {
				min_length = length;
				min = result.second;
				damageable = result.first;
			}
		}
	}
	return { damageable, min };

}

auto mark::world::collide(mark::vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<mark::idamageable>> {

	std::vector<std::reference_wrapper<mark::idamageable>> out;
	for (auto& unit : m_units) {
		auto tmp = unit->collide(center, radius);
		std::copy(
			tmp.begin(),
			tmp.end(),
			std::back_inserter(out)
		);
	}
	return out;
}

auto mark::world::damage(mark::world::damage_info& info) -> std::pair<mark::vector<double>, bool> {
	assert(info.context);
	const auto collision = this->collide(info.segment);
	if (!std::isnan(collision.second.x)) {
		auto dead = false;
		if (collision.first) {
			if (collision.first->damage(info.damage) && info.piercing == 1) {
				dead = true;
			}
		} else {
			dead = true;
		}
		if (info.aoe_radius >= 0.f) {
			auto damageables = this->collide(collision.second, info.aoe_radius);
			for (auto damageable : damageables) {
				damageable.get().damage(info.damage);
			}
		}
		return { collision.second, dead };
	}
	return { { NAN, NAN }, false };
}
