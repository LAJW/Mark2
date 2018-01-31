#include "stdafx.h"
#include <sstream>
#include <optional>
#include <utility>
#include "algorithm.h"
#include "module_turret.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_projectile.h"
#include "unit_modular.h"
#include "world.h"
#include "exception.h"

mark::module::turret::turret(module::turret::info& info):
	base_turret(vector<unsigned>(info.size),
		info.resource_manager->image("turret-base.png")),
	m_image(info.resource_manager->image("turret.png")),
	m_image_variant(info.resource_manager->random(0, 12)),
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
	m_physical(info.physical),
	m_energy(info.energy),
	m_heat(info.heat),
	m_projectile_angular_velocity(info.projectile_angular_velocity),
	m_velocity(info.velocity),
	m_acceleration(info.acceleration),
	m_aoe_radius(info.aoe_radius),
	m_seek_radius(info.seek_radius),
	m_range(info.range),
	m_piercing(info.piercing)
{
	m_cur_health = info.cur_health;
	m_max_health = info.max_health;
}


void mark::module::turret::tick(tick_context& context) {
	this->module::base::tick(context);
	m_adsr.tick(context.dt);
	auto pos = this->pos();

	base_turret::tick();
	if (let target = this->target()) {
		*m_shared_target = *target;
	}

	if (m_angular_velocity == 0.f) {
		if (std::abs(grid_pos().x) > std::abs(grid_pos().y)) {
			m_rotation = (grid_pos().x > 0 ? 0 : 180.f) + parent().rotation();
		} else {
			m_rotation = (grid_pos().y > 0 ? 90 : -90.f) + parent().rotation();
		}
	} else {
		m_rotation = turn(
			*m_shared_target - pos, m_rotation, m_angular_velocity, context.dt);
	}
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else if (this->can_shoot()) {
		m_cur_cooldown = 1.f / m_rate_of_fire;
		m_adsr.trigger();
		unit::projectile::info info;
		info.world = &parent().world();
		if (m_guided) {
			info.guide = m_shared_target;
		}
		for (let i : range(m_projectile_count)) {
			info.pos = pos;
			let heat_angle = m_cone
				* m_cone_curve(m_cur_heat / 100.f)
				* (m_angular_velocity != 0.f && m_cone_curve==curve::flat)
					? context.random(-1.f, 1.f)
					: 0.f;
			float cur_angle = 0.f;
			if (m_projectile_count != 1) {
				cur_angle = (static_cast<float>(i) / static_cast<float>(m_projectile_count - 1) - 0.5f) * m_cone;
			}
			info.rotation = m_rotation + cur_angle + heat_angle;
			info.velocity = m_velocity;
			info.aoe_radius = m_aoe_radius;
			info.seek_radius = m_seek_radius;
			info.team = parent().team();
			info.piercing = m_piercing;
			info.critical_chance = m_critical_chance;
			info.critical_multiplier = m_critical_multiplier;
			context.units.emplace_back(std::make_shared<unit::projectile>(info));
		}
		m_cur_heat = std::min(m_cur_heat + m_heat_per_shot, 100.f);
	}
	let heat_color = this->heat_color();
	{
		sprite info;
		info.image = m_image;
		info.pos = pos - rotate(vector<double>(m_adsr.get() - 32.0, 0.0), m_rotation);
		info.size = 32.f;
		info.rotation = m_rotation;
		info.color = heat_color;
		info.frame = 1 + m_image_variant * 2;
		context.sprites[2].emplace_back(info);
	}
	{
		sprite info;
		info.image = m_image;
		info.pos = pos;
		info.size = 32.f;
		info.rotation = m_rotation;
		info.color = heat_color;
		info.frame = m_image_variant * 2;
		context.sprites[2].emplace_back(info);
	}
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
	if (m_cone_curve == curve::linear) {
		os << "Low accuracy when hot" << std::endl;
	} else if (m_cone_curve == curve::invert) {
		os << "High accuracy when hot" << std::endl;
	} else if (m_cone_curve == curve::sin) {
		os << "Average heat for best accuracy" << std::endl;
	}
	return os.str();
}

mark::module::turret::turret(resource::manager& rm, const YAML::Node& node):
	module::base_turret(rm, node),
	m_image(rm.image("turret.png")),
	m_image_variant(rm.random(0, 12)),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f),
	m_cur_cooldown(node["cur_cooldown"].as<float>()),
	m_rate_of_fire(node["rate_of_fire"].as<float>()),
	m_rotation(node["rotation"].as<float>()),
	m_angular_velocity(node["angular_velocity"].as<float>()),
	m_projectile_count(node["projectile_count"].as<unsigned>()),
	m_burst_delay(node["burst_delay"].as<float>()),
	m_guided(node["guided"].as<bool>()),
	m_cone(node["cone"].as<float>()),
	m_cone_curve(curve::deserialise(node["cone_curve"].as<std::string>())),
	m_heat_per_shot(node["heat_per_shot"].as<float>()),
	m_critical_chance(node["critical_chance"].as<float>()),
	m_critical_multiplier(node["critical_multiplier"].as<float>()),
	m_physical(node["physical"].as<float>()),
	m_energy(node["energy"].as<float>()),
	m_heat(node["heat"].as<float>()),
	m_projectile_angular_velocity(node["projectile_angular_velocity"].as<float>()),
	m_velocity(node["velocity"].as<float>()),
	m_aoe_radius(node["aoe_radius"].as<float>()),
	m_seek_radius(node["seek_radius"].as<float>()),
	m_range(node["range"].as<float>()),
	m_piercing(node["piercing"].as<size_t>()) { }

void mark::module::turret::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base_turret::serialise(out);

	out << Key << "cur_cooldown" << Value << m_cur_cooldown;
	out << Key << "rate_of_fire" << Value << m_rate_of_fire;
	out << Key << "rate_of_fire_curve" << Value << curve::serialise(m_rate_of_fire_curve);
	out << Key << "rotation" << Value << m_rotation;
	out << Key << "angular_velocity" << Value << m_angular_velocity;
	out << Key << "projectile_count" << Value << static_cast<unsigned>(m_projectile_count);
	out << Key << "burst_delay" << Value << m_burst_delay;
	out << Key << "guided" << Value << m_guided;
	out << Key << "cone" << Value << m_cone;
	out << Key << "cone_curve" << Value << curve::serialise(m_cone_curve);
	out << Key << "heat_per_shot" << Value << m_heat_per_shot;
	out << Key << "critical_chance" << Value << m_critical_chance;
	out << Key << "critical_multiplier" << Value << m_critical_multiplier;

	out << Key << "physical" << Value << m_physical;
	out << Key << "energy" << Value << m_energy;
	out << Key << "heat" << Value << m_heat;
	out << Key << "projectile_angular_velocity" << Value << m_projectile_angular_velocity;
	out << Key << "velocity" << Value << m_velocity;
	out << Key << "acceleration" << Value << m_acceleration;
	out << Key << "aoe_radius" << Value << m_aoe_radius;
	out << Key << "seek_radius" << Value << m_seek_radius;
	out << Key << "range" << Value << m_range;
	out << Key << "piercing" << Value << m_piercing;

	out << EndMap;
}

auto mark::module::turret::passive() const noexcept -> bool
{ return false; }
