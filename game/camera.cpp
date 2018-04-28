#include "camera.h"
#include <declarations.h>
#include <stdafx.h>
#include <unit/base.h>

mark::camera::camera(const YAML::Node& node)
	: m_pos(node["pos"].as<vd>())
{}

void mark::camera::target(std::shared_ptr<unit::base> target)
{
	m_target = target;
}

auto mark::camera::target() -> std::shared_ptr<unit::base> { return m_target; }

auto mark::camera::target() const -> std::shared_ptr<const unit::base>
{
	return m_target;
}

void mark::camera::update(double dt)
{
	m_adsr.update(dt);
	m_x_lfo.update(dt);
	m_y_lfo.update(dt);
	constexpr let T = .5;
	if (m_target) {
		let target_pos = m_target->pos();
		if (target_pos != m_prev_target_pos) {
			m_prev_target_pos = target_pos;
			let dist = length(target_pos - m_pos);
			m_a = 2. * dist / T / T;
			m_velocity = m_a * T;
		}
	}
	let target_pos = m_prev_target_pos;
	let diff = target_pos - m_pos;
	let dist = length(diff);
	if (dist == 0.) {
		return;
	}
	let dir = diff / dist;
	m_velocity -= m_a * dt;
	if (m_velocity * dt < dist && m_velocity > 0) {
		m_pos += m_velocity * dir * dt;
	} else {
		m_pos = target_pos;
		m_velocity = 0.;
		m_a = 0;
	}
}

auto mark::camera::pos() const -> vd
{
	return m_pos
		+ vd(m_x_lfo.get(), m_y_lfo.get())
		* std::pow(m_adsr.get(), 3.f) * 10.;
}

void mark::camera::trigger() { m_adsr.trigger(); }

void mark::camera::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << "type"
		<< "camera";
	if (m_target) {
		out << Key << "target_id" << Value
			<< reinterpret_cast<size_t>(m_target.get());
	}

	out << Key << "pos" << Value << BeginMap;
	out << Key << "x" << Value << m_pos.x;
	out << Key << "y" << Value << m_pos.y;
	out << EndMap;

	out << EndMap;
}
