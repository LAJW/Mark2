#include "stdafx.h"
#include "command.h"
#include "map.h"
#include "module_cargo.h"
#include "module_core.h"
#include "module_shield_generator.h"
#include "module_turret.h"
#include "module_energy_generator.h"
#include "module_battery.h"
#include "module_flamethrower.h"
#include "module_engine.h"
#include "module_cannon.h"
#include "particle.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_base.h"
#include "unit_damageable.h"
#include "unit_gate.h"
#include "unit_landing_pad.h"
#include "unit_minion.h"
#include "unit_modular.h"
#include "instance.h"

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

auto to_base(std::unique_ptr<mark::module::base> ptr) {
	return ptr;
}

auto create_ship(mark::resource::manager& resource_manager, mark::instance& instance, mark::vector<double> pos = { 0, 0 }) {
	auto vessel = std::make_shared<mark::unit::modular>(instance, pos, 10.f);
	auto core = to_base(std::make_unique<mark::module::core>(resource_manager));
	(void)vessel->attach({ -1, -1 }, core);
	auto cargo1 = std::make_unique<mark::module::cargo>(resource_manager);
	std::unique_ptr<mark::module::base> shield_generator = std::make_unique<mark::module::shield_generator>(resource_manager);
	(void)cargo1->attach({ 0, 0 }, shield_generator);
	(void)vessel->attach({ -1, 1 }, to_base(std::move(cargo1)));
	(void)vessel->attach({ -1, -3 }, to_base(std::make_unique<mark::module::cargo>(resource_manager)));
	(void)vessel->attach({ 1, -1 }, to_base(std::make_unique<mark::module::shield_generator>(resource_manager)));
	(void)vessel->attach({ 3, -3 }, to_base(std::make_unique<mark::module::flamethrower>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_1, { 3, -3 });
	(void)vessel->attach({ 3, 1 }, to_base(std::make_unique<mark::module::flamethrower>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_1, { 3, 1 });
	(void)vessel->attach({ -3, -3 }, to_base(make_turret(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::shoot, { -3, -3 });
	(void)vessel->attach({ -3, 1 }, to_base(make_turret(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::shoot, { -3, 1 });
	(void)vessel->attach({ -1, 3 }, to_base(std::make_unique<mark::module::cannon>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_2, { -1, 3 });
	(void)vessel->attach({ -1, -5 }, to_base(std::make_unique<mark::module::cannon>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_2, { -1, -5 });
	(void)vessel->attach({ -3, -5 }, to_base(make_mortar(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::shoot, { -3, -5 });
	(void)vessel->attach({ -3, 3 }, to_base(make_mortar(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::shoot, { -3, 3 });
	(void)vessel->attach({ -3, -1 }, to_base(std::make_unique<mark::module::energy_generator>(resource_manager)));
	(void)vessel->attach({ -5, -1 }, to_base(std::make_unique<mark::module::battery>(resource_manager)));
	(void)vessel->attach({ -7, -3 }, to_base(std::make_unique<mark::module::engine>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_3, { -7, -3 });
	(void)vessel->attach({ -7, 1 }, to_base(std::make_unique<mark::module::engine>(resource_manager)));
	(void)vessel->toggle_bind(mark::command::type::ability_3, { -7, 1 });
	return vessel;
}

mark::instance::instance(mark::resource::manager& resource_manager, const bool empty) :
	m_resource_manager(resource_manager),
	m_map(empty
		? mark::map::make_square(resource_manager)
		: mark::map::make_cavern(resource_manager)),
	image_bar(resource_manager.image("bar.png")),
	image_font(resource_manager.image("font.png")),
	image_stun(resource_manager.image("stun.png")) {
	if (empty) {
		return;
	}
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<mark::unit::landing_pad>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end;
			}
		}
	}
	end:;
	for (int x = 999; x >= 0; x--) {
		for (int y = 999; y >= 0; y--) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<mark::unit::gate>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end2;
			}
		}
	}
	end2:;
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			const auto pos = mark::vector<double>(32 * (x - 500), 32 * (y - 500));
			if (m_map.traversable(pos, 64.0) && this->find(pos, 320.0).empty()) {
				// m_units.push_back(std::make_shared<mark::unit::minion>(*this, pos));
				mark::command command;
				command.type = mark::command::type::ai;
				m_units.push_back(create_ship(resource_manager, *this, pos));
				m_units.back()->command(command);
			}
		}
	}
	auto vessel = create_ship(resource_manager, *this);
	vessel->team(1);
	m_camera_target = vessel;
	m_units.push_back(vessel);
}

mark::instance::~instance() = default;

auto mark::instance::map() const -> const mark::map&{
	return m_map;
}

auto mark::instance::resource_manager() -> mark::resource::manager& {
	return m_resource_manager;
}

void mark::instance::tick(
	mark::tick_context& context, mark::vector<double> screen_size)
{
	if (context.dt > 0.1) {
		context.dt = 0.1;
	}
	const auto camera = vmap(m_camera, std::round);
	const auto offset = screen_size / 2.0
		+ mark::vector<double>(map::tile_size, map::tile_size) * 2.;
	m_map.tick(camera - offset, camera + offset, context);

	for (auto& unit : m_units) {
		// ticking can damage other units and they may become dead in the process
		// dead unit is a unit due to be removed
		if (!unit->dead()) {
			unit->tick(context);
		}
	}
	for (auto& particle : m_particles) {
		particle.tick(context.dt, context.sprites);
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

	if (const auto camera_target = m_camera_target.lock()) {
		constexpr const auto T = .5;
		const auto target_pos = camera_target->pos();
		const auto diff = target_pos - m_camera;
		const auto dist = length(diff);
		if (target_pos != m_prev_target_pos) {
			m_prev_target_pos = target_pos;
			m_a = 2. * dist / T / T;
			m_camera_velocity = m_a * T;
		}
		if (dist != 0.) {
			const auto dir = diff / dist;
			m_camera_velocity -= m_a * context.dt;
			if (m_camera_velocity * context.dt < dist
				&& m_camera_velocity > 0) {
				m_camera += m_camera_velocity * dir * context.dt;
			} else {
				m_camera = target_pos;
				m_camera_velocity = 0.;
				m_a = 0;
			}
		}
	}
}

auto mark::instance::find(mark::vector<double> pos, double radius) -> std::vector<std::shared_ptr<mark::unit::base>> {
	std::vector<std::shared_ptr<mark::unit::base>> out;
	for (auto& unit : m_units) {
		if (mark::length(unit->pos() - pos) < radius) {
			out.push_back(unit);
		}
	}
	return out;
}

auto mark::instance::find(mark::vector<double> pos, double radius,
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

auto mark::instance::find_one(mark::vector<double> pos, double radius,
	const std::function<bool(const mark::unit::base&)>& pred)
	-> std::shared_ptr<mark::unit::base> {
	for (auto& unit : m_units) {
		if (mark::length(unit->pos() - pos) < radius && pred(*unit)) {
			return unit;
		}
	}
	return nullptr;
}

void mark::instance::command(const mark::command& command) {
	auto& camera_target = m_camera_target.lock();
	if (camera_target) {
		camera_target->command(command);
	}
}

void mark::instance::target(const std::shared_ptr<mark::unit::base>& target) {
	m_camera_target = target;
}

auto mark::instance::target() -> std::shared_ptr<mark::unit::base> {
	return m_camera_target.lock();
}

auto mark::instance::target() const -> std::shared_ptr<const mark::unit::base> {
	return m_camera_target.lock();
}

auto mark::instance::collide(const mark::segment_t& ray) ->
	std::pair<interface::damageable *, std::optional<mark::vector<double>>> {
	auto maybe_min = m_map.collide(ray);
	double min_length = maybe_min
		? mark::length(ray.first - maybe_min.value())
		: INFINITY;
	interface::damageable* out = nullptr;
	for (auto& unit_ : m_units) {
		if (const auto unit
			= dynamic_cast<mark::unit::damageable*>(unit_.get())) {
			auto[damageable, pos] = unit->collide(ray);;
			if (damageable) {
				const auto length = mark::length(ray.first - pos);
				if (length < min_length) {
					min_length = length;
					maybe_min = pos;
					out = damageable;
				}
			}

		}
	}
	return { out, maybe_min };
}

auto mark::instance::collide(mark::vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<interface::damageable>> {

	std::vector<std::reference_wrapper<interface::damageable>> out;
	for (auto& unit_ : m_units) {
		if (const auto unit
			= dynamic_cast<mark::unit::damageable*>(unit_.get())) {
			const auto tmp = unit->collide(center, radius);
			std::copy(tmp.begin(), tmp.end(), std::back_inserter(out));
		}
	}
	return out;
}

auto mark::instance::damage(mark::instance::damage_info& info) ->
	std::pair<std::optional<mark::vector<double>>, bool> {
	assert(info.context);
	const auto [ damageable, maybe_pos ] = this->collide(info.segment);
	if (maybe_pos) {
		const auto pos = maybe_pos.value();
		auto dead = false;
		if (info.piercing == 1 && damageable && damageable->damage(info.damage)
			|| !damageable) {
			dead = true;
		}
		if (info.aoe_radius >= 0.f) {
			const auto damageables = this->collide(pos, info.aoe_radius);
			for (const auto damageable : damageables) {
				damageable.get().damage(info.damage);
			}
		}
		return { pos, dead };
	}
	return { { }, false };
}

// Serializer / Deserializer

mark::instance::instance(mark::resource::manager& rm, const YAML::Node& node) :
	m_resource_manager(rm),
	m_map(rm, node["map"]),
	m_camera(node["camera"]["x"].as<double>(), node["camera"]["y"].as<double>()),
	image_bar(rm.image("bar.png")),
	image_font(rm.image("font.png")),
	image_stun(rm.image("stun.png")) {

	std::unordered_map<uint64_t, std::weak_ptr<mark::unit::base>> unit_map;
	uint64_t camera_target_id = node["camera_target_id"].as<uint64_t>();
	for (const auto& unit_node : node["units"]) {
		m_units.push_back(mark::unit::deserialize(*this, unit_node));
		const auto unit_id = unit_node["id"].as<uint64_t>();
		unit_map.emplace(unit_id, m_units.back());
	}
	for (const auto& unit_node : node["units"]) {
		const auto unit_id = unit_node["id"].as<uint64_t>();
		unit_map[unit_id].lock()->resolve_ref(unit_node, unit_map);
	}
	m_camera_target = unit_map.at(camera_target_id);

}

void mark::instance::next() {
	m_map = mark::map::make_cavern(m_resource_manager);
	auto camera_target = m_camera_target.lock();
	// TODO: Transfer old units and map to history
	m_units.clear();
	m_units.push_back(camera_target);
	camera_target->pos({ 0, 0 });
	m_camera = { 0, 0 };

	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<mark::unit::landing_pad>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end;
			}
		}
	}
	end:;
	for (int x = 999; x >= 0; x--) {
		for (int y = 999; y >= 0; y--) {
			if (m_map.traversable(mark::vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<mark::unit::gate>(*this, mark::vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end2;
			}
		}
	}
	end2:;
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			const auto pos = mark::vector<double>(32 * (x - 500), 32 * (y - 500));
			if (m_map.traversable(pos, 64.0) && this->find(pos, 320.0).empty()) {
				// m_units.push_back(std::make_shared<mark::unit::minion>(*this, pos));
				mark::command command;
				command.type = mark::command::type::ai;
				m_units.push_back(create_ship(m_resource_manager, *this, pos));
				m_units.back()->command(command);
			}
		}
	}
}

void mark::instance::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;

	out << Key << "type" << Value << "instance";

	const auto camera_target = m_camera_target.lock();
	if (camera_target) {
		out << Key << "camera_target_id" << Value << camera_target->id();
	}

	out << Key << "camera" << Value << BeginMap;
	out << Key << "x" << Value << m_camera.x;
	out << Key << "y" << Value << m_camera.y;
	out << EndMap;

	out << Key << "units" << Value << BeginSeq;
	for (const auto& unit : m_units) {
		unit->serialize(out);
	}
	out << EndSeq;

	out << Key << "map" << Value;
	m_map.serialize(out);

	out << EndMap;
}