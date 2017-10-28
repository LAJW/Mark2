#include "stdafx.h"
#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_projectile.h"

mark::unit::minion::minion(mark::world& world, vector<double> pos):
	unit::damageable(world, pos),
	m_model(world.resource_manager().image("mark1.png")),
	m_gun_cooldown(0.5f),
	m_model_shield(world.resource_manager(), 116.f),
	m_image_explosion(world.resource_manager().image("explosion.png")) { }

void mark::unit::minion::tick(tick_context& context) {
	double dt = context.dt;
	context.lights.emplace_back(pos(), sf::Color::White);
	m_gun_cooldown.tick(dt);
	m_model.tick(dt);
	m_model_shield.tick(context, pos());
	const auto velocity = 100.0;
	const auto angular_velocity = 90.f;

	auto neighbors = world().find(pos(), 50.0);
	if (world().target()) {
		const auto target_pos = world().target()->pos();
		const auto distance = target_pos - pos();
		if (length(distance) < 1000) {
			const auto length = mark::length(distance);
			auto direction2 = vector<double>(0, 0);
			for (const auto& neighbor : neighbors) {
				auto dist = (pos() - neighbor->pos());
				auto len = mark::length(dist);
				if (len) {
					dist = dist / len;
					direction2 += dist;
				}
			}
			if (m_path_age <= 0.f || m_path_cache.size() > 0 && mark::length(m_path_cache.back() - target_pos) < 150.f) {
				m_path_cache = world().map().find_path(pos(), target_pos);
				m_path_age = 1.f;
			} else {
				m_path_age -= static_cast<float>(context.dt);
			}
			vector<double> direction;
			if (m_path_cache.size() > 3) {
				const auto first = vector<double>(m_path_cache[m_path_cache.size() - 3]);
				direction = normalize((first - pos())) + normalize(direction2);
			} else if (mark::length(target_pos - pos()) > 320.0) {
				direction = normalize((target_pos - pos())) + normalize(direction2);
			}
			pos() += direction * velocity * dt;
			if (direction.x == 0 && direction.y == 0) {
				m_rotation = turn(target_pos - pos(), m_rotation, angular_velocity, dt);
			} else {
				m_rotation = turn(direction, m_rotation, angular_velocity, dt);
			}
			if (m_gun_cooldown.trigger()) {
				unit::projectile::info attr;
				attr.world = &world();
				attr.pos = pos();
				attr.rotation = m_rotation;
				attr.velocity = 500.f;
				attr.team = this->team();
				context.units.emplace_back(std::make_shared<unit::projectile>(attr));
			}
		}
	}

	context.sprites[1].push_back(m_model.render(pos(), 116.f, m_rotation, sf::Color::White));
	tick_context::bar_info bar;
	bar.image = world().image_bar;
	bar.pos = pos() + vector<double>(0, -72);
	bar.type = tick_context::bar_type::health;
	bar.percentage = static_cast<float>(m_health) / 100.f;
	context.render(bar);
}


auto mark::unit::minion::dead() const -> bool {
	return m_dead;
}

auto mark::unit::minion::damage(const interface::damageable::info& attr) -> bool {
	if (m_health >= 0 && attr.team != this->team()
		&& attr.damaged->find(this) == attr.damaged->end()) {
		m_model_shield.trigger(attr.pos);
		this->m_health -= attr.physical;
		attr.damaged->insert(this);
		return true;
	}
	return false;
}

auto mark::unit::minion::collide(const segment_t& ray) ->
	std::optional<std::pair<interface::damageable*, vector<double>>>
{
	const auto ship_radius = 58.f;
	if (const auto intersection = intersect(ray, pos(), ship_radius)) {
		return { { this, *intersection} };
	}
	return { };
}

auto mark::unit::minion::collide(vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<interface::damageable>> {
	std::vector<std::reference_wrapper<interface::damageable>> out;
	const auto ship_radius = 58.f;
	if (length(pos() - center) <= ship_radius + radius) {
		return { *this };
	}
	return { };
}

void mark::unit::minion::on_death(tick_context & context) {
	if (m_health < 0) {
		tick_context::spray_info spray;
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

