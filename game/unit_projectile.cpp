#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"

mark::unit::projectile::projectile(mark::world& world, mark::vector<double> pos, float rotation):
	mark::unit::base(world, pos),
	m_rotation(rotation),
	m_image(world.resource_manager().image("shell.png")),
	m_im_tail(world.resource_manager().image("glare.png")) {
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto step = mark::rotate(mark::vector<double>(1, 0), m_rotation) * 1000.0 * dt;
	m_pos += step;
	auto nearby = m_world.find(m_pos, 50.f);
	auto enemy_it = std::find_if(nearby.begin(), nearby.end(), [this](std::shared_ptr<mark::unit::base>& unit) {
		return unit->team() != this->team();
	});
	if (!m_world.map().traversable(m_pos)
		|| enemy_it != nearby.end()) {
		if (enemy_it != nearby.end()) {
			(*enemy_it)->damage(10, m_pos - step);
		}
		for (int i = 0; i < 80; i++) {
			float direction = static_cast<float>(m_world.resource_manager().random_double(-180.0, 180.0));
			float speed = static_cast<float>(m_world.resource_manager().random_double(5.0, 75.0));
			context.particles.emplace_back(m_im_tail, m_pos, speed, direction, 0.30f, sf::Color(125, 125, 125, 75));
		}
		m_dead = true;
	}


	for (int i = 0; i < 4; i++) {
		const auto pos = m_pos - step * static_cast<double>(i) / 4.0;
		float direction = static_cast<float>(m_world.resource_manager().random_double(-15.0, 15.0)) + 180.f + m_rotation;
		context.particles.emplace_back(m_im_tail, pos, 100.f, direction, 0.30f, sf::Color(175, 175, 175, 75));
	}
	for (int i = 0; i < 4; i++) {
		const auto pos = m_pos - step * static_cast<double>(i) / 4.0;
		float direction = static_cast<float>(m_world.resource_manager().random_double(-15.0, 15.0)) + 180.f + m_rotation;
		context.particles.emplace_back(m_im_tail, pos, 50.f, direction, 0.15f);
	}
	context.sprites[0].push_back(mark::sprite(m_im_tail, m_pos - step, 32.f, 0, 0, sf::Color(255, 200, 150, 255)));
	context.sprites[1].push_back(mark::sprite(m_image, m_pos, 10.f, m_rotation));
}

auto mark::unit::projectile::dead() const -> bool {
	return m_dead;
}