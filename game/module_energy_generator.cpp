#include <algorithm>
#include "module_energy_generator.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::energy_generator::energy_generator(mark::resource::manager& manager) :
	m_image_base(manager.image("shield-generator.png")),
	m_image_bar(manager.image("bar.png")),
	mark::module::base({ 2, 2 }, manager.image("shield-generator.png")) {

}

void mark::module::energy_generator::tick(mark::tick_context& context) {
	m_cur_energy = std::min(m_cur_energy + m_energy_regen * static_cast<float>(context.dt), m_max_energy);
	const auto pos = this->pos();
	context.sprites[0].push_back(mark::sprite(m_image_base, pos, mark::module::size * 2.f, parent().rotation()));
	// render the bar
	const auto energy = 100;
	for (int i = 0; i < 10; i++) {
		const auto offset_x = 7.f * static_cast<float>(i - 5);
		context.sprites[50].push_back(mark::sprite(m_image_bar, pos + mark::vector<double>(offset_x, -mark::module::size * 2.0), 8.f, 0, 5));
	}
	context.render_bar(m_image_bar, pos + mark::vector<double>(0, -mark::module::size), mark::tick_context::bar_type::energy, m_cur_energy / m_max_energy);

}

auto mark::module::energy_generator::dead() const -> bool {
	return false;
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
