#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include <assert.h>

namespace {
	static auto validate(const mark::unit::projectile::attributes& essence) {
		assert(essence.world != nullptr);
		assert(essence.rotation != NAN);
		assert(essence.velocity != NAN);
		assert(essence.seek_radius >= 0.f);
		return essence;
	}
}

mark::unit::projectile::projectile(const mark::unit::projectile::attributes& essence) :
	mark::unit::projectile::projectile(::validate(essence), true) { }

mark::unit::projectile::projectile(const mark::unit::projectile::attributes& essence, bool):
	mark::unit::base(*essence.world, essence.pos),
	m_image(essence.world->resource_manager().image("shell.png")),
	m_im_tail(essence.world->resource_manager().image("glare.png")),
	m_velocity(essence.velocity),
	m_rotation(essence.rotation),
	m_seek_radius(essence.seek_radius) {
	this->team(essence.team);
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto step = mark::rotate(mark::vector<double>(1, 0), m_rotation) * 1000.0 * dt;
	if (m_seek_radius >= 0.f) {
		auto target = m_world.find_one(m_pos, m_seek_radius, [this](const mark::unit::base& unit) {
			return unit.team() != this->team() && !unit.invincible();
		});
		if (target) {
			const auto direction = mark::normalize((target->pos() - m_pos));
			const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -m_rotation)));
			const auto rot_step = static_cast<float>(turn_direction  * 500.f * dt);
			if (std::abs(mark::atan(mark::rotate(direction, -m_rotation))) < 32.f * dt) {
				m_rotation = static_cast<float>(mark::atan(direction));
			} else {
				m_rotation += rot_step;
			}
		}
	}

	m_pos += step;
	
	std::unordered_set<mark::idamageable*> damaged;
	mark::idamageable::attributes attr;
	attr.damaged = &damaged;
	attr.pos = m_pos;
	attr.team = this->team();
	attr.physical = 10.f;
	if (m_world.damage(attr)) {
		m_dead = true;
	} else {
		const auto intersection = m_world.map().collide({ m_pos - step, m_pos });
		if (!std::isnan(intersection.x)) {
			m_pos = intersection;
			m_dead = true;
		}
	}
	if (m_dead) {
		context.spray(m_im_tail, m_pos, std::make_pair(5.f, 75.f), 0.3f, 8.f, 80, 0.0, 0.f, 360.f, { 125, 125, 125, 75 });
	}
	context.spray(m_im_tail, m_pos, std::make_pair(5.f, 75.f), 0.3f, 8.f, 80, 0.0, 0.f, 360.f, { 125, 125, 125, 75 });
	context.spray(m_im_tail, m_pos, 100.f, 0.3f, 8.f, 4, mark::length(step), m_rotation + 180.f, 30.f, { 175, 175, 175, 75 });
	context.spray(m_im_tail, m_pos, 75.f, 0.15f, 8.f, 4, mark::length(step), m_rotation + 180.f, 30.f);
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