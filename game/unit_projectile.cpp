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
	auto enemy = m_world.find_one(m_pos, 300.f, [this](const mark::unit::base& unit) {
		return unit.team() != this->team() && !unit.invincible() && unit.collides(this->m_pos, 5.f);
	});
	if (enemy || !m_world.map().traversable(m_pos)) {
		if (enemy) {
			enemy->damage(10, m_pos - step);
		}
		m_dead = true;
		context.spray(m_im_tail, m_pos, std::make_pair(5.f, 75.f), 0.3f, 8.f, 80, 0.0, 0.f, 360.f, { 125, 125, 125, 75 });
	}
	context.spray(m_im_tail, m_pos, 100.f, 0.3f, 8.f, 4, mark::length(step), m_rotation + 180.f, 30.f, { 175, 175, 175, 75 });
	context.sprites[0].push_back(mark::sprite(m_im_tail, m_pos - step, 32.f, 0, 0, sf::Color(255, 200, 150, 255)));
	context.sprites[1].push_back(mark::sprite(m_image, m_pos, 10.f, m_rotation));
}

auto mark::unit::projectile::dead() const -> bool {
	return m_dead;
}

auto mark::unit::projectile::invincible() const -> bool {
	return true;
}

auto mark::unit::projectile::collides(mark::vector<double> pos, float radius) const -> bool {
	return false;
}
