#include "turret.h"
#include <algorithm.h>
#include <exception.h>
#include <particle.h>
#include <property_manager.h>
#include <resource_image.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <tick_context.h>
#include <unit/modular.h>
#include <unit/projectile.h>
#include <utility>
#include <world.h>

void mark::module::turret::tick(tick_context& context)
{
	this->module::base::tick(context);
	let dt = context.dt;
	let fdt = static_cast<float>(dt);
	m_adsr.tick(dt);
	let pos = this->pos();
	m_targeting_system.tick();

	if (let target = m_targeting_system.target()) {
		*m_shared_target = *target;
	}

	m_rotation = [&] {
		if (m_angular_velocity != 0.f) {
			return turn(
				*m_shared_target - pos, m_rotation, m_angular_velocity, dt);
		}
		if (std::abs(grid_pos().x) > std::abs(grid_pos().y)) {
			return (grid_pos().x > 0 ? 0 : 180.f) + parent().rotation();
		}
		return (grid_pos().y > 0 ? 90 : -90.f) + parent().rotation();
	}();
	const auto cooldown = 1.f / m_rate_of_fire;
	m_cur_cooldown = [&] {
		if (m_is_chargeable && m_cur_cooldown != 0.f) {
			return m_is_charging ? std::max(m_cur_cooldown - fdt, 0.f)
								 : std::min(m_cur_cooldown + fdt, cooldown);
		}
		return m_cur_cooldown - fdt;
	}();
	if (m_cur_cooldown <= 0.f &&
		((!m_is_chargeable && !m_stunned &&
		  m_targeting_system.request_charge()) ||
		 (m_is_chargeable && !m_is_charging))) {
		m_cur_cooldown = cooldown;
		m_adsr.trigger();
		let projectile_count = static_cast<float>(m_projectile_count);
		let range = mark::range(projectile_count);
		std::transform(
			range.begin(),
			range.end(),
			std::back_inserter(context.units),
			[&](let i) {
				unit::projectile::info _;
				_.world = &parent().world();
				_.guide = m_guided ? m_shared_target : nullptr;
				_.pos = pos;
				let heat_angle = m_cone * m_cone_curve(m_cur_heat / 100.f) *
								m_angular_velocity !=
							0.f &&
						m_cone_curve == curve::flat
					? context.random(-1.f, 1.f)
					: 0.f;
				let cur_angle = projectile_count != 1.f
					? (i / (projectile_count - 1.f) - 0.5f) * m_cone
					: 0.f;
				_.rotation = m_rotation + cur_angle + heat_angle;
				_.velocity = m_velocity;
				_.physical = m_physical;
				_.aoe_radius = m_aoe_radius;
				_.seek_radius = m_seek_radius;
				_.team = parent().team();
				_.piercing = m_piercing;
				_.critical_chance = m_critical_chance;
				_.critical_multiplier = m_critical_multiplier;
				return std::make_shared<unit::projectile>(_);
			});
		m_cur_heat = std::min(m_cur_heat + m_heat_per_shot, 100.f);
	}
	let heat_color = this->heat_color();
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos =
			pos - rotate(vector<double>(m_adsr.get() - 32.0, 0.0), m_rotation);
		_.size = 32.f;
		_.rotation = m_rotation;
		_.color = heat_color;
		_.frame = 1 + m_image_variant * 2;
		return _;
	}());
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos = pos;
		_.size = 32.f;
		_.rotation = m_rotation;
		_.color = heat_color;
		_.frame = m_image_variant * 2;
		return _;
	}());
	if (m_is_chargeable && m_cur_cooldown < cooldown) {
		let charge = 1.f - m_cur_cooldown / cooldown;
		let fx_pos = pos + rotate(vector<double>(64., 0), m_rotation);
		context.sprites[3].emplace_back([&] {
			sprite _;
			_.image = m_im_orb;
			_.pos = fx_pos;
			_.size = 64.f * charge;
			_.frame = 0;
			return _;
		}());
		context.particles.push_back([&] {
			let direction = context.random(-180.f, 180.f);
			mark::particle::info _;
			_.pos = fx_pos +
				rotate(vector<double>(64, 0), direction) *
					static_cast<double>(charge) * context.random(.5, 1.);
			_.direction = direction;
			_.velocity = -charge * 64.f;
			_.image = m_im_orb;
			_.lifespan = .5f;
			_.size = 12.f * charge;
			_.layer = 3;
			return _;
		}());
	}
}

