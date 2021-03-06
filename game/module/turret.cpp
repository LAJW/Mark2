﻿#include "turret.h"
#include <algorithm.h>
#include <algorithm/match.h>
#include <exception.h>
#include <particle.h>
#include <property_manager.h>
#include <resource/image.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <targeting_system.h>
#include <unit/modular.h>
#include <unit/projectile.h>
#include <update_context.h>
#include <utility>
#include <world.h>

static auto cooldown(float rate_of_fire) { return 1.f / rate_of_fire; }

void mark::module::turret::update(update_context& context)
{
	this->module::base::update(context);
	m_adsr.update(context.dt);
	if (m_targeting_system) {
		m_targeting_system->update(context);
	}

	if (let target = this->targeting_system().target()) {
		*m_shared_target = *target;
	}

	m_rotation = this->rotation(context.dt);
	if (this->can_shoot()) {
		for (let index : mark::range(m_projectile_count)) {
			context.units.push_back(
				this->make_projectile(context, parent().world(), index));
		}
		m_cur_cooldown = ::cooldown(m_rate_of_fire);
		m_adsr.trigger();
		m_cur_heat = std::min(m_cur_heat + m_heat_per_shot, 100.f);
	}
	m_cur_cooldown = this->cooldown(context.dt);
	this->render(context);
}

auto mark::module::turret::rotation(double dt) const noexcept -> float
{
	if (m_angular_velocity != 0.f) {
		Expects(m_shared_target);
		let direction = *m_shared_target - pos();
		return turn(direction, m_rotation, m_angular_velocity, dt);
	}
	if (std::abs(grid_pos().x) >= std::abs(grid_pos().y)) {
		return (grid_pos().x > 0 ? 0 : 180.f) + parent().rotation();
	}
	return (grid_pos().y > 0 ? 90 : -90.f) + parent().rotation();
}

auto mark::module::turret::cooldown(double dt) const noexcept -> float
{
	let dtf = gsl::narrow_cast<float>(dt);
	if (m_is_chargeable && m_cur_cooldown != 0.f) {
		return m_is_charging
			? std::max(m_cur_cooldown - dtf, 0.f)
			: std::min(m_cur_cooldown + dtf, ::cooldown(m_rate_of_fire));
	}
	return m_cur_cooldown - dtf;
}

auto mark::module::turret::can_shoot() const noexcept -> bool
{
	if (m_stunned || m_cur_cooldown > 0.f) {
		return false;
	}
	if (m_is_chargeable) {
		return !m_is_charging;
	}
	return this->targeting_system().can_shoot();
}

auto mark::module::turret::make_projectile(
	update_context& context,
	mark::world& world,
	size_t index) const -> shared_ptr<unit::projectile>
{
	let rotation = [&] {
		let heat_angle =
			m_cone * m_cone_curve(m_cur_heat / 100.f) * m_angular_velocity
					!= 0.f
				&& m_cone_curve == curve::flat
			? context.random(-1.f, 1.f)
			: 0.f;
		let projectile_count_f = static_cast<float>(m_projectile_count);
		let index_f = static_cast<float>(index);
		let cur_angle = m_projectile_count != 1
			? (index_f / (projectile_count_f - 1.f) - 0.5f) * m_cone
			: 0.f;
		return m_rotation + cur_angle + heat_angle;
	}();
	return std::make_shared<unit::projectile>([&] {
		unit::projectile::info _;
		static_cast<projectile_config&>(_) = *this;
		_.world = world;
		_.guide = m_guided ? m_shared_target : nullptr;
		_.pos = pos();
		_.rotation = rotation;
		if (m_unstable) {
			_.lfo = context.random(2.f, 4.f);
			_.phase = context.random(0.f, 1.f);
		}
		_.team = parent().team();
		return _;
	}());
}

