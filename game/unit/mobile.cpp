#include "mobile.h"
#include <algorithm.h>
#include <map.h>
#include <random.h>
#include <resource_manager.h>
#include <stdafx.h>
#include <world.h>

// Calculate acceleration, given current velocity and distance to target
// Returns negative acceleration (can be greater than supplied acceleration)
// if distance is small enough, that supplied acceleration is not enough to
// stop a ship in that distance
static auto acceleration(
	const double length,
	const double velocity,
	const double acceleration) -> double
{
	let v = velocity;
	let a = acceleration;
	let T = v / a;
	let s = v * T - 0.5 * a * T * T;
	if (length <= s) {
		return -0.5 * v * v / s;
	}
	return acceleration;
}

mark::unit::mobile::mobile(mark::world& world, const YAML::Node& node)
	: damageable(world, node)
	, m_moveto(node["moveto"].as<vd>())
{}

mark::unit::mobile::mobile(const info& info)
	: damageable(info)
	, m_velocity(info.velocity)
	, m_moveto(info.moveto ? *info.moveto : info.pos)
{}

static auto calculate_velocity(
	double distance,
	double max_velocity,
	double cur_velocity,
	double max_acceleration,
	double dt)
{
	auto acceleration =
		::acceleration(distance, cur_velocity, max_acceleration);
	if (max_velocity <= cur_velocity) {
		acceleration = std::abs(acceleration) * -1.0;
	}
	return std::clamp(cur_velocity + acceleration * dt, 0., max_velocity);
}

auto mark::unit::mobile::can_calculate_path(bool can_pathfind) const -> bool
{
	let is_player_controlled = team() == 1;
	if (is_player_controlled) {
		return true;
	}
	if (!can_pathfind) {
		return false;
	}
	if (m_path_age <= 0.f) {
		return true;
	}
	let target_has_moved = !m_path_cache.empty()
		&& length(m_path_cache.front() - m_moveto) < this->radius();
	return target_has_moved;
}

auto mark::unit::mobile::calculate_path(bool can_pathfind, double dt) const
	-> std::pair<std::vector<vd>, float>
{
	auto [path, age] = [&]() -> std::pair<std::vector<vd>, float> {
		if (this->can_calculate_path(can_pathfind)) {
			let radius = this->radius();
			return { world().map().find_path(pos(), m_moveto, radius), 1.f };
		}
		return { m_path_cache, m_path_age - static_cast<float>(dt) };
	}();
	while (!path.empty() && length(path.back() - pos()) <= map::tile_size) {
		path.pop_back();
	}
	return { path, age };
}

auto mark::unit::mobile::avoid_present_neighbor_collisions(
	double step_len) const -> std::optional<vd>
{
	let radius = this->radius();
	let colliding_allies = world().find<mobile>(pos(), radius, [&](let& unit) {
		return unit.team() == this->team() && &unit != this;
	});
	if (colliding_allies.empty()) {
		return {};
	}
	let least_resistance_direction =
		accumulate(colliding_allies, vd(), [&](vd sum, let& ally) {
			let overlap = radius + ally->radius() - length(ally->pos() - pos());
			let direction = normalize(ally->pos() - pos());
			return sum + direction * overlap;
		});
	return -normalize(least_resistance_direction) * step_len;
}

auto mark::unit::mobile::avoid_future_neighbor_collisions(vd step) const -> vd
{
	let step_len = length(step);
	let radius = this->radius();
	let future_colliding_allies =
		world().find<mobile>(pos() + step, radius, [&](let& unit) {
			return unit.team() == this->team() && &unit != this;
		});
	if (!future_colliding_allies.empty()) {
		let& ally = future_colliding_allies.front();
		let diff = ally->pos() - pos();
		let d = length(diff);
		let dir = normalize(diff);
		let ortho = rotate(dir, 90.f);
		let R1n2 = radius + ally->radius();
		let d_comp = dir * (d - R1n2);
		let ortho_dir = ortho * (ortho.x * step.x + ortho.y * step.y);
		let ortho_comp = normalize(ortho_dir) * (step_len - (d - R1n2));
		step = normalize(d_comp + ortho_comp) * step_len;
	}
	let collides_after_step = any_of(future_colliding_allies, [&](let& ally) {
		return length(pos() + step - ally->pos())
			< radius + ally->radius() - 10.;
	});
	if (collides_after_step) {
		return {};
	}
	return step;
}

