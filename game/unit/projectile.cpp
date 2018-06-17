#include "projectile.h"
#include "modular.h"
#include <algorithm.h>
#include <module/base.h>
#include <property_manager.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/damageable.h>
#include <update_context.h>
#include <world.h>

namespace {

template <typename prop_man, typename T>
void bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(physical);
	MARK_BIND(antimatter);
	MARK_BIND(energy);
	MARK_BIND(heat);
	MARK_BIND(critical_chance);
	MARK_BIND(critical_multiplier);
	MARK_BIND(velocity);
	MARK_BIND(acceleration);
	MARK_BIND(aoe_radius);
	MARK_BIND(seek_radius);
	MARK_BIND(knockback);
	MARK_BIND(piercing);
	MARK_BIND(range);
	MARK_BIND(projectile_angular_velocity);
	MARK_BIND(knockback);
	MARK_BIND(bounces);
}

static auto validate(const mark::unit::projectile::info& args)
{
	Expects(!std::isnan(args.rotation));
	return args;
}
} // namespace

void mark::bind(property_manager& property_manager, projectile_config& instance)
{
	::bind(property_manager, instance);
}

void mark::bind(
	property_serializer& property_manager,
	const projectile_config& instance)
{
	::bind(property_manager, instance);
}

mark::unit::projectile::projectile(const unit::projectile::info& args)
	: unit::projectile::projectile(::validate(args), true)
{}

mark::unit::projectile::projectile(const unit::projectile::info& args, bool)
	: unit::base(args)
	, projectile_config(args)
	, m_image(args.world->resource_manager().image("shell.png"))
	, m_im_tail(args.world->resource_manager().image("glare.png"))
	, m_im_explosion(args.world->resource_manager().image("explosion.png"))
	, m_rotation(args.rotation)
	, m_rotation_lfo(args.lfo, args.phase)
	, m_guide(args.guide)
{}

namespace mark {
static auto dot_product(vd a, vd b) { return a.x * b.x + a.y * b.y; }
static auto reflect(vd dir, vd normal)
{
	let n = normalize(normal);
	return dir - 2 * dot_product(dir, n) * n;
}

static auto
reflect(const interface::damageable& damageable, vd collision, float rotation)
{
	let center = [&] {
		if (let module = dynamic_cast<const module::base*>(&damageable)) {
			return module->pos();
		}
		let unit = dynamic_cast<const unit::damageable*>(&damageable);
		Expects(unit);
		return unit->pos();
	}();
	let dir = rotate(vd(1, 0), rotation);
	let normal = center - collision;
	return atan(reflect(dir, normal));
}
} // namespace mark

void mark::unit::projectile::update(update_context& context)
{
	let dt = context.dt;
	m_rotation_lfo.update(dt);
	let rotation = m_rotation + m_rotation_lfo.get() * 15.f;
	m_velocity = std::max(0., m_velocity + m_acceleration * dt);
	let step = rotate(vd(1, 0), rotation) * m_velocity * dt;
	let turn_speed = m_projectile_angular_velocity;
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

	let[collisions, terrain_hit, reflected_angle] = world().damage([&] {
		world::damage_info _;
		_.context = &context;
		_.segment = { pos() - step * 1.2, pos() };
		_.aoe_radius = m_aoe_radius;
		_.piercing = m_piercing;
		_.damage.knocked = &m_knocked;
		_.damage.damaged = &m_damaged;
		_.damage.team = this->team();
		_.damage.physical = m_physical;
		_.damage.antimatter = m_antimatter;
		_.damage.critical_chance = m_critical_chance;
		_.damage.critical_multiplier = m_critical_multiplier;
		_.damage.stun_chance = 0.1f;
		_.damage.stun_duration = 1.f;
		_.damage.knockback = m_knockback;
		return _;
	}());
	let reflective_hit = false && // TODO: Filter out dead objects
		!m_damaged.empty() && any_of(m_damaged, [&](let damageable) {
							 return damageable->reflective();
						 });
	if (m_velocity == 0.
		|| collisions.size() >= m_piercing && !reflective_hit) {
		m_dead = true;
	} else {
		if (!reflective_hit) {
			m_piercing -= collisions.size();
		} else {
			team(team() ? 0 : 1);
		}
		if (terrain_hit) {
			m_rotation = reflected_angle;
			pos(pos() - collisions.back()
				+ rotate(vd(5., 0.), reflected_angle));
		} else if (!m_damaged.empty() && !collisions.empty()) {
			// TODO: This doesn't work with more than 1 reflected target
			let& damaged = *m_damaged.begin()->get();
			m_rotation = reflect(damaged, collisions.back(), m_rotation);
		}
		if (reflective_hit) {
			m_damaged.clear();
			m_guide = nullptr;
		}
	}
	if (!collisions.empty() && terrain_hit) {
		pos(collisions.back());
	}
	render([&, &collisions = collisions] {
		render_info _;
		_.context = context;
		_.collisions = collisions;
		_.step = step;
		_.is_heavy_damage = m_physical > 100;
		return _;
	}());
}

