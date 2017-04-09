#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"

mark::unit::projectile::projectile(mark::world& world, mark::vector<double> pos, float rotation)
	:mark::unit::base(world, pos) {
	m_image = world.resource_manager().image("shell.png");
}

auto mark::unit::projectile::render() const -> std::vector<mark::sprite> {
	const auto rotation = m_rotation + 90.f;
	return { mark::sprite(m_image, m_pos.x, m_pos.y, 50.f, rotation) };
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	m_pos += mark::rotate(mark::vector<double>(1, 0), m_rotation) * 100.0 * dt;
	if (m_world.map().traversable(m_pos)) {
		m_dead = true;
	}
}

auto mark::unit::projectile::dead() const -> bool {
	return m_dead;
}