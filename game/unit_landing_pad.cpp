#include "unit_landing_pad.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "module_shield_generator.h"

mark::unit::landing_pad::landing_pad(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_image(world.resource_manager().image("landing-pad.png")) {
}

void mark::unit::landing_pad::tick(mark::tick_context& context) {
	context.sprites[0].push_back(mark::sprite(m_image, m_pos, 320.f, 0.f));
	const auto image_grid = m_world.resource_manager().image("grid-background.png");
	if (m_ship.lock()) {
		for (int x = -10; x < 10; x++) {
			for (int y = -10; y < 10; y++) {
				const auto pos = m_pos + mark::vector<double>(x, y) * 32.0;
				context.sprites[0].push_back(mark::sprite(image_grid, pos, 32.f));
			}
		}
	}
}

void mark::unit::landing_pad::dock(mark::unit::modular* ship) {
	if (ship) {
		auto ship_ptr = std::dynamic_pointer_cast<mark::unit::modular>(m_world.find_one(m_pos, 500.0, [ship](const mark::unit::base& unit) {
			return &unit == ship;
		}));
		m_ship = ship_ptr;
	} else {
		m_ship.reset();
	}
}
