#include "stdafx.h"
#include "world.h"
#include "algorithm.h"
#include "command.h"
#include "map.h"
#include "module/battery.h"
#include "module/cannon.h"
#include "module/cargo.h"
#include "module/core.h"
#include "module/energy_generator.h"
#include "module/engine.h"
#include "module/flamethrower.h"
#include "module/shield_generator.h"
#include "module/turret.h"
#include "particle.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit/base.h"
#include "unit/damageable.h"
#include "unit/gate.h"
#include "unit/landing_pad.h"
#include "unit/minion.h"
#include "unit/modular.h"
#include "world_stack.h"

namespace {
const auto voxel_dim = mark::vector<std::size_t>{50, 50};
} // namespace

mark::world::world(resource::manager& rm)
	: m_resource_manager(rm)
	, m_map(std::make_unique<mark::map>(map::make_square(rm)))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({0, 0}),
		  m_map->map_to_world(vector<int>{m_map->size()}))
	, image_bar(rm.image("bar.png"))
	, image_font(rm.image("font.png"))
	, image_stun(rm.image("stun.png"))
{
}

// Helper function for finding positions for key locations using different
// over-the-area iterations
template <typename T>
static auto find_pos(
	const mark::map& map,
	mark::vector<int> size,
	double object_radius,
	T strategy)
{
	using namespace mark;
	assert(size.x > 0);
	assert(size.y > 0);
	let center = size / 2;
	return strategy(
		[&](const vector<int>& point) -> std::optional<vector<double>> {
			let pos = vector<double>(point - center) * map::tile_size;
			if (map.traversable(pos, object_radius)) {
				return pos;
			}
			return {};
		});
}

// Find a position matching gate size in the top left corner
static auto find_gate_pos(const mark::map& map, mark::vector<int> size)
	-> std::optional<mark::vector<double>>
{
	return find_pos(
		map,
		size,
		mark::unit::gate::radius,
		[&](auto proc) -> std::optional<mark::vector<double>> {
			for (let point : mark::range(size)) {
				if (let result = proc(point)) {
					return *result;
				}
			}
			return {};
		});
}

// Find a position matching gate size in the bottom right corner
static auto find_landing_pad_pos(const mark::map& map, mark::vector<int> size)
	-> std::optional<mark::vector<double>>
{
	return find_pos(
		map,
		size,
		mark::unit::landing_pad::radius,
		[&](auto proc) -> std::optional<mark::vector<double>> {
			for (int x = size.x - 1; x >= 0; --x) {
				for (int y = size.y - 1; y >= 0; --y) {
					if (let result = proc({x, y})) {
						return *result;
					}
				}
			}
			return {};
		});
}

mark::world::world(
	world_stack& stack,
	resource::manager& resource_manager,
	bool initial)
	: m_resource_manager(resource_manager)
	, m_map(std::make_unique<mark::map>(map::make_cavern(resource_manager)))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({0, 0}),
		  m_map->map_to_world(vector<int>{m_map->size()}))
	, image_bar(resource_manager.image("bar.png"))
	, image_font(resource_manager.image("font.png"))
	, image_stun(resource_manager.image("stun.png"))
	, m_stack(&stack)
{
	let map_size = vector<int>(1000, 1000);
	let spawn_ship = [&]() {
		return std::dynamic_pointer_cast<unit::modular>(
			unit::deserialise(*this, stack.blueprints().at("ship")));
	};
	let spawn_gate = [&](vector<double> pos, bool inverted) {
		m_units.push_back(std::make_shared<unit::gate>([&] {
			unit::gate::info _;
			_.world = this;
			_.pos = pos;
			_.inverted = inverted;
			return _;
		}()));
	};
	let gate_spawn_point = find_gate_pos(map(), map_size);
	assert(gate_spawn_point.has_value());
	spawn_gate(*gate_spawn_point, false);
	m_units.push_back(std::make_shared<unit::landing_pad>([&] {
		let landing_pad_spawn_point = find_landing_pad_pos(map(), map_size);
		assert(gate_spawn_point.has_value());
		unit::gate::info _;
		_.world = this;
		_.pos = *landing_pad_spawn_point;
		return _;
	}()));
	let ship_radius = [&] {
		let unit = spawn_ship();
		return unit->radius();
	}();

	for (let point : range(map_size)) {
		let pos = m_map->map_to_world(point);
		if (m_map->traversable(pos, ship_radius) &&
			this->find(pos, ship_radius * 4.).empty()) {
			/*
			auto unit = spawn_ship();
			unit->pos(pos);
			unit->ai(true);
			m_units.push_back(move(unit)); */

			auto new_unit = std::make_shared<unit::minion>([&] {
				unit::minion::info _;
				_.pos = pos;
				_.world = this;
				return _;
			}());
			m_units.push_back(new_unit);
			// It's important to keep updating the spatial partition so that
			// `find` works as expected
			m_space_bins.at(compute_index(m_space_bins, new_unit->pos()))
				.emplace_back(new_unit);
		}
	}
	if (initial) {
		auto vessel = spawn_ship();
		vessel->ai(false);
		vessel->command(command::move{vector<double>()});
		vessel->team(1);
		vessel->pos({0., 0.});
		m_camera_target = vessel;
		m_units.push_back(move(vessel));
	}
	else {
		spawn_gate({0, 0}, true);
	}
}

