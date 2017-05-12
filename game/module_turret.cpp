#pragma once
#include <sstream>
#include "module_turret.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_projectile.h"
#include "unit_modular.h"
#include "world.h"

mark::module::turret::turret(mark::module::turret::info& info):
	base(mark::vector<unsigned>(info.size),
		info.resource_manager->image("turret-base.png")),
	m_im_base(info.resource_manager->image("turret-base.png")),
	m_im_cannon(info.resource_manager->image("turret-cannon.png")),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f),
	m_rate_of_fire(info.rate_of_fire),
	m_rotation(info.rotation),
	m_angular_velocity(info.angular_velocity),
	m_projectile_count(info.projectile_count),
	m_burst_delay(info.burst_delay),
	m_guided(info.guided),
	m_cone(info.cone),
	m_cone_curve(info.cone_curve),
	m_heat_per_shot(info.heat_per_shot),
	m_critical_chance(info.critical_chance),
	m_critical_multiplier(info.critical_multiplier),
	m_max_health(info.max_health),
	m_cur_health(info.cur_health), 
	m_cur_heat(info.cur_heat),
	m_physical(info.physical),
	m_energy(info.energy),
	m_heat(info.heat),
	m_projectile_angular_velocity(info.projectile_angular_velocity),
	m_velocity(info.velocity),
	m_acceleration(info.acceleration),
	m_aoe_radius(info.aoe_radius),
	m_seek_radius(info.seek_radius),
	m_range(info.range),
	m_piercing(info.piercing) { }

void mark::module::turret::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	m_adsr.tick(context.dt);
	auto pos = this->pos();
	if (m_angular_velocity == 0.f) {
		if (std::abs(grid_pos().x) > std::abs(grid_pos().y)) {
			m_rotation = (grid_pos().x > 0 ? 0 : 180.f) + parent().rotation();
		} else {
			m_rotation = (grid_pos().y > 0 ? 90 : -90.f) + parent().rotation();
		}
	} else {
		// TODO Respect angular velocity here
		m_rotation = mark::turn(m_target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	m_cur_heat = std::max(0.0, m_cur_heat - 10.f * context.dt);
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else if (m_shoot) {
		m_cur_cooldown = 1.f / m_rate_of_fire;
		m_adsr.trigger();
		mark::unit::projectile::info info;
		info.world = &parent().world();
		if (m_guided) {
			info.guide = std::dynamic_pointer_cast<mark::unit::modular>(parent().shared_from_this());
		}
		for (int i = 0; i < m_projectile_count; i++) {
			info.pos = pos;
			const auto heat_angle = m_cone * m_cone_curve(m_cur_heat / 100.f) * context.random(-1.f, 1.f);
			float cur_angle = 0.f;
			if (m_projectile_count != 1) {
				cur_angle = (static_cast<float>(i) / static_cast<float>(m_projectile_count - 1) - 0.5) * m_cone;
			}
			info.rotation = m_rotation + cur_angle + heat_angle;
			info.velocity = m_velocity;
			info.aoe_radius = m_aoe_radius;
			info.seek_radius = m_seek_radius;
			info.team = parent().team();
			info.piercing = m_piercing;
			info.critical_chance = m_critical_chance;
			info.critical_multiplier = m_critical_multiplier;
			context.units.emplace_back(std::make_shared<mark::unit::projectile>(info));
		}
		m_cur_heat = std::min(m_cur_heat + m_heat_per_shot, 100.f);
	}
	const auto heat_color = static_cast<uint8_t>(std::round((1.f - m_cur_heat / 100.f) * 255.f));
	{
		mark::sprite::info info;
		info.image = m_im_cannon;
		info.pos = pos - mark::rotate(mark::vector<double>(m_adsr.get() - 16.0, 0.0), m_rotation);
		info.size = 32.f;
		info.rotation = m_rotation;
		info.color = { 255, heat_color, heat_color, 255 };
		context.sprites[1].emplace_back(info);
	}
	{
		mark::sprite::info info;
		info.image = m_im_base;
		info.pos = pos;
		info.size = 32.f;
		info.rotation = parent().rotation();
		info.color = { 255, heat_color, heat_color, 255 };
		context.sprites[0].emplace_back(info);
	}
}

void mark::module::turret::target(mark::vector<double> pos) {
	m_target = pos;
}

void mark::module::turret::shoot(mark::vector<double> pos, bool release) {
	m_target = pos;
	m_shoot = !release;
}

auto mark::module::turret::describe() const -> std::string {
	std::ostringstream os;
	os << "Turret" << std::endl;
	os << "Health: " << std::round(m_cur_health) << " of " << std::round(m_max_health) << std::endl;
	os << "Physical damage: " << m_physical << std::endl;
	os << "Energy damage: " << m_energy << std::endl;
	os << "Heat damage: " << m_energy << std::endl;
	os << "Missiles per shot: " << static_cast<unsigned>(m_projectile_count) << std::endl;
	os << "Cone of fire: " << m_cone << std::endl;
	os << "Critical chance: " << std::round(m_critical_chance * 1000) / 10 << std::endl;
	os << "Critical multiplier: " << std::round(m_critical_multiplier * 1000) / 10 << std::endl;
	os << "Heat per shot: " << m_heat_per_shot << std::endl;
	if (m_cone_curve == mark::curve::linear) {
		os << "Low accuracy when hot" << std::endl;
	} else if (m_cone_curve == mark::curve::invert) {
		os << "High accuracy when hot" << std::endl;
	} else if (m_cone_curve == mark::curve::sin) {
		os << "Average heat for best accuracy" << std::endl;
	}
	return os.str();
}