auto mark::unit::mobile::avoid_bumping_into_terrain(vd step) const
	-> std::optional<vd>
{
	let radius = this->radius();
	if (!world().map().traversable(pos(), radius)
		|| world().map().traversable(pos() + step, radius)) {
		return step;
	}
	if (world().map().traversable(pos() + vd(step.x, 0), radius)) {
		return vd(step.x, 0);
	}
	if (world().map().traversable(pos() + vd(0, step.y), radius)) {
		return vd(0, step.y);
	}
	return std::nullopt;
}

auto mark::unit::mobile::update_movement_impl(
	const update_movement_info& info) const -> update_movement_impl_result
{
	let dt = info.dt;
	let distance = length(m_moveto - pos());
	let next_step_reaches_target = distance <= m_velocity * dt;
	let velocity = next_step_reaches_target
		? 0.
		: calculate_velocity(
			  distance, info.max_velocity, m_velocity, info.acceleration, dt);
	let allied_collisions_enabled = info.ai;
	if (allied_collisions_enabled) {
		if (let step = avoid_present_neighbor_collisions(velocity * dt)) {
			if (let maybe_step = avoid_bumping_into_terrain(*step)) {
				return {
					pos() + *maybe_step, velocity, m_path_cache, m_path_age
				};
			}
			return { pos(), 0., m_path_cache, m_path_age };
		}
	}
	auto [step, path_cache, path_age] = [&] {
		if (next_step_reaches_target) {
			return std::make_tuple(m_moveto - pos(), m_path_cache, m_path_age);
		}
		// Path is sometimes not computed for non-critical units to improve
		// performance, as they rarely change direction
		let can_pathfind = info.ai || (*info.random)(0, 2) == 0;
		let[path_cache, path_age] = this->calculate_path(can_pathfind, dt);
		let dir = path_cache.empty() ? normalize(m_moveto - pos())
									 : normalize(path_cache.back() - pos());
		let step = dir * velocity * dt;
		return std::make_tuple(step, path_cache, path_age);
	}();
	if (allied_collisions_enabled) {
		step = this->avoid_future_neighbor_collisions(step);
	}
	if (let maybe_step = avoid_bumping_into_terrain(step)) {
		return { pos() + *maybe_step, velocity, path_cache, path_age };
	}
	return { pos(), 0., path_cache, path_age };
}

void mark::unit::mobile::update_movement(const update_movement_info& info)
{
	auto [pos, velocity, path, path_age] = this->update_movement_impl(info);
	m_path_cache = std::move(path);
	m_path_age = path_age;
	this->pos(pos);
	m_velocity = velocity;
}

void mark::unit::mobile::command(const command::any& any)
{
	if (let move = std::get_if<command::move>(&any)) {
		m_moveto = move->to;
		m_path_cache = world().map().find_path(pos(), m_moveto, radius());
		if (!m_path_cache.empty()
			&& length(m_path_cache.front() - m_moveto) > map::tile_size) {
			m_moveto = m_path_cache.front();
		}
	}
}

void mark::unit::mobile::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	damageable::serialize(out);
	out << Key << "velocity" << Value << m_velocity;
	out << Key << "moveto" << Value << BeginMap;
	out << Key << "x" << Value << m_moveto.x;
	out << Key << "y" << Value << m_moveto.y;
	out << EndMap;
}

void mark::unit::mobile::stop()
{
	// Resets all queues and all modules
	for (let i : mark::range(11)) {
		this->command(command::activate{});
		this->command(command::release{});
	}
	m_velocity = {};
	m_moveto = pos();
}
