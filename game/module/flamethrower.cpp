#include <stdafx.h>
#include "flamethrower.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>
#include <unit/modular.h>
#include <world.h>

void mark::module::flamethrower::command(const command::any& any)
{
	if (std::holds_alternative<command::activate>(any)) {
		m_shoot = true;
	}
	else if (std::holds_alternative<command::release>(any)) {
		m_shoot = false;
	}
}

void mark::module::flamethrower::tick(tick_context& context)
{
	this->module::base::tick(context);
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
	if (!m_stunned && m_shoot) {
		context.render([&] {
			tick_context::spray_info _;
			_.image =
				parent().world().resource_manager().image("explosion.png");
			_.pos = pos;
			_.lifespan(0.2f, 0.5f);
			_.diameter(16.f, 64.f);
			_.direction = parent().rotation();
			_.cone = 30.f;
			_.velocity(700.f, 1000.f);
			_.count = 4;
			return _;
		}());

		std::unordered_set<interface::damageable*> damaged;
		for (float i = -15; i < 15; i++) {
			let cur =
				pos + rotate(vector<double>(300, 0), i + parent().rotation());
			let collision = parent().world().damage([&] {
				world::damage_info _;
				_.context = &context;
				_.aoe_radius = 0.f;
				_.piercing = 1;
				_.segment = {pos, cur};
				_.damage.damaged = &damaged;
				_.damage.physical = 200.f * static_cast<float>(context.dt);
				_.damage.team = parent().team();
				return _;
			}());
		}
	}
}

auto mark::module::flamethrower::describe() const -> std::string
{
	return "Battery";
}

// Serialize / Deserialize

template <typename prop_man, typename T>
void mark::module::flamethrower::bind(prop_man& property_manager, T& instance)
{
	(void)property_manager;
	(void)instance;
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
{
}

void mark::module::flamethrower::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
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