void mark::unit::projectile::render(const render_info& info) const
{
	auto& context = *info.context;
	let is_heavy_damage = info.is_heavy_damage;
	let& collisions = info.collisions;
	if (info.is_heavy_damage && !info.collisions.empty()) {
		context.crit = true;
	}
	for (let collision : collisions) {
		context.render([&] {
			update_context::spray_info _;
			_.image = is_heavy_damage ? m_im_explosion : m_im_tail;
			_.pos = collision;
			_.velocity(5.f, is_heavy_damage ? 500.f : 75.f);
			_.lifespan(is_heavy_damage ? .7f : .3f);
			_.diameter(is_heavy_damage ? 32.f : 8.f);
			_.count = is_heavy_damage ? 80 : 40;
			_.cone = 360.f;
			if (m_antimatter > m_physical) {
				_.color = { 200, 0, 255, 75 };
			} else {
				_.color = { 125, 125, 125, 75 };
			}
			return _;
		}());
	}
	// tail: grey dust
	context.render([&] {
		update_context::spray_info _;
		_.image = is_heavy_damage ? m_im_explosion : m_im_tail;
		_.pos = pos();
		_.velocity(100.f);
		_.lifespan(0.3f);
		_.diameter(8.f);
		_.count = 4;
		_.step = mark::length(info.step);
		_.direction = m_rotation + 180.f;
		_.cone = 30.f;
		_.color = { 175, 175, 175, 75 };
		return _;
	}());
	// tail: white overlay
	context.render([&] {
		update_context::spray_info _;
		_.image = m_im_tail;
		_.pos = pos();
		_.velocity(75.f);
		_.lifespan(0.15f);
		_.diameter(8.f);
		_.count = 4;
		_.step = mark::length(info.step);
		_.direction = m_rotation + 180.f;
		_.cone = 30.f;
		if (m_antimatter > m_physical) {
			_.color = { 200, 0, 255, 255 };
		} else {
			_.color = sf::Color::White;
		}
		return _;
	}());
	context.sprites[0].emplace_back([&] {
		sprite _;
		_.image = m_im_tail;
		_.pos = pos() - info.step;
		_.size = 32.f;
		_.color = sf::Color(255, 200, 150, 255);
		return _;
	}());
	context.sprites[1].emplace_back([&] {
		sprite _;
		_.image = m_image;
		_.pos = pos();
		_.size = 10.f;
		_.rotation = m_rotation;
		return _;
	}());
	context.lights.emplace_back(pos(), sf::Color::White);
}

auto mark::unit::projectile::dead() const -> bool { return m_dead; }

// Serializer / Deserializer

mark::unit::projectile::projectile(mark::world& world, const YAML::Node& node)
	: unit::base(world, node)
	, m_image(world.resource_manager().image("shell.png"))
	, m_im_tail(world.resource_manager().image("glare.png"))
	, m_im_explosion(world.resource_manager().image("explosion.png"))
{
	property_manager property_manager(world.resource_manager());
	mark::bind(property_manager, *this);
	Expects(!property_manager.deserialize(node));
}

void mark::unit::projectile::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::projectile::type_name;
	base::serialize(out);
	property_serializer serializer;
	mark::bind(serializer, *this);
	serializer.serialize(out);
	out << Key << "damaged" << Value << BeginSeq;
	for (let& damaged : m_damaged) {
		out << reinterpret_cast<uint64_t>(damaged.get());
	}
	out << EndSeq;

	out << EndMap;
}

void mark::unit::projectile::resolve_ref(
	const YAML::Node&,
	const std::unordered_map<uint64_t, weak_ptr<unit::base>>&)
{
	// TODO: Update guides
}
