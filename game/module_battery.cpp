#include "module_battery.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::battery::battery(mark::resource::manager& manager) :
	m_image_base(manager.image("shield-generator.png")),
	m_image_bar(manager.image("bar.png")),
	mark::module::base({ 2, 2 }, manager.image("shield-generator.png")) {

}

void mark::module::battery::tick(mark::tick_context& context) {
	const auto pos = this->pos();
	context.sprites[0].push_back(mark::sprite(m_image_base, pos, mark::module::size * 2.f, parent().rotation()));
	mark::tick_context::bar_info bar;
	bar.image = m_image_bar,
	bar.pos = pos + mark::vector<double>(0, -mark::module::size * 2.0);
	bar.type = mark::tick_context::bar_type::energy;
	bar.percentage = m_cur_energy / m_max_energy;
	context.render(bar);
	for (auto& module : this->neighbours()) {
		if (m_cur_energy < m_max_energy
			&& module.get().energy_ratio() > this->energy_ratio()) {
			m_cur_energy += module.get().harvest_energy();
		}
	}
}

void mark::module::battery::shoot(mark::vector<double> pos) {
	/* noop */
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
