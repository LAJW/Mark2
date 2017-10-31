#include "stdafx.h"
#include "module_flamethrower.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "world.h"
#include "resource_manager.h"
#include "unit_modular.h"

mark::module::flamethrower::flamethrower(resource::manager& manager) :
	module::base({ 2, 2 }, manager.image("turret.png")),
	m_image_base(manager.image("turret.png")) { }

void mark::module::flamethrower::command(const command::any & any)
{
	if (std::holds_alternative<command::use>(any)) {
		m_shoot = true;
	} else if (std::holds_alternative<command::release>(any)) {
		m_shoot = false;
	}
}

void mark::module::flamethrower::tick(tick_context& context) {
	this->module::base::tick(context);
	const auto pos = this->pos();

	{
		sprite info;
		info.image = m_image_base;
		info.pos = pos;
		info.size = module::size * 2.f;
		info.rotation = parent().rotation();
		info.color = this->heat_color();
		context.sprites[2].emplace_back(info);
	}

	if (m_shoot) {
		tick_context::spray_info spray_info;
		spray_info.image = parent().world().resource_manager().image("explosion.png");
		spray_info.pos = pos;
		spray_info.lifespan(0.2f, 0.5f);
		spray_info.diameter(16.f, 64.f);
		spray_info.direction = parent().rotation();
		spray_info.cone = 30.f;
		spray_info.velocity(700.f, 1000.f);
		spray_info.count = 4;
		context.render(spray_info);

		std::unordered_set<interface::damageable*> damaged;
		for (float i = -15; i < 15; i++) {
			const auto cur = pos + rotate(vector<double>(300, 0), i + parent().rotation());
			world::damage_info damage_info;
			damage_info.context = &context;
			damage_info.aoe_radius = 0.f;
			damage_info.piercing = 1;
			damage_info.segment = { pos, cur };
			damage_info.damage.damaged = &damaged;
			damage_info.damage.physical = 200.f * static_cast<float>(context.dt);
			damage_info.damage.team = parent().team();
			const auto collision = parent().world().damage(damage_info);
		}
	}

}

auto mark::module::flamethrower::describe() const->std::string {
	return "Battery";
}

auto mark::module::flamethrower::harvest_energy() -> float {
	return 0.f;
}

auto mark::module::flamethrower::energy_ratio() const -> float {
	return 0.f;
}

// Serialize / Deserialize

mark::module::flamethrower::flamethrower(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("turret.png")) { }

void mark::module::flamethrower::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::flamethrower::reserved() const noexcept -> reserved_type
{ return reserved_type::front; }

auto mark::module::flamethrower::passive() const noexcept -> bool
{ return false; }
