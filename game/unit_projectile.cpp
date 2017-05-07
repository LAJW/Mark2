#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include <assert.h>
#include "unit_modular.h"

namespace {
	static auto validate(const mark::unit::projectile::info& args) {
		assert(args.world != nullptr);
		assert(!std::isnan(args.rotation));
		assert(!std::isnan(args.velocity));
		assert(args.seek_radius >= 0.f);
		assert(args.aoe_radius >= 0.f);
		return args;
	}
}

mark::unit::projectile::projectile(const mark::unit::projectile::info& args) :
	mark::unit::projectile::projectile(::validate(args), true) { }

mark::unit::projectile::projectile(const mark::unit::projectile::info& args, bool):
	mark::unit::base(*args.world, args.pos),
	m_image(args.world->resource_manager().image("shell.png")),
	m_im_tail(args.world->resource_manager().image("glare.png")),
	m_velocity(args.velocity),
	m_rotation(args.rotation),
	m_seek_radius(args.seek_radius),
	m_aoe_radius(args.aoe_radius),
	m_piercing(args.piercing),
	m_guide(args.guide) {
	this->team(static_cast<int>(args.team));
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto step = mark::rotate(mark::vector<double>(1, 0), m_rotation) * 1000.0 * dt;
	const auto guide = m_guide.lock();
	const auto turn_speed = 500.f;
	if (guide) {
		const auto lookat = guide->lookat();
		if (mark::length(lookat - m_pos) < m_velocity * dt * 2.0) {
			m_guide.reset();
		} else {
			const auto direction = lookat - m_pos;
			const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -m_rotation)));
			const auto rot_step = static_cast<float>(turn_direction  * turn_speed * dt);
			if (std::abs(mark::atan(mark::rotate(direction, -m_rotation))) < turn_speed * dt) {
				m_rotation = static_cast<float>(mark::atan(direction));
			} else {
				m_rotation += rot_step;
			}
		}
	} else if (m_seek_radius >= 0.f) {
		auto target = m_world.find_one(
			m_pos,
			m_seek_radius,
			[this](const mark::unit::base& unit) {
			return unit.team() != this->team() && !unit.invincible();
		});
		if (target) {
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
		m_pos - step * 1.5,
		m_pos + step * 0.5
	});
	auto damaged = false;
	if (!std::isnan(collision.second.x)) {
		if (collision.first) {
			mark::idamageable::attributes args;
			args.damaged = &m_damaged;
			args.pos = collision.second;
			args.team = this->team();
			args.physical = 10.f;
			if (collision.first->damage(args)) {
				// hit an enemy unit
				if (m_piercing == 1) {
					m_pos = collision.second;
					m_dead = true;
				}
			}
			damaged = true;
		} else {
			// hit a wall
			m_pos = collision.second;
			m_dead = true;
			damaged = true;
		}
	}
	if (m_dead) {
		if (m_aoe_radius >= 0.f) {
			auto damageables = m_world.collide(m_pos, m_aoe_radius);
			mark::idamageable::attributes args;
			args.damaged = &m_damaged;
			args.pos = m_pos;
			args.team = this->team();
			args.physical = 10.f;
			for (auto damageable : damageables) {
				damageable.get().damage(args);
			}
		}
	} else {
		// tail: grey dust
		{
			mark::tick_context::spray_info spray;
			spray.image = m_im_tail;
			spray.pos = pos();
			spray.velocity(100.f);
			spray.lifespan(0.3f);
			spray.diameter(8.f);
			spray.count = 4;
			spray.step = mark::length(step);
			spray.direction = m_rotation + 180.f;
			spray.cone = 30.f;
			spray.color = { 175, 175, 175, 75 };
			context.render(spray);
		}
		// tail: white overlay
		{
			mark::tick_context::spray_info spray;
			spray.image = m_im_tail;
			spray.pos = pos();
			spray.velocity(75.f);
			spray.lifespan(0.15f);
			spray.diameter(8.f);
			spray.count = 4;
			spray.step = mark::length(step);
			spray.direction = m_rotation + 180.f;
			spray.cone = 30.f;
			context.render(spray);
		}
		{
			mark::sprite::info args;
			args.image = m_im_tail;
			args.pos = m_pos - step;
			args.size = 32.f;
			args.color = sf::Color(255, 200, 150, 255);
			context.sprites[0].emplace_back(args);
		}
		{
			mark::sprite::info args;
			args.image = m_image;
			args.pos = m_pos;
			args.size = 10.f;
			args.rotation = m_rotation;
			context.sprites[1].emplace_back(args);
		}
	}
	if (damaged) {
		mark::tick_context::spray_info spray;
		spray.image = m_im_tail;
		spray.pos = pos();
		spray.velocity(5.f, 75.f);
		spray.lifespan(0.3f);
		spray.diameter(8.f);
		spray.count = 80;
		spray.cone = 360.f;
		spray.color = { 125, 125, 125, 75 };
		context.render(spray);
	}
}

auto mark::unit::projectile::dead() const -> bool {
	return m_dead;
}

auto mark::unit::projectile::invincible() const -> bool {
	return true;
}

auto mark::unit::projectile::collide(const mark::segment_t&) ->
	std::pair<mark::idamageable *, mark::vector<double>> {
	return { nullptr, { NAN, NAN } };
}
