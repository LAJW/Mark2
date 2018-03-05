#include <stdafx.h>
#include "battery.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>

mark::module::battery::battery(resource::manager& manager) :
	m_image_base(manager.image("battery.png")),
	module::base({ 2, 2 }, manager.image("battery.png")) {

}

void mark::module::battery::tick(tick_context& context) {
	this -> module::base::tick(context);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = this->pos();
		_.size = module::size * 2.f;
		_.rotation = parent_rotation();
		let frame = (1.f - m_cur_energy / m_max_energy) * 4.f;
		_.frame = static_cast<uint8_t>(std::round(frame));
		_.color = this->heat_color();
		return _;
	}());
	for (auto& module : this->neighbours()) {
		if (m_cur_energy < m_max_energy
			&& module.first.get().energy_ratio() > this->energy_ratio()) {
			m_cur_energy += module.first.get().harvest_energy(context.dt);
		}
	}
}

auto mark::module::battery::describe() const->std::string {
	return "Battery";
}

auto mark::module::battery::harvest_energy(double dt) -> float {
	let delta = 1.0 * dt;
	if (m_cur_energy > delta) {
		m_cur_energy -= delta;
		return delta;
	}
	return 0.f;
}

auto mark::module::battery::energy_ratio() const -> float {
	return m_cur_energy / m_max_energy;
}

// deserialise / serialise

mark::module::battery::battery(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("battery.png")),
	m_max_energy(node["max_energy"].as<float>()),
	m_cur_energy(node["cur_energy"].as<float>()) { }

void mark::module::battery::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << EndMap;
}

auto mark::module::battery::passive() const noexcept -> bool
{ return true; }
