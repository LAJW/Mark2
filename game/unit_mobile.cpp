#include "stdafx.h"
#include "unit_mobile.h"
#include "world.h"
#include "resource_manager.h"

static auto start_breaking(
	const double length,
	const double velocity,
	const double acceleration) -> bool
{
	const auto v = velocity, a = acceleration;
	const auto s = std::pow(v, 2.0) / a - 0.5 * std::pow(v, 3.) / std::pow(a, 2.);
	return length <= s;
}


mark::unit::mobile::mobile(mark::world& world, const YAML::Node& node)
	: damageable(world, node)
	, m_moveto(node["moveto"].as<vector<double>>()) { }

void mark::unit::mobile::tick_movement(double dt, double max_velocity, bool ai)
{
	const auto radius = this->radius();
	vector<double> step;
	const auto distance = length(m_moveto - pos());
	if (distance > m_velocity * dt) {
		const auto acceleration = 500.0;
		if (start_breaking(distance, m_velocity, acceleration)
			|| m_velocity > max_velocity) {
			m_velocity = std::max(0., m_velocity - acceleration * dt);
		} else {
			m_velocity = std::min(max_velocity, m_velocity + acceleration * dt);
		}
		if (team() != 1
			&& (m_path_age <= 0.f
				|| !m_path_cache.empty()
					&& length(m_path_cache.front() - m_moveto) < radius)
			&& world().map().can_find()
			&& !world().resource_manager().random(0, 2)) {
			m_path_cache = world().map().find_path(pos(), m_moveto, radius);
			m_path_age = 1.f;
		} else {
			m_path_age -= static_cast<float>(dt);
		}

		while (!m_path_cache.empty() && length(m_path_cache.back() - pos()) <= map::tile_size) {
			m_path_cache.pop_back();
		}
		const auto dir = m_path_cache.empty()
			? normalize(m_moveto - pos())
			: normalize(m_path_cache.back() - pos());
		step = dir * m_velocity * dt;
	} else {
		step = m_moveto - pos();
		m_velocity = 0.0;
	}
	if (ai) {
		const auto allies = world().find<mobile>(
			pos(), radius, [this](const auto& unit) {
			return unit.team() == this->team();
		});
		for (const auto& ally : allies) {
			if (length(pos() + step - ally->pos())
				< length(pos() - ally->pos())) {
				step = { 0, 0 };
			}
		}
	}
	// TODO: intersect radius with terrain, stick to the wall, follow x/y axis
	if (world().map().traversable(pos() + step, radius)) {
		pos() += step;
	} else if (world().map().traversable(
			pos() + vector<double>(step.x, 0), radius)) {
		pos() += vector<double>(step.x, 0);
	} else if (world().map().traversable(
			pos() + vector<double>(0, step.y), radius)) {
		pos() += vector<double>(0, step.y);
	}
}

void mark::unit::mobile::command(const command::any& any)
{
	if (const auto move = std::get_if<command::move>(&any)) {
		m_moveto = move->to;
		m_path_cache = world().map().find_path(pos(), m_moveto, radius());
		if (!m_path_cache.empty()
			&& length(m_path_cache.front() - m_moveto) > map::tile_size) {
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
	m_velocity = { };
	m_moveto = pos();
}