#include "stdafx.h"
#include "algorithm.h"
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
#include "world.h"
#include "world_stack.h"

mark::world::world(
	world_stack& stack,
	resource::manager& resource_manager,
	const bool empty,
	const bool inital)
	: m_resource_manager(resource_manager)
	, m_map(empty
		? map::make_square(resource_manager)
		: map::make_cavern(resource_manager))
	, image_bar(resource_manager.image("bar.png"))
	, image_font(resource_manager.image("font.png"))
	, image_stun(resource_manager.image("stun.png"))
	, m_stack(stack)
{
	const auto& templates = stack.templates();
	if (empty) {
		return;
	}
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			if (m_map.traversable(vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<unit::landing_pad>(*this, vector<double>(32 * (x - 500), 32 * (y - 500))));
				goto end;
			}
		}
	}
	end:;
	for (int x = 999; x >= 0; x--) {
		for (int y = 999; y >= 0; y--) {
			if (m_map.traversable(vector<double>(32 * (x - 500), 32 * (y - 500)), 100.0)) {
				m_units.push_back(std::make_shared<unit::gate>(
					*this, vector<double>(32 * (x - 500), 32 * (y - 500)), false));
				goto end2;
			}
		}
	}
	end2:;
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1000; y++) {
			const auto pos = vector<double>(32 * (x - 500), 32 * (y - 500));
			if (m_map.traversable(pos, 64.0) && this->find(pos, 320.0).empty()) {
				// m_units.push_back(std::make_shared<unit::minion>(*this, pos));
				mark::command command;
				command.type = command::type::ai;
				m_units.push_back(unit::deserialise(*this, templates.at("ship")));
				m_units.back()->pos(pos);
				m_units.back()->command(command);
			}
		}
	}
	if (inital) {
		auto vessel = std::dynamic_pointer_cast<unit::modular>(
			unit::deserialise(*this, templates.at("ship")));
		vessel->ai(false);
		mark::command command;
		command.release = true;
		command.type = command::type::move;
		vessel->command(command);
		vessel->team(1);
		vessel->pos({ 0., 0. });
		m_camera_target = vessel;
		m_units.push_back(vessel);
	} else {
		m_units.push_back(std::make_shared<unit::gate>(
			*this, vector<double>(), true));
	}
}

mark::world::~world() = default;

auto mark::world::map() const -> const mark::map&
{ return m_map; }

auto mark::world::resource_manager() -> resource::manager&
{ return m_resource_manager; }

