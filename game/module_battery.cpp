#include "module_battery.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::battery::battery(mark::resource::manager& manager) :
	m_image_base(manager.image("battery.png")),
	mark::module::base({ 2, 2 }, manager.image("battery.png")) {

}

void mark::module::battery::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	const auto pos = this->pos();

	mark::sprite::info info;
	info.image = m_image_base;
	info.pos = pos;
	info.size = mark::module::size * 2.f;
	info.rotation = parent().rotation();
	info.frame = static_cast<uint8_t>(std::round((1.f - m_cur_energy / m_max_energy) * 4.f));
	info.color = this->heat_color();
	context.sprites[0].emplace_back(info);

	for (auto& module : this->neighbours()) {
		if (m_cur_energy < m_max_energy
			&& module.get().energy_ratio() > this->energy_ratio()) {
			m_cur_energy += module.get().harvest_energy();
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

void mark::module::battery::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << "module_battery";
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << EndMap;
}
