#include <algorithm>
#include "module_energy_generator.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "world.h"

mark::module::energy_generator::energy_generator(mark::resource::manager& manager) :
	m_image_base(manager.image("energy-generator.png")),
	m_image_bar(manager.image("bar.png")),
	mark::module::base({ 2, 2 }, manager.image("energy-generator.png")) {

}

void mark::module::energy_generator::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	m_cur_energy = std::min(m_cur_energy + m_energy_regen * static_cast<float>(context.dt), m_max_energy);
	const auto pos = this->pos();
	mark::sprite::info info;
	info.image = m_image_base;
	info.pos = pos;
	info.size = mark::module::size * 2.f;
	info.rotation = parent().rotation();
	info.frame = static_cast<uint8_t>(std::round(m_cur_energy / m_max_energy * 4.f));
	info.color = this->heat_color();
	context.sprites[0].emplace_back(info);
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

void mark::module::energy_generator::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << "module_energy_generator";
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << Key << "energy_regen" << Value << m_energy_regen;
	out << EndMap;
}
