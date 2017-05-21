#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include "unit_projectile.h"

mark::unit::minion::minion(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_model(world.resource_manager().image("mark1.png")),
	m_gun_cooldown(0.5f),
	m_model_shield(world.resource_manager(), 116.f),
	m_image_explosion(world.resource_manager().image("explosion.png")) { }

void mark::unit::minion::tick(mark::tick_context& context) {
	double dt = context.dt;
	context.lights.emplace_back(m_pos, sf::Color::White);
	m_gun_cooldown.tick(dt);
	m_model.tick(dt);
	m_model_shield.tick(context, m_pos);
	const auto velocity = 100.0;
	const auto angular_velocity = 90.f;

	auto neighbors = m_world.find(m_pos, 50.0);
	if (m_world.target()) {
		const auto target_pos = m_world.target()->pos();
		const auto distance = target_pos - m_pos;
		if (mark::length(distance) < 1000) {
			const auto length = mark::length(distance);
			auto direction2 = mark::vector<double>(0, 0);
			for (const auto& neighbor : neighbors) {
				auto dist = (m_pos - neighbor->pos());
				auto len = mark::length(dist);
				if (len) {
					dist = dist / len;
					direction2 += dist;
				}
			}
			const auto path = m_world.map().find_path(m_pos, target_pos);
			mark::vector<double> direction;
			if (path.size() > 3) {
				const auto first = mark::vector<double>(path[path.size() - 3]);
				direction = mark::normalize((first - m_pos)) + mark::normalize(direction2);
			} else if (mark::length(target_pos - m_pos) > 320.0) {
				direction = mark::normalize((target_pos - m_pos)) + mark::normalize(direction2);
			}
			m_pos += direction * velocity * dt;
			if (direction.x == 0 && direction.y == 0) {
				m_rotation = mark::turn(target_pos - m_pos, m_rotation, angular_velocity, dt);
			} else {
				m_rotation = mark::turn(direction, m_rotation, angular_velocity, dt);
			}
			if (m_gun_cooldown.trigger()) {
				mark::unit::projectile::info attr;
				attr.world = &m_world;
				attr.pos = m_pos;
				attr.rotation = m_rotation;
				attr.velocity = 500.f;
				attr.team = this->team();
				context.units.emplace_back(std::make_shared<mark::unit::projectile>(attr));
			}
		}
	}

	context.sprites[1].push_back(m_model.render(m_pos, 116.f, m_rotation, sf::Color::White));
	mark::tick_context::bar_info bar;
	bar.image = m_world.resource_manager().image("bar.png");
	bar.pos = m_pos + mark::vector<double>(0, -72);
	bar.type = mark::tick_context::bar_type::health;
	bar.percentage = static_cast<float>(m_health) / 100.f;
	context.render(bar);
	if (m_health < 0) {
		mark::tick_context::spray_info spray;
		spray.image = m_image_explosion;
		spray.pos = pos();
		spray.velocity(50.f, 350.f);
		spray.lifespan(0.5f);
		spray.diameter(32.f);
		spray.count = 80;
		context.render(spray);
		m_dead = true;
	}
}

auto mark::unit::minion::dead() const -> bool {
	return m_dead;
}

auto mark::unit::minion::damage(const mark::idamageable::info& attr) -> bool {
	if (m_health >= 0 && attr.team != this->team()
		&& attr.damaged->find(this) == attr.damaged->end()) {
		m_model_shield.trigger(attr.pos);
		this->m_health -= attr.physical;
		attr.damaged->insert(this);
		return true;
	}
	return false;
}

auto mark::unit::minion::invincible() const -> bool {
	return m_health < 0;
}

auto mark::unit::minion::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable *, mark::vector<double>> {

	const auto ship_radius = 58.f;
	const auto intersection = mark::intersect(ray, m_pos, ship_radius);
	if (!std::isnan(intersection.x)) {
		return { this, intersection };
	} else {
		return { nullptr, { NAN, NAN } };
	}
}

auto mark::unit::minion::collide(mark::vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<mark::idamageable>> {
	std::vector<std::reference_wrapper<mark::idamageable>> out;
	const auto ship_radius = 58.f;
	if (mark::length(m_pos - center) <= ship_radius + radius) {
		return { *this };
	} else {
		return { };
	}
}
