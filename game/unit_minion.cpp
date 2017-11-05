#include "stdafx.h"
#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "unit_projectile.h"

mark::unit::minion::minion(mark::world& world, vector<double> pos):
	unit::mobile(world, pos),
	m_model(world.resource_manager().image("mark1.png")),
	m_gun_cooldown(0.5f),
	m_model_shield(world.resource_manager(), static_cast<float>(this->radius()) * 2.f),
	m_image_explosion(world.resource_manager().image("explosion.png")) { }

void mark::unit::minion::tick(tick_context& context) {
	double dt = context.dt;
	context.lights.emplace_back(pos(), sf::Color::White);
	m_gun_cooldown.tick(dt);
	m_model.tick(dt);
	m_model_shield.tick(context, pos());
	const auto velocity = 100.0;
	const auto angular_velocity = 90.f;

	auto neighbours = world().find(pos(), this->radius());
	if (const auto target = world().target()) {
		const auto distance = target->pos() - pos();
		if (length(distance) < 1000) {
			const auto length = mark::length(distance);
			auto direction2 = vector<double>(0, 0);
			for (const auto& neighbour : neighbours) {
				auto dist = (pos() - neighbour->pos());
				auto len = mark::length(dist);
				if (len) {
					dist = dist / len;
					direction2 += dist;
				}
			}
			mobile::command(command::move{ target->pos() });
			mobile::tick_movement(dt, velocity, true);
			m_rotation = turn(
				normalize(distance), m_rotation, angular_velocity, dt);
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
	std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>>
{
	if (const auto intersection = intersect(ray, pos(), this->radius())) {
		return { { *this, *intersection} };
	}
	return { };
}

auto mark::unit::minion::collide(vector<double> center, float radius) ->
	std::vector<std::reference_wrapper<interface::damageable>>
{
	std::vector<std::reference_wrapper<interface::damageable>> out;
	if (length(pos() - center) <= this->radius() + radius) {
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

auto mark::unit::minion::radius() const -> double
{ return 58.f; }

