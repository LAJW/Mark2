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
	m_gun_cooldown.tick(dt);
	m_model.tick(dt);
	m_model_shield.tick(context, m_pos);

	auto neighbors = m_world.find(m_pos, 50.0);
	const auto distance = m_world.camera() - m_pos;
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
	const auto path = m_world.map().find_path(m_pos, m_world.camera());
	if (path.size() > 3) {
		const auto first = mark::vector<double>(path[path.size() - 3]);
		const auto direction = mark::normalize((first - m_pos)) + mark::normalize(direction2);
		const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -mark::atan(m_direction))));
		m_direction = rotate(m_direction, static_cast<float>(turn_direction  * 180.f * dt));
		m_pos += direction * 100.0 * dt;
	} else if (mark::length(m_world.camera() - m_pos) > 320.0) {
		const auto direction = mark::normalize((m_world.camera() - m_pos)) + mark::normalize(direction2);
		const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -mark::atan(m_direction))));
		m_direction = rotate(m_direction, static_cast<float>(turn_direction  * 180.f * dt));
		m_pos += direction * 100.0 * dt;
	}
	if (m_gun_cooldown.trigger()) {
		auto projectile = std::make_shared<mark::unit::projectile>(m_world, m_pos, mark::atan(m_direction));
		projectile->team(this->team());
		context.units.emplace_back(std::move(projectile));
	}


	const auto rotation = mark::atan(m_direction);
	context.sprites[1].push_back(m_model.render(m_pos, 116.f, rotation, sf::Color::White));
	if (m_health < 0) {
		for (int i = 0; i < 80; i++) {
			float direction = static_cast<float>(m_world.resource_manager().random_double(-180.0, 180.0));
			float speed = static_cast<float>(m_world.resource_manager().random_double(50.0, 350.0));
			float size = static_cast<float>(m_world.resource_manager().random_double(32.0, 64.0));
			context.particles.emplace_back(m_image_explosion, m_pos, speed, direction, 0.5f, sf::Color::White, size);
		}
		m_dead = true;
	}
}

auto mark::unit::minion::dead() const -> bool {
	return m_dead;
}

void mark::unit::minion::damage(unsigned amount, mark::vector<double> pos) {
	m_model_shield.trigger(pos);
	this->m_health -= amount;
}

auto mark::unit::minion::invincible() const -> bool {
	return m_health < 0;
}

auto mark::unit::minion::collides(mark::vector<double> pos, float radius) const -> bool {
	return mark::length(pos - m_pos) < static_cast<double>(radius + 116.f);
}
