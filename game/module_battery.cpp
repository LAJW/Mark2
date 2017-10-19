#include "stdafx.h"
#include "module_battery.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::battery::battery(resource::manager& manager) :
	m_image_base(manager.image("battery.png")),
	module::base({ 2, 2 }, manager.image("battery.png")) {

}

void mark::module::battery::tick(tick_context& context) {
	this -> module::base::tick(context);
	sprite info;
	info.image = m_image_base;
	info.pos = this->pos();
	info.size = module::size * 2.f;
	info.rotation = parent_rotation();
	info.frame = static_cast<uint8_t>(std::round((1.f - m_cur_energy / m_max_energy) * 4.f));
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);

	for (auto& module : this->neighbours()) {
		if (m_cur_energy < m_max_energy
			&& module.first.get().energy_ratio() > this->energy_ratio()) {
			m_cur_energy += module.first.get().harvest_energy();
		}
	}
}

auto mark::module::battery::describe() const->std::string {
	return "Battery";
}

auto mark::module::battery::harvest_energy() -> float {
	if (m_cur_energy > 1.f) {
		m_cur_energy -= 1.f;
		return 1.f;
	}
	return 0.f;
}

auto mark::module::battery::energy_ratio() const -> float {
	return m_cur_energy / m_max_energy;
}

// deserialize / serialize

mark::module::battery::battery(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("battery.png")),
	m_max_energy(node["max_energy"].as<float>()),
	m_cur_energy(node["cur_energy"].as<float>()) { }

void mark::module::battery::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	this->serialize_base(out);
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << EndMap;
}

auto mark::module::battery::passive() const noexcept -> bool
{ return true; }
