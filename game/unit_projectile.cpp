#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include <assert.h>

namespace {
	static auto validate(const mark::unit::projectile::attributes& args) {
		assert(args.world != nullptr);
		assert(args.rotation != NAN);
		assert(args.velocity != NAN);
		assert(args.seek_radius >= 0.f);
		return args;
	}
}

mark::unit::projectile::projectile(const mark::unit::projectile::attributes& args) :
	mark::unit::projectile::projectile(::validate(args), true) { }

mark::unit::projectile::projectile(const mark::unit::projectile::attributes& args, bool):
	mark::unit::base(*args.world, args.pos),
	m_image(args.world->resource_manager().image("shell.png")),
	m_im_tail(args.world->resource_manager().image("glare.png")),
	m_velocity(args.velocity),
	m_rotation(args.rotation),
	m_seek_radius(args.seek_radius) {
	this->team(static_cast<int>(args.team));
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto step = mark::rotate(mark::vector<double>(1, 0), m_rotation) * 1000.0 * dt;
	if (m_seek_radius >= 0.f) {
		auto target = m_world.find_one(
			m_pos,
			m_seek_radius,
			[this](const mark::unit::base& unit) {
			return unit.team() != this->team() && !unit.invincible();
		});
		if (target) {
			const auto turn_speed = 500.f;
			const auto direction = target->pos() - m_pos;
			const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -m_rotation)));
			const auto rot_step = static_cast<float>(turn_direction  * turn_speed * dt);
			if (std::abs(mark::atan(mark::rotate(direction, -m_rotation))) < turn_speed * dt) {
				m_rotation = static_cast<float>(mark::atan(direction));
			} else {
				m_rotation += rot_step;
			}
		}
	}
	m_pos += step;
	const auto collision = m_world.collide({
		m_pos - step,
		m_pos + mark::normalize(step) * 2.0
	});
	if (collision.first != nullptr) {
		std::unordered_set<mark::idamageable*> damaged;
		mark::idamageable::attributes args;
		args.damaged = &damaged;
		args.pos = collision.second;
		args.team = this->team();
		args.physical = 10.f;
		if (collision.first->damage(args)) {
			m_pos = collision.second;
			m_dead = true;
		}
	}
	if (!m_dead) {
		const auto intersection = m_world.map().collide({
			m_pos - step,
			m_pos + mark::normalize(step) * 2.0
		});
		if (!std::isnan(intersection.x)) {
			m_pos = intersection;
			m_dead = true;
		}
	}
	if (m_dead) {
		context.spray(m_im_tail, m_pos, std::make_pair(5.f, 75.f), 0.3f, 8.f, 80, 0.0, 0.f, 360.f, { 125, 125, 125, 75 });
	} else {
		context.spray(m_im_tail, m_pos, std::make_pair(5.f, 75.f), 0.3f, 8.f, 80, 0.0, 0.f, 360.f, { 125, 125, 125, 75 });
		context.spray(m_im_tail, m_pos, 100.f, 0.3f, 8.f, 4, mark::length(step), m_rotation + 180.f, 30.f, { 175, 175, 175, 75 });
		context.spray(m_im_tail, m_pos, 75.f, 0.15f, 8.f, 4, mark::length(step), m_rotation + 180.f, 30.f);
		{
			mark::sprite::arguments args;
			args.image = m_im_tail;
			args.pos = m_pos - step;
			args.size = 32.f;
			args.color = sf::Color(255, 200, 150, 255);
			context.sprites[0].emplace_back(args);
		}
		{
			mark::sprite::arguments args;
			args.image = m_image;
			args.pos = m_pos;
			args.size = 10.f;
			args.rotation = m_rotation;
			context.sprites[1].emplace_back(args);
		}
	}
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

auto mark::unit::projectile::collide(const mark::segment_t&) ->
	std::pair<mark::idamageable *, mark::vector<double>> {
	return { nullptr, { NAN, NAN } };
}
