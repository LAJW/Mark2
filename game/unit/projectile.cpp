#include "projectile.h"
#include "modular.h"
#include <assert.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>
#include <world.h>

namespace {
static auto validate(const mark::unit::projectile::info& args)
{
	assert(args.world != nullptr);
	assert(!std::isnan(args.rotation));
	assert(!std::isnan(args.velocity));
	assert(args.seek_radius >= 0.f);
	assert(args.aoe_radius >= 0.f);
	return args;
}
} // namespace

mark::unit::projectile::projectile(const unit::projectile::info& args)
	: unit::projectile::projectile(::validate(args), true)
{}

mark::unit::projectile::projectile(const unit::projectile::info& args, bool)
	: unit::base(args)
	, m_image(args.world->resource_manager().image("shell.png"))
	, m_im_tail(args.world->resource_manager().image("glare.png"))
	, m_im_explosion(args.world->resource_manager().image("explosion.png"))
	, m_velocity(args.velocity)
	, m_rotation(args.rotation)
	, m_rotation_lfo(args.lfo, args.phase)
	, m_seek_radius(args.seek_radius)
	, m_aoe_radius(args.aoe_radius)
	, m_critical_chance(args.critical_chance)
	, m_critical_multiplier(args.critical_multiplier)
	, m_piercing(args.piercing)
	, m_guide(args.guide)
	, m_physical(args.physical)
	, m_knockback(args.knockback)
{}

void mark::unit::projectile::update(update_context& context)
{
	double dt = context.dt;
	m_rotation_lfo.update(dt);
	let rotation = m_rotation + m_rotation_lfo.get() * 15.f;
	let step = rotate(vector<double>(1, 0), rotation)
		* static_cast<double>(m_velocity) * dt;
	let turn_speed = 500.f;
	if (m_guide) {
		if (length(*m_guide - pos()) < m_velocity * dt * 2.0) {
			m_guide.reset();
		} else {
			m_rotation = turn(*m_guide - pos(), m_rotation, turn_speed, dt);
		}
	} else if (m_seek_radius >= 0.f) {
		auto target = world().find_one<unit::damageable>(
			pos(), m_seek_radius, [this](const unit::base& unit) {
				return unit.team() != this->team();
			});
		if (target) {
			m_rotation =
				turn(target->pos() - pos(), m_rotation, turn_speed, dt);
		}
	}
	pos(pos() + step);
	world::damage_info info;
	info.context = &context;
	info.segment = { pos() - step * 1.2, pos() };
	info.aoe_radius = m_aoe_radius;
	info.piercing = m_piercing;
	info.damage.knocked = &m_knocked;
	info.damage.damaged = &m_damaged;
	info.damage.team = this->team();
	info.damage.physical = m_physical;
	info.damage.critical_chance = m_critical_chance;
	info.damage.critical_multiplier = m_critical_multiplier;
	info.damage.stun_chance = 0.1f;
	info.damage.stun_duration = 1.f;
	info.damage.knockback = m_knockback;

	let[collisions, terrain_hit, reflected_angle] = world().damage(info);
	if (collisions.size() >= m_piercing) {
		m_dead = true;
	} else {
		m_piercing -= collisions.size();
		if (terrain_hit) {
			m_rotation = reflected_angle;
			pos(pos() - collisions.back()
				+ rotate(vector<double>(5., 0.), reflected_angle));
		}
	}
	if (!collisions.empty() && terrain_hit) {
		pos(collisions.back());
	}
	let is_heavy_damage = m_physical > 100;
	if (is_heavy_damage && !collisions.empty()) {
		context.crit = true;
	}
	for (let collision : collisions) {
		update_context::spray_info spray;
		spray.image = is_heavy_damage ? m_im_explosion : m_im_tail;
		spray.pos = collision;
		spray.velocity(5.f, is_heavy_damage ? 500.f : 75.f);
		spray.lifespan(is_heavy_damage ? .7f : .3f);
		spray.diameter(is_heavy_damage ? 32.f : 8.f);
		spray.count = is_heavy_damage ? 80 : 40;
		spray.cone = 360.f;
		spray.color = { 125, 125, 125, 75 };
		context.render(spray);
	}
	// tail: grey dust
	{
		update_context::spray_info spray;
		spray.image = is_heavy_damage ? m_im_explosion : m_im_tail;
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
		update_context::spray_info spray;
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
		sprite args;
		args.image = m_im_tail;
		args.pos = pos() - step;
		args.size = 32.f;
		args.color = sf::Color(255, 200, 150, 255);
		context.sprites[0].emplace_back(args);
	}
	{
		sprite args;
		args.image = m_image;
		args.pos = pos();
		args.size = 10.f;
		args.rotation = m_rotation;
		context.sprites[1].emplace_back(args);
	}
	context.lights.emplace_back(pos(), sf::Color::White);
}

auto mark::unit::projectile::dead() const -> bool { return m_dead; }

// Serializer / Deserializer

mark::unit::projectile::projectile(mark::world& world, const YAML::Node& node)
	: unit::base(world, node)
	, m_image(world.resource_manager().image("shell.png"))
	, m_im_tail(world.resource_manager().image("glare.png"))
	, m_im_explosion(world.resource_manager().image("explosion.png"))
	, m_velocity(node["velocity"].as<float>())
	, m_rotation(node["rotation"].as<float>())
	, m_seek_radius(node["seek_radius"].as<float>())
	, m_aoe_radius(node["aoe_radius"].as<float>())
	, m_critical_chance(node["critical_chance"].as<float>())
	, m_critical_multiplier(node["critical_multiplier"].as<float>())
	, m_piercing(node["piercing"].as<unsigned>())
	, m_physical(node["physical"].as<float>(10.f))
{}

void mark::unit::projectile::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::projectile::type_name;
	base::serialize(out);
	out << Key << "rotation" << Value << m_rotation;
	out << Key << "velocity" << Value << m_velocity;
	out << Key << "seek_radius" << Value << m_seek_radius;
	out << Key << "aoe_radius" << Value << m_aoe_radius;
	out << Key << "critical_chance" << Value << m_critical_chance;
	out << Key << "critical_multiplier" << Value << m_critical_multiplier;
	out << Key << "piercing" << Value << m_piercing;
	out << Key << "physical" << Value << m_physical;

	out << Key << "damaged" << Value << BeginSeq;
	for (let& damaged : m_damaged) {
		out << reinterpret_cast<uint64_t>(damaged);
	}
	out << EndSeq;

	out << EndMap;
}

void mark::unit::projectile::resolve_ref(
	const YAML::Node&,
	const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>&)
{
	// TODO: Update guides
}