mark::world::~world() = default;

auto mark::world::map() const -> const mark::map& { return *m_map; }

auto mark::world::resource_manager() -> resource::manager&
{
	return m_resource_manager;
}

void mark::world::tick(tick_context& context, vector<double> screen_size)
{
	if (context.dt > 0.1) {
		context.dt = 0.1;
	}
	m_camera_adsr.tick(context.dt);
	m_camera_x_lfo.tick(context.dt);
	m_camera_y_lfo.tick(context.dt);

	{
		let camera = vmap(m_camera, std::round);
		let offset = screen_size / 2.0 +
			vector<double>(map::tile_size, map::tile_size) * 2.;
		m_map->tick(camera - offset, camera + offset, context);
	}

	// update the spatial partition
	divide_space(begin(m_units), end(m_units), m_space_bins);

	for (auto& unit : m_units) {
		// ticking can damage other units and they may become dead in the
		// process
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
		let last = remove_if(begin(m_units), end(m_units), [&](auto& unit) {
			if (unit->dead()) {
				unit->on_death(context);
				return true;
			}
			return false;
		});
		m_units.erase(last, m_units.end());
		m_units.insert(
			m_units.end(),
			make_move_iterator(begin(context.units)),
			make_move_iterator(end(context.units)));
	}
	// Add/remove particles
	{
		let last = remove_if(
			begin(m_particles), end(m_particles), [](const particle& particle) {
				return particle.dead();
			});
		m_particles.erase(last, end(m_particles));
		m_particles.insert(
			end(m_particles),
			make_move_iterator(begin(context.particles)),
			make_move_iterator(end(context.particles)));
	}
	if (context.crit == true) {
		m_camera_adsr.trigger();
	}

	if (let camera_target = m_camera_target.lock()) {
		constexpr let T = .5;
		let target_pos = camera_target->pos();
		let diff = target_pos - m_camera;
		let dist = length(diff);
		if (target_pos != m_prev_target_pos) {
			m_prev_target_pos = target_pos;
			m_a = 2. * dist / T / T;
			m_camera_velocity = m_a * T;
		}
		if (dist != 0.) {
			let dir = diff / dist;
			m_camera_velocity -= m_a * context.dt;
			if (m_camera_velocity * context.dt < dist &&
				m_camera_velocity > 0) {
				m_camera += m_camera_velocity * dir * context.dt;
			}
			else {
				m_camera = target_pos;
				m_camera_velocity = 0.;
				m_a = 0;
			}
		}
	}
}

void mark::world::command(const mark::command::any& command)
{
	if (auto camera_target = m_camera_target.lock()) {
		camera_target->command(command);
	}
}

void mark::world::target(const std::shared_ptr<unit::base>& target)
{
	m_camera_target = target;
}

auto mark::world::target() -> std::shared_ptr<unit::base>
{
	return m_camera_target.lock();
}

auto mark::world::target() const -> std::shared_ptr<const unit::base>
{
	return m_camera_target.lock();
}

void mark::world::attach(const std::shared_ptr<mark::unit::base>& unit)
{
	if (std::find(m_units.cbegin(), m_units.cend(), unit) == m_units.cend()) {
		m_units.push_back(unit);
	}
	unit->m_world = *this;
}

