#include <stdafx.h>
#include "mobile.h"
#include <algorithm.h>
#include <map.h>
#include <resource_manager.h>
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
	, m_moveto(node["moveto"].as<vector<double>>())
{
}

mark::unit::mobile::mobile(const info& info)
	: damageable(info)
	, m_velocity(info.velocity)
	, m_moveto(info.moveto ? *info.moveto : info.pos)
{
}

auto mark::unit::mobile::tick_movement_impl(
	const tick_movement_info& info,
	const bool random_can_pathfind) const
	-> std::tuple<vector<double>, double, std::vector<vector<double>>, float>
{
	let dt = info.dt;
	let radius = this->radius();
	let[step, velocity, path_cache, path_age] = [&] {
		let distance = length(m_moveto - pos());
		if (distance > m_velocity * dt) {
			let new_velocity = [&] {
				auto acceleration =
					::acceleration(distance, m_velocity, info.acceleration);
				if (info.max_velocity <= m_velocity) {
					acceleration = std::abs(acceleration) * -1.0;
				}
				return acceleration <= 0.
					? std::max(0., m_velocity + acceleration * dt)
					: std::min(
						  info.max_velocity, m_velocity + acceleration * dt);
			}();
			auto [path_cache, path_age] = [&] {
				if (team() != 1 &&
					(m_path_age <= 0.f ||
					 (!m_path_cache.empty() &&
					  length(m_path_cache.front() - m_moveto) < radius)) &&
					world().map().can_find() && random_can_pathfind) {
					return std::make_pair(
						world().map().find_path(pos(), m_moveto, radius), 1.f);
				}
				return std::make_pair(
					m_path_cache, m_path_age - static_cast<float>(dt));
			}();
			while (!path_cache.empty() &&
				   length(path_cache.back() - pos()) <= map::tile_size) {
				path_cache.pop_back();
			}
			let dir = path_cache.empty() ? normalize(m_moveto - pos())
										 : normalize(path_cache.back() - pos());
			let step = dir * new_velocity * dt;
			return std::make_tuple(step, new_velocity, path_cache, path_age);
		}
		else {
			let step = m_moveto - pos();
			return std::make_tuple(step, 0.0, m_path_cache, m_path_age);
		}
	}();
	let should_stop = [&, step = step] {
		if (info.ai) {
			let allies = world().find<mobile>(pos(), radius, [this](let& unit) {
				return unit.team() == this->team();
			});
			for (let& ally : allies) {
				if (length(pos() + step - ally->pos()) <
					length(pos() - ally->pos())) {
					return true;
				}
			}
		}
		return false;
	}();
	// If current position is not traversable, go to the nearest traversable,
	// as pointed by map.find_path, even if next position is not traversable
	let new_pos = [&, step = step] {
		if (should_stop)
			return pos();
		if (!world().map().traversable(pos(), radius) ||
			world().map().traversable(pos() + step, radius)) {
			return pos() + step;
		}
		else if (world().map().traversable(
					 pos() + vector<double>(step.x, 0), radius)) {
			return pos() + vector<double>(step.x, 0);
		}
		else if (world().map().traversable(
					 pos() + vector<double>(0, step.y), radius)) {
			return pos() + vector<double>(0, step.y);
		}
		return pos();
	}();
	return {new_pos, velocity, path_cache, path_age};
}

void mark::unit::mobile::tick_movement(const tick_movement_info& info)
{
	let random_can_pathfind = world().resource_manager().random(0, 2);
	auto [pos, velocity, path, path_age] =
		this->tick_movement_impl(info, random_can_pathfind);
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
		if (!m_path_cache.empty() &&
			length(m_path_cache.front() - m_moveto) > map::tile_size) {
			m_moveto = m_path_cache.front();
		}
	}
}

void mark::unit::mobile::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	damageable::serialise(out);
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