void mark::world::tick(
	tick_context& context, vector<double> screen_size)
{
	if (context.dt > 0.1) {
		context.dt = 0.1;
	}
	const auto camera = vmap(m_camera, std::round);
	const auto offset = screen_size / 2.0
		+ vector<double>(map::tile_size, map::tile_size) * 2.;
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
		auto last = std::remove_if(
			m_units.begin(), m_units.end(), [&context](auto& unit) {
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
			[](const particle& particle) {
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

void mark::world::command(const mark::command_any& command)
{
	if (auto camera_target = m_camera_target.lock()) {
		camera_target->command(command);
	}
}

void mark::world::target(const std::shared_ptr<unit::base>& target)
{ m_camera_target = target; }

auto mark::world::target() -> std::shared_ptr<unit::base>
{ return m_camera_target.lock(); }

auto mark::world::target() const -> std::shared_ptr<const unit::base>
{ return m_camera_target.lock(); }

void mark::world::attach(const std::shared_ptr<mark::unit::base>& unit)
{
	if (std::find(m_units.cbegin(), m_units.cend(), unit) == m_units.cend()) {
		m_units.push_back(unit);
	}
	unit->m_world=*this;
}

auto mark::world::collide(const segment_t& ray)
	-> std::variant<std::monostate, vector<double>, collision_type>
{
	auto maybe_min = m_map.collide(ray);
	double min_length = maybe_min
		? length(ray.first - maybe_min.value())
		: INFINITY;
	interface::damageable* out = nullptr;
	for (auto& unit_ : m_units) {
		if (const auto unit = dynamic_cast<unit::damageable*>(unit_.get())) {
			if (const auto result = unit->collide(ray)) {
				auto[damageable, pos] = *result;
				const auto length = mark::length(ray.first - pos);
				if (length < min_length) {
					min_length = length;
					maybe_min = pos;
					out = &damageable.get();
				}
			}
		}
	}
	if (out) {
		return std::make_pair(std::ref(*out), *maybe_min);
	} else if (maybe_min) {
		return *maybe_min;
	}
	return std::monostate();
}

auto mark::world::collide(vector<double> center, float radius)
	-> std::vector<std::reference_wrapper<interface::damageable>>
{
	std::vector<std::reference_wrapper<interface::damageable>> out;
	for (auto& unit_ : m_units) {
		if (const auto unit
			= dynamic_cast<unit::damageable*>(unit_.get())) {
			const auto tmp = unit->collide(center, radius);
			std::copy(tmp.begin(), tmp.end(), std::back_inserter(out));
		}
	}
	return out;
}

auto mark::world::damage(world::damage_info& info)
	-> std::optional<vector<double>>
{
	assert(info.context);
	const auto result = this->collide(info.segment);
	if (std::holds_alternative<std::monostate>(result)) {
		return { };
	}
	const auto[pos, connected] = [&]() {
		if (const auto pos = std::get_if<vector<double>>(&result)) {
			return std::make_pair(*pos, true);
		}
		const auto&[damageable, pos] = std::get<collision_type>(result);
		info.damage.pos = pos;
		return std::make_pair(pos, damageable.get().damage(info.damage));
	}();
	if (!connected) {
		return { };
	}
	if (info.aoe_radius > 0.f) {
		const auto damageables = this->collide(pos, info.aoe_radius);
		for (const auto aoe_damageable : damageables) {
			info.damage.pos = pos;
			aoe_damageable.get().damage(info.damage);
		}
	}
	return pos;
}

// Serializer / Deserializer

mark::world::world(
	world_stack& stack,
	resource::manager& rm,
	const YAML::Node& node)
	: m_resource_manager(rm)
	, m_map(rm, node["map"])
	, m_camera(
		node["camera"]["x"].as<double>(),
		node["camera"]["y"].as<double>())
	, image_bar(rm.image("bar.png"))
	, image_font(rm.image("font.png"))
	, image_stun(rm.image("stun.png"))
	, m_stack(stack)
{
	std::unordered_map<uint64_t, std::weak_ptr<unit::base>> unit_map;
	uint64_t camera_target_id = node["camera_target_id"].as<uint64_t>();
	for (const auto& unit_node : node["units"]) {
		m_units.push_back(unit::deserialise(*this, unit_node));
		const auto unit_id = unit_node["id"].as<uint64_t>();
		unit_map.emplace(unit_id, m_units.back());
	}
	for (const auto& unit_node : node["units"]) {
		const auto unit_id = unit_node["id"].as<uint64_t>();
		unit_map[unit_id].lock()->resolve_ref(unit_node, unit_map);
	}
	m_camera_target = unit_map.at(camera_target_id);

}

void mark::world::next()
{ m_stack.next(); }

void mark::world::prev()
{ m_stack.prev(); }

void mark::world::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;

	out << Key << "type" << Value << "world";

	const auto camera_target = m_camera_target.lock();
	if (camera_target) {
		out << Key << "camera_target_id"
			<< Value << reinterpret_cast<size_t>(camera_target.get());
	}

	out << Key << "camera" << Value << BeginMap;
	out << Key << "x" << Value << m_camera.x;
	out << Key << "y" << Value << m_camera.y;
	out << EndMap;

	out << Key << "units" << Value << BeginSeq;
	for (const auto& unit : m_units) {
		unit->serialise(out);
	}
	out << EndSeq;

	out << Key << "map" << Value;
	m_map.serialise(out);

	out << EndMap;
}