void mark::module::turret::render(update_context& context) const
{
	let heat_color = this->heat_color();
	let pos = this->pos();
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos = pos - rotate(vd(m_adsr.get() - 32.0, 0.0), m_rotation);
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
	let cooldown = 1.f / m_rate_of_fire;
	if (m_is_chargeable && m_cur_cooldown < cooldown) {
		let charge = 1.f - m_cur_cooldown / cooldown;
		let fx_pos = pos + rotate(vd(64., 0), m_rotation);
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
			_.pos = fx_pos
				+ rotate(vd(64, 0), direction) * static_cast<double>(charge)
					* context.random(.5, 1.);
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
	if (m_physical >= 0) {
		os << "Physical damage: " << m_physical << std::endl;
	}
	if (m_antimatter >= 0) {
		os << "Antimatter damage: " << m_antimatter << std::endl;
	}
	if (m_energy >= 0) {
		os << "Energy damage: " << m_energy << std::endl;
	}
	if (m_heat) {
		os << "Heat damage: " << m_heat << std::endl;
	}
	os << "Missiles per shot: " << m_projectile_count << std::endl;
	os << "Cone of fire: " << m_cone << std::endl;
	os << "Critical chance: " << std::round(m_critical_chance * 1000) / 10
	   << std::endl;
	os << "Critical multiplier: "
	   << std::round(m_critical_multiplier * 1000) / 10 << std::endl;
	os << "Heat per shot: " << m_heat_per_shot << std::endl;
	if (m_cone_curve == curve::linear) {
		os << "Low accuracy when hot" << std::endl;
	} else if (m_cone_curve == curve::invert) {
		os << "High accuracy when hot" << std::endl;
	} else if (m_cone_curve == curve::sin) {
		os << "Average heat for best accuracy" << std::endl;
	}
	os << base::describe() << std::endl;
	return os.str();
}

auto mark::module::turret::targeting_system() noexcept
	-> mark::targeting_system&
{
	return m_targeting_system ? *m_targeting_system
							  : parent().targeting_system();
}

template <typename prop_man, typename T>
void mark::module::turret::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(rate_of_fire);
	MARK_BIND(cur_cooldown);
	MARK_BIND(rotation);
	MARK_BIND(angular_velocity);
	MARK_BIND(unstable);
	MARK_BIND(projectile_count);
	MARK_BIND(burst_delay);
	MARK_BIND(guided);
	MARK_BIND(cone);
	MARK_BIND(heat_per_shot);
	MARK_BIND(is_chargeable);
	MARK_BIND(is_charging);
}

auto mark::module::turret::targeting_system() const noexcept
	-> const mark::targeting_system&
{
	return m_targeting_system ? *m_targeting_system
							  : parent().targeting_system();
}

mark::module::turret::turret(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_image(rm.image("turret.png"))
	, m_im_orb(rm.image("orb.png"))
	, m_image_variant(random(0, 11))
	, m_adsr(0.1f, 8.f, 0.1f, 0.8f)
	, m_rate_of_fire_curve(curve::deserialize(
		  node["rate_of_fire_curve"].as<std::string>("flat")))
	, m_cone_curve(
		  curve::deserialize(node["cone_curve"].as<std::string>("flat")))
{
	property_manager property_manager(random);
	bind(property_manager, *this);
	mark::bind(property_manager, *this);
	if (property_manager.deserialize(node)) {
		throw std::runtime_error(
			"Could not deserialize" + std::string(type_name));
	}
	if (m_angular_velocity > 0.f) {
		m_targeting_system = std::make_unique<mark::targeting_system>(*this);
	}
}

mark::module::turret::~turret() = default;

void mark::module::turret::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	property_serializer serializer;
	bind(serializer, *this);
	mark::bind(serializer, *this);
	serializer.serialize(out);
	out << Key << "rate_of_fire_curve" << Value
		<< curve::serialize(m_rate_of_fire_curve);
	out << Key << "cone_curve" << Value << curve::serialize(m_cone_curve);
	base::serialize(out);
	out << EndMap;
}

auto mark::module::turret::passive() const noexcept -> bool { return false; }

void mark::module::turret::command(const command::any& any)
{
	match(any, [&](const command::activate&) {
		m_is_charging = true;
	}, [&](const command::release&) {
		m_is_charging = false;
	}, [](let&) {});
	targeting_system().command(any);
}
