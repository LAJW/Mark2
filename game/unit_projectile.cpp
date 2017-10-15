#include "stdafx.h"
#include "unit_projectile.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "tick_context.h"
#include <assert.h>
#include "unit_modular.h"

namespace {
	static auto validate(const mark::unit::projectile::info& args) {
		assert(args.world != nullptr);
		assert(!std::isnan(args.rotation));
		assert(!std::isnan(args.velocity));
		assert(args.seek_radius >= 0.f);
		assert(args.aoe_radius >= 0.f);
		return args;
	}
}

mark::unit::projectile::projectile(const mark::unit::projectile::info& args) :
	mark::unit::projectile::projectile(::validate(args), true) { }

mark::unit::projectile::projectile(const mark::unit::projectile::info& args, bool):
	mark::unit::base(*args.world, args.pos),
	m_image(args.world->resource_manager().image("shell.png")),
	m_im_tail(args.world->resource_manager().image("glare.png")),
	m_velocity(args.velocity),
	m_rotation(args.rotation),
	m_seek_radius(args.seek_radius),
	m_aoe_radius(args.aoe_radius),
	m_critical_chance(args.critical_chance),
	m_critical_multiplier(args.critical_multiplier),
	m_piercing(args.piercing),
	m_guide(args.guide) {
	this->team(static_cast<int>(args.team));
}

void mark::unit::projectile::tick(mark::tick_context& context) {
	double dt = context.dt;
	const auto step = mark::rotate(mark::vector<double>(1, 0), m_rotation) * static_cast<double>(m_velocity) * dt;
	const auto turn_speed = 500.f;
	if (m_guide) {
		if (mark::length(*m_guide - pos()) < m_velocity * dt * 2.0) {
			m_guide.reset();
		} else {
			m_rotation = mark::turn(*m_guide - pos(), m_rotation, turn_speed, dt);
		}
	} else if (m_seek_radius >= 0.f) {
		auto target = m_world.find_one(
			pos(),
			m_seek_radius,
			[this](const mark::unit::base& unit) {
			return unit.team() != this->team()
				&& dynamic_cast<const interface::damageable*> (&unit);
		});
		if (target) {
			m_rotation = mark::turn(target->pos() - pos(), m_rotation, turn_speed, dt);
		}
	}
	pos() += step;
	mark::world::damage_info info;
	info.context = &context;
	info.segment = { pos() - step, pos() };
	info.aoe_radius = m_aoe_radius;
	info.piercing = m_piercing;
	info.damage.damaged = &m_damaged;
	info.damage.team = this->team();
	info.damage.physical = 10.f;
	info.damage.critical_chance = m_critical_chance;
	info.damage.critical_multiplier = m_critical_multiplier;
	info.damage.stun_chance = 0.1f;
	info.damage.stun_duration = 1.f;
	const auto [ maybe_pos, died ] = m_world.damage(info);
	m_dead = died;
	if (maybe_pos) {
		mark::tick_context::spray_info spray;
		spray.image = m_im_tail;
		spray.pos = maybe_pos.value();
		spray.velocity(5.f, 75.f);
		spray.lifespan(0.3f);
		spray.diameter(8.f);
		spray.count = 80;
		spray.cone = 360.f;
		spray.color = { 125, 125, 125, 75 };
		context.render(spray);
	} else {
		// tail: grey dust
		{
			mark::tick_context::spray_info spray;
			spray.image = m_im_tail;
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
			mark::tick_context::spray_info spray;
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
			mark::sprite args;
			args.image = m_im_tail;
			args.pos = pos() - step;
			args.size = 32.f;
			args.color = sf::Color(255, 200, 150, 255);
			context.sprites[0].emplace_back(args);
		}
		{
			mark::sprite args;
			args.image = m_image;
			args.pos = pos();
			args.size = 10.f;
			args.rotation = m_rotation;
			context.sprites[1].emplace_back(args);
		}
		context.lights.emplace_back(pos(), sf::Color::White);
	}
}

auto mark::unit::projectile::dead() const -> bool {
	return m_dead;
}

// Serializer / Deserializer

mark::unit::projectile::projectile(mark::world& world, const YAML::Node& node):
	mark::unit::base(world, node),
	m_image(world.resource_manager().image("shell.png")),
	m_im_tail(world.resource_manager().image("glare.png")),
	m_velocity(node["velocity"].as<float>()),
	m_rotation(node["rotation"].as<float>()),
	m_seek_radius(node["seek_radius"].as<float>()),
	m_aoe_radius(node["aoe_radius"].as<float>()),
	m_critical_chance(node["critical_chance"].as<float>()),
	m_critical_multiplier(node["critical_multiplier"].as<float>()),
	m_piercing(node["piercing"].as<unsigned>()) { }


void mark::unit::projectile::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::projectile::type_name;
	this->serialize_base(out);
	out << Key << "rotation" << Value << m_rotation;
	out << Key << "velocity" << Value << m_velocity;
	out << Key << "seek_radius" << Value << m_seek_radius;
	out << Key << "aoe_radius" << Value << m_aoe_radius;
	out << Key << "critical_chance" << Value << m_critical_chance;
	out << Key << "critical_multiplier" << Value << m_critical_multiplier;
	out << Key << "piercing" << Value << m_piercing;

	out << Key << "damaged" << Value << BeginSeq;
	for (const auto& damaged : m_damaged) {
		out << reinterpret_cast<uint64_t>(damaged);
	}
	out << EndSeq;

	out << EndMap;
}

void mark::unit::projectile::resolve_ref(
	const YAML::Node& node,
	const std::unordered_map<uint64_t, std::weak_ptr<mark::unit::base>>& units) {
	
	if (node["guide"]) {
		// TODO
	}
}
