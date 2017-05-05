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
			mark::unit::projectile::attributes attr;
			attr.world = &m_world;
			attr.pos = m_pos;
			attr.rotation = mark::atan(m_direction);
			attr.velocity = 500.f;
			attr.team = this->team();
			context.units.emplace_back(std::make_shared<mark::unit::projectile>(attr));
		}
	}

	const auto rotation = mark::atan(m_direction);
	context.sprites[1].push_back(m_model.render(m_pos, 116.f, rotation, sf::Color::White));
	context.render_bar(m_world.resource_manager().image("bar.png"), m_pos + mark::vector<double>(0, -72), mark::tick_context::bar_type::health, static_cast<float>(m_health) / 100.f);
	if (m_health < 0) {
		context.spray(m_image_explosion, pos(), std::make_pair(50.f, 350.f), 0.5f, 32.f, 80);
		m_dead = true;
	}
}

auto mark::unit::minion::dead() const -> bool {
	return m_dead;
}

auto mark::unit::minion::damage(const mark::idamageable::attributes& attr) -> bool {
	if (m_health >= 0 && attr.team != this->team()) {
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
