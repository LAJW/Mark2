#include "world.h"
#include "algorithm.h"
#include "camera.h"
#include "command.h"
#include "map.h"
#include "particle.h"
#include "resource_manager.h"
#include "stdafx.h"
#include "unit/base.h"
#include "unit/damageable.h"
#include "unit/gate.h"
#include "unit/landing_pad.h"
#include "unit/minion.h"
#include "unit/modular.h"
#include "unit/projectile.h"
#include "update_context.h"
#include "world_stack.h"

namespace {
let voxel_dim = mark::vector<std::size_t>{ 50, 50 };
} // namespace

mark::world::world(resource::manager& rm, random& random)
	: m_resource_manager(rm)
	, m_map(std::make_unique<mark::map>(map::make_square(rm, random)))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({ 0, 0 }),
		  m_map->map_to_world(vi32{ m_map->size() }))
	, m_camera(std::make_unique<mark::camera>())
	, image_bar(rm.image("bar.png"))
	, image_font(rm.image("font.png"))
	, image_stun(rm.image("stun.png"))
{}

// Helper function for finding positions for key locations using different
// over-the-area iterations
template <typename T>
static auto find_pos(
	const mark::map& map,
	mark::vi32 size,
	double object_radius,
	T strategy)
{
	using namespace mark;
	assert(size.x > 0);
	assert(size.y > 0);
	let center = size / 2;
	return strategy([&](vi32 point) -> std::optional<vd> {
		let pos = vd(point - center) * map::tile_size;
		if (map.traversable(pos, object_radius)) {
			return pos;
		}
		return {};
	});
}

// Find a position matching gate size in the top left corner
static auto find_gate_pos(const mark::map& map, mark::vi32 size)
	-> std::optional<mark::vd>
{
	return find_pos(
		map,
		size,
		mark::unit::gate::size / 2.,
		[&](auto proc) -> std::optional<mark::vd> {
			for (let point : mark::range(size)) {
				if (let result = proc(point)) {
					return *result;
				}
			}
			return {};
		});
}

// Find a position matching gate size in the bottom right corner
static auto find_landing_pad_pos(const mark::map& map, mark::vi32 size)
	-> std::optional<mark::vd>
{
	return find_pos(
		map,
		size,
		mark::unit::landing_pad::size / 2,
		[&](auto proc) -> std::optional<mark::vd> {
			for (int x = size.x - 1; x >= 0; --x) {
				for (int y = size.y - 1; y >= 0; --y) {
					if (let result = proc({ x, y })) {
						return *result;
					}
				}
			}
			return {};
		});
}

static const std::vector<std::string> ship_blueprint_ids = {
	"ship",
	"laser-ship",
	"grenade-ship",
};

