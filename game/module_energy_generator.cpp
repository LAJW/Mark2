#include <algorithm>
#include "module_energy_generator.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "world.h"

mark::module::energy_generator::energy_generator(mark::resource::manager& manager) :
	m_image_base(manager.image("shield-generator.png")),
	m_image_bar(manager.image("bar.png")),
	mark::module::base({ 2, 2 }, manager.image("shield-generator.png")) {

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
	context.sprites[0].emplace_back(info);

	mark::tick_context::bar_info energy_bar;
	energy_bar.image = m_image_bar;
	energy_bar.pos = pos + mark::vector<double>(0, -mark::module::size);
	energy_bar.type = mark::tick_context::bar_type::energy;
	energy_bar.percentage = m_cur_energy / m_max_energy;
	context.render(energy_bar);
}

void mark::module::energy_generator::shoot(mark::vector<double> pos) {
	/* noop */
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
