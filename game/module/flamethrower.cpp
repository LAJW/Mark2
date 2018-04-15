#include "flamethrower.h"
#include "targeting_system.h"
#include <property_manager.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

void mark::module::flamethrower::command(const command::any& any)
{
	parent().targeting_system().command(any);
}

void mark::module::flamethrower::update(update_context& context)
{
	this->module::base::update(context);
	this->render(context);
	if (!this->can_shoot()) {
		return;
	}
	let pos = this->pos();
	std::unordered_set<interface::damageable*> damaged;
	let half_cone = m_cone_of_fire / 2.f;
	for (let cur_angle : mark::range(-half_cone, half_cone + 1.f)) {
		let absolute_angle = cur_angle + parent().rotation();
		let offset = rotate(vector<double>(m_range, 0.), absolute_angle);
		let collision = parent().world().damage([&] {
			world::damage_info _;
			_.context = &context;
			_.aoe_radius = 0.f;
			_.piercing = 1;
			_.segment = { pos, pos + offset };
			_.damage.damaged = &damaged;
			_.damage.physical = m_dps * static_cast<float>(context.dt);
			_.damage.team = parent().team();
			return _;
		}());
	}
}

void mark::module::flamethrower::render(update_context& context) const
{
	let pos = this->pos();
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = pos;
		_.size = module::size * 2.f;
		_.rotation = parent().rotation();
		_.color = this->heat_color();
		return _;
	}());
	if (!this->can_shoot()) {
		return;
	}
	context.render([&] {
		update_context::spray_info _;
		_.image = m_image_fire;
		_.pos = pos;
		_.lifespan(0.2f, 0.5f);
		_.diameter(16.f, 64.f);
		_.direction = parent().rotation();
		_.cone = m_cone_of_fire;
		_.velocity(
			static_cast<float>(m_range * 2.5), static_cast<float>(m_range * 3.));
		_.count = 4;
		return _;
	}());
}

auto mark::module::flamethrower::can_shoot() const -> bool
{
	if (m_stunned || !parent().targeting_system().can_shoot()) {
		return false;
	}
	if (!parent().targeting_system().ai()) {
		return true;
	}
	let target = parent().targeting_system().target();
	if (!target) {
		return false;
	}
	return length(*target - this->pos()) <= m_range;
}

auto mark::module::flamethrower::describe() const -> std::string
{
	std::ostringstream os;
	os << "Flamethrower" << std::endl;
	os << "Damage per second: " << m_dps << std::endl;
	os << "Range: " << m_range << std::endl;
	os << "Cone of fire: " << m_cone_of_fire << " degrees" << std::endl;
	return os.str();
}

// Serialize / Deserialize

template <typename prop_man, typename T>
void mark::module::flamethrower::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(cone_of_fire);
	MARK_BIND(dps);
	MARK_BIND(range);
}

void mark::module::flamethrower::bind(mark::property_manager& property_manager)
{
	bind(property_manager, *this);
	base::bind(property_manager);
}

mark::module::flamethrower::flamethrower(
	resource::manager& rm,
	const YAML::Node& node)
	: module::base(rm, node)
	, m_image_base(rm.image("turret.png"))
	, m_image_fire(rm.image("explosion.png"))
{}

void mark::module::flamethrower::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialize(out);
	out << EndMap;
}

auto mark::module::flamethrower::reserved() const noexcept -> reserved_kind
{
	return reserved_kind::front;
}

auto mark::module::flamethrower::passive() const noexcept -> bool
{
	return false;
}
