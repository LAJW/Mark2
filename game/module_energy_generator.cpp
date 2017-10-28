#include "stdafx.h"
#include "module_energy_generator.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "world.h"

mark::module::energy_generator::energy_generator(resource::manager& manager) :
	m_image_base(manager.image("energy-generator.png")),
	m_image_bar(manager.image("bar.png")),
	module::base({ 2, 2 }, manager.image("energy-generator.png")) {

}

void mark::module::energy_generator::tick(tick_context& context) {
	this->module::base::tick(context);
	m_cur_energy = std::min(m_cur_energy + m_energy_regen * static_cast<float>(context.dt), m_max_energy);
	const auto pos = this->pos();
	sprite info;
	info.image = m_image_base;
	info.pos = pos;
	info.size = module::size * 2.f;
	info.rotation = parent_rotation();
	info.frame = static_cast<uint8_t>(std::round(m_cur_energy / m_max_energy * 4.f));
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);
}

auto mark::module::energy_generator::describe() const->std::string {
	return "Energy Generator";
}

auto mark::module::energy_generator::harvest_energy() -> float {
	m_cur_energy -= 1.0;
	return 1.0;
}

auto mark::module::energy_generator::energy_ratio() const -> float {
	return m_cur_energy / m_max_energy;
}

// Serialize / Deserialize

mark::module::energy_generator::energy_generator(
	resource::manager& rm,
	const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("energy-generator.png")),
	m_image_bar(rm.image("bar.png")),
	m_cur_energy(node["cur_energy"].as<float>()),
	m_max_energy(node["max_energy"].as<float>()),
	m_energy_regen(node["energy_regen"].as<float>()) { }

void mark::module::energy_generator::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << Key << "energy_regen" << Value << m_energy_regen;
	out << EndMap;
}

auto mark::module::energy_generator::passive() const noexcept -> bool
{ return true; }