mark::world::world(
	world_stack& stack,
	resource::manager& resource_manager,
	random& random,
	bool initial)
	: m_resource_manager(resource_manager)
	, m_map(std::make_unique<mark::map>(
		  map::make_cavern(resource_manager, random)))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({ 0, 0 }),
		  m_map->map_to_world(vi32{ m_map->size() }))
	, image_bar(resource_manager.image("bar.png"))
	, image_font(resource_manager.image("font.png"))
	, image_stun(resource_manager.image("stun.png"))
	, m_camera(std::make_unique<mark::camera>())
	, m_stack(&stack)
{
	let constexpr spawn_ships = true;
	let map_size = vi32(1000, 1000);
	let spawn_ship = [&]() {
		let blueprint_id = random(size_t(0), ship_blueprint_ids.size() - 1);
		let ship_blueprint_id = ship_blueprint_ids[blueprint_id];
		let& ship_blueprint = stack.blueprints().at(ship_blueprint_id);
		return std::dynamic_pointer_cast<unit::modular>(
			unit::deserialize(*this, random, ship_blueprint).get());
	};
	let spawn_gate = [&](vd pos, bool inverted) {
		m_units.push_back(std::make_shared<unit::gate>([&] {
			unit::gate::info _;
			_.world = *this;
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
		_.world = *this;
		_.pos = *landing_pad_spawn_point;
		return _;
	}()));
	let ship_radius = [&] {
		let unit = spawn_ship();
		return unit->radius();
	}();

	for (let point : range(map_size)) {
		let pos = m_map->map_to_world(point);
		if (length(pos) < 1200 || !m_map->traversable(pos, ship_radius)
			|| !this->find(pos, ship_radius * 4.).empty()) {
			continue;
		}
		let enemy = [&]() -> shared_ptr<unit::base> {
			if (spawn_ships) {
				let unit = spawn_ship();
				unit->pos(pos);
				unit->ai(true);
				return unit;
			}
			return std::make_shared<unit::minion>([&] {
				unit::minion::info _;
				_.pos = pos;
				_.world = *this;
				return _;
			}());
		}();
		m_units.push_back(enemy);
		m_space_bins.at(compute_index(m_space_bins, enemy->pos()))
			.emplace_back(enemy);
	}
	if (initial) {
		auto vessel = spawn_ship();
		vessel->ai(false);
		vessel->command(command::move{ vd() });
		vessel->team(1);
		vessel->pos({ 0., 0. });
		m_camera->target(vessel);
		m_units.push_back(move(vessel));
	} else {
		spawn_gate({ 0, 0 }, true);
	}
}

mark::world::~world() = default;

auto mark::world::map() const -> const mark::map& { return *m_map; }

auto mark::world::camera() const -> vd { return m_camera->pos(); }

auto mark::world::resource_manager() -> resource::manager&
{
	return m_resource_manager;
}

void mark::world::update(update_context& context, vd screen_size)
{
	if (context.dt > 0.1) {
		context.dt = 0.1;
	}
	m_camera->update(context.dt);
	{
		let camera = vmap(m_camera->pos(), std::round);
		let offset =
			screen_size / 2.0 + vd(map::tile_size, map::tile_size) * 2.;
		m_map->update(camera - offset, camera + offset, context);
	}

	update_spatial_partition();

	for (auto& unit : m_units) {
		// updateing can damage other units and they may become dead in the
		// process
		// dead unit is a unit due to be removed
		if (!unit->dead()) {
			unit->update(context);
		}
	}
	for (auto& particle : m_particles) {
		particle.update(context.dt, context.sprites);
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
	if (context.crit) {
		m_camera->trigger();
	}
}

void mark::world::command(const mark::command::any& command)
{
	if (auto camera_target = m_camera->target()) {
		camera_target->command(command);
	}
}

void mark::world::target(const shared_ptr<unit::base>& target)
{
	m_camera->target(target);
}

auto mark::world::target() -> shared_ptr<unit::base>
{
	return m_camera->target();
}

auto mark::world::target() const -> shared_ptr<const unit::base>
{
	return m_camera->target();
}

void mark::world::attach(not_null<shared_ptr<mark::unit::base>> unit)
{
	if (std::find(m_units.cbegin(), m_units.cend(), unit) == m_units.cend()) {
		m_units.push_back(unit.get());
	}
	unit->m_world = *this;
}

auto mark::world::collide(const segment_t& ray) -> collide_result
{
	auto map_collision = m_map->collide(ray);
	const double max_length =
		map_collision ? length(map_collision->pos - ray.first) : INFINITY;
	std::deque<collision_type> collisions;
	let ray_center = ray.first + (ray.second - ray.first) / 2.;
	// TODO: Magical variable - space bins don't seem to respect radius.
	let ray_radius = 400. + length(ray.second - ray.first);
	for (auto& unit : this->find<unit::damageable>(ray_center, ray_radius)) {
		if (let result = unit->collide(ray)) {
			auto [damageable, pos] = *result;
			let length = mark::length(pos - ray.first);
			if (length < max_length) {
				collisions.push_back({ damageable, pos });
			}
		}
	}
	sort(begin(collisions), end(collisions), [&](let& a, let& b) {
		return length(a.pos - ray.first) < length(b.pos - ray.first);
	});
	return { move(collisions), map_collision };
}

auto mark::world::collide(vd center, double radius)
	-> std::vector<ref<interface::damageable>>
{
	std::vector<ref<interface::damageable>> out;
	for (auto& unit : this->find<unit::damageable>(center, radius)) {
		let tmp = unit->collide(center, radius);
		copy(begin(tmp), end(tmp), back_inserter(out));
	}
	return out;
}

void mark::world::update_spatial_partition()
{
	decltype(m_units) non_projectiles;
	non_projectiles.reserve(m_units.size());
	copy_if(
		begin(m_units),
		end(m_units),
		back_inserter(non_projectiles),
		[](let& base) {
			return !std::dynamic_pointer_cast<unit::projectile>(base.get());
		});
	divide_space(begin(non_projectiles), end(non_projectiles), m_space_bins);
}

auto mark::world::damage(world::damage_info info) -> damage_result
{
	assert(info.context);
	auto [unit_collisions, terrain_collision] = this->collide(info.segment);
	if (!info.aerial && unit_collisions.empty() && !terrain_collision) {
		return {};
	}
	std::vector<vd> collisions;
	while (!unit_collisions.empty()) {
		let & [ damageable, pos ] = unit_collisions.front();
		info.damage.pos = pos;
		if (damageable.get().damage(info.damage)) {
			collisions.push_back(pos);
			if (collisions.size() >= info.piercing) {
				break;
			}
		}
		unit_collisions.pop_front();
	}
	if (terrain_collision
		&& (info.piercing < collisions.size() || collisions.empty())) {
		collisions.push_back(terrain_collision->pos);
	}
	if (info.aerial && collisions.empty()) {
		let average_pos = info.segment.first
			+ (info.segment.first - info.segment.second) / 2.;
		collisions.push_back(info.segment.first);
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
	damage_result result;
	result.collisions = move(collisions);
	result.hit_terrain = terrain_collision.has_value();
	if (terrain_collision)
		result.reflected = terrain_collision->reflected_angle;
	return result;
}

// Serializer / Deserializer

mark::world::world(
	world_stack& stack,
	resource::manager& rm,
	mark::random& random,
	const YAML::Node& node)
	: m_resource_manager(rm)
	, m_map(std::make_unique<mark::map>(rm, random, node["map"]))
	, m_space_bins(
		  voxel_dim,
		  m_map->map_to_world({ 0, 0 }),
		  m_map->map_to_world(vi32{ m_map->size() }))
	, m_camera(std::make_unique<mark::camera>(node["camera"]))
	, image_bar(rm.image("bar.png"))
	, image_font(rm.image("font.png"))
	, image_stun(rm.image("stun.png"))
	, m_stack(&stack)
{
	std::unordered_map<uint64_t, weak_ptr<unit::base>> unit_map;
	let camera_target_id = node["camera"]["target_id"].as<uint64_t>();
	for (let& unit_node : node["units"]) {
		m_units.push_back(unit::deserialize(*this, random, unit_node));
		let unit_id = unit_node["id"].as<uint64_t>();
		unit_map.emplace(unit_id, m_units.back().get());
	}
	for (let& unit_node : node["units"]) {
		let unit_id = unit_node["id"].as<uint64_t>();
		unit_map[unit_id].lock()->resolve_ref(unit_node, unit_map);
	}
	m_camera->target(unit_map.at(camera_target_id).lock());
}

void mark::world::next() { m_stack->next(); }

void mark::world::prev() { m_stack->prev(); }

void mark::world::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;

	out << Key << "type" << Value << "world";

	out << Key << "camera" << Value;
	m_camera->serialize(out);

	out << Key << "units" << Value << BeginSeq;
	for (let& unit : m_units) {
		unit->serialize(out);
	}
	out << EndSeq;

	out << Key << "map" << Value;
	m_map->serialize(out);

	out << EndMap;
}

auto mark::world::blueprints() const
	-> const std::unordered_map<std::string, YAML::Node>&
{
	return m_stack->blueprints();
}