auto mark::world::collide(const segment_t& ray)
	-> std::pair<std::deque<collision_type>, std::optional<vector<double>>>
{
	auto map_collision = m_map->collide(ray);
	const double min_length =
		map_collision ? length(*map_collision - ray.first) : INFINITY;
	std::deque<collision_type> collisions;
	for (auto& unit_ : m_units) {
		if (let unit = dynamic_cast<unit::damageable*>(unit_.get())) {
			if (let result = unit->collide(ray)) {
				auto [damageable, pos] = *result;
				let length = mark::length(pos - ray.first);
				if (length < min_length) {
					collisions.push_back({damageable, pos});
				}
			}
		}
	}
	sort(begin(collisions), end(collisions), [&](let& a, let& b) {
		return length(a.second - ray.first) < length(b.second - ray.first);
	});
	return {move(collisions), map_collision};
}

auto mark::world::collide(vector<double> center, float radius)
	-> std::vector<std::reference_wrapper<interface::damageable>>
{
	std::vector<std::reference_wrapper<interface::damageable>> out;
	for (auto& unit_ : m_units) {
		if (let unit = dynamic_cast<unit::damageable*>(unit_.get())) {
			let tmp = unit->collide(center, radius);
			copy(begin(tmp), end(tmp), back_inserter(out));
		}
	}
	return out;
}

auto mark::world::damage(world::damage_info info)
	-> std::pair<std::vector<vector<double>>, bool>
{
	assert(info.context);
	auto [potential_collisions, crash_pos] = this->collide(info.segment);
	if (potential_collisions.empty() && !crash_pos) {
		return {};
	}
	std::vector<vector<double>> collisions;
	while (!potential_collisions.empty()) {
		let & [ damageable, pos ] = potential_collisions.front();
		info.damage.pos = pos;
		if (damageable.get().damage(info.damage)) {
			collisions.push_back(pos);
			if (collisions.size() >= info.piercing) {
				break;
			}
		}
		potential_collisions.pop_front();
	}
	if (crash_pos &&
		(info.piercing < collisions.size() || collisions.empty())) {
		collisions.push_back(*crash_pos);
	}
	if (info.aoe_radius > 0.f) {
		for (let& collision : collisions) {
			let damageables = this->collide(collision, info.aoe_radius);
			for (let aoe_damageable : damageables) {
				info.damage.pos = collision;
				aoe_damageable.get().damage(info.damage);
			}
		}
	}
	return {collisions, crash_pos.has_value()};
}

// Serializer / Deserializer

mark::world::world(
	world_stack& stack,
	resource::manager& rm,
	const YAML::Node& node)
	: m_resource_manager(rm)
	, m_map(std::make_unique<mark::map>(rm, node["map"]))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({0, 0}),
		  m_map->map_to_world(vector<int>{m_map->size()}))
	, m_camera(
		  node["camera"]["x"].as<double>(),
		  node["camera"]["y"].as<double>())
	, image_bar(rm.image("bar.png"))
	, image_font(rm.image("font.png"))
	, image_stun(rm.image("stun.png"))
	, m_stack(&stack)
{
	std::unordered_map<uint64_t, std::weak_ptr<unit::base>> unit_map;
	let camera_target_id = node["camera_target_id"].as<uint64_t>();
	for (let& unit_node : node["units"]) {
		m_units.push_back(unit::deserialise(*this, unit_node));
		let unit_id = unit_node["id"].as<uint64_t>();
		unit_map.emplace(unit_id, m_units.back());
	}
	for (let& unit_node : node["units"]) {
		let unit_id = unit_node["id"].as<uint64_t>();
		unit_map[unit_id].lock()->resolve_ref(unit_node, unit_map);
	}
	m_camera_target = unit_map.at(camera_target_id);
}

void mark::world::next() { m_stack->next(); }

void mark::world::prev() { m_stack->prev(); }

void mark::world::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;

	out << Key << "type" << Value << "world";

	let camera_target = m_camera_target.lock();
	if (camera_target) {
		out << Key << "camera_target_id" << Value
			<< reinterpret_cast<size_t>(camera_target.get());
	}

	out << Key << "camera" << Value << BeginMap;
	out << Key << "x" << Value << m_camera.x;
	out << Key << "y" << Value << m_camera.y;
	out << EndMap;

	out << Key << "units" << Value << BeginSeq;
	for (let& unit : m_units) {
		unit->serialise(out);
	}
	out << EndSeq;

	out << Key << "map" << Value;
	m_map->serialise(out);

	out << EndMap;
}

auto mark::world::blueprints() const
	-> const std::unordered_map<std::string, YAML::Node>&
{
	return m_stack->blueprints();
}