auto mark::module::turret::describe() const -> std::string
{
	std::ostringstream os;
	os << "Turret" << std::endl;
	os << "Health: " << std::round(m_cur_health) << " of "
	   << std::round(m_max_health) << std::endl;
	os << "Physical damage: " << m_physical << std::endl;
	os << "Energy damage: " << m_energy << std::endl;
	os << "Heat damage: " << m_energy << std::endl;
	os << "Missiles per shot: " << static_cast<unsigned>(m_projectile_count)
	   << std::endl;
	os << "Cone of fire: " << m_cone << std::endl;
	os << "Critical chance: " << std::round(m_critical_chance * 1000) / 10
	   << std::endl;
	os << "Critical multiplier: "
	   << std::round(m_critical_multiplier * 1000) / 10 << std::endl;
	os << "Heat per shot: " << m_heat_per_shot << std::endl;
	if (m_cone_curve == curve::linear) {
		os << "Low accuracy when hot" << std::endl;
	}
	else if (m_cone_curve == curve::invert) {
		os << "High accuracy when hot" << std::endl;
	}
	else if (m_cone_curve == curve::sin) {
		os << "Average heat for best accuracy" << std::endl;
	}
	return os.str();
}

template <typename prop_man, typename T>
void mark::module::turret::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(rate_of_fire);
	MARK_BIND(cur_cooldown);
	MARK_BIND(rotation);
	MARK_BIND(angular_velocity);
	MARK_BIND(projectile_count);
	MARK_BIND(burst_delay);
	MARK_BIND(guided);
	MARK_BIND(cone);
	MARK_BIND(heat_per_shot);
	MARK_BIND(critical_chance);
	MARK_BIND(critical_multiplier);
	MARK_BIND(physical);
	MARK_BIND(energy);
	MARK_BIND(heat);
	MARK_BIND(projectile_angular_velocity);
	MARK_BIND(velocity);
	MARK_BIND(aoe_radius);
	MARK_BIND(seek_radius);
	MARK_BIND(range);
	MARK_BIND(piercing);
	MARK_BIND(is_chargeable);
}

void mark::module::turret::bind(mark::property_manager& property_manager)
{
	bind(property_manager, *this);
	base::bind(property_manager);
}

mark::module::turret::turret(resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_targeting_system(*this)
	, m_image(rm.image("turret.png"))
	, m_im_orb(rm.image("orb.png"))
	, m_image_variant(rm.random(0, 11))
	, m_adsr(0.1f, 8.f, 0.1f, 0.8f)
	, m_cone_curve(
		  curve::deserialise(node["cone_curve"].as<std::string>("flat")))
	, m_rate_of_fire_curve(curve::deserialise(
		  node["rate_of_fire_curve"].as<std::string>("flat")))
{
	property_manager property_manager(rm);
	bind(property_manager);
	if (property_manager.deserialise(node)) {
		throw std::runtime_error(
			"Could not deserialise" + std::string(type_name));
	}
}

void mark::module::turret::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	property_serialiser serialiser;
	bind(serialiser, *this);
	serialiser.serialise(out);
	out << Key << "rate_of_fire_curve" << Value
		<< curve::serialise(m_rate_of_fire_curve);
	out << Key << "cone_curve" << Value << curve::serialise(m_cone_curve);
	base::serialise(out);
	out << EndMap;
}

auto mark::module::turret::passive() const noexcept -> bool { return false; }

void mark::module::turret::command(const command::any& any)
{
	if (std::holds_alternative<command::activate>(any)) {
		m_is_charging = true;
	}
	else if (std::holds_alternative<command::release>(any)) {
		m_is_charging = false;
	}
	m_targeting_system.command(any);
}
