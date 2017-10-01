#include "stdafx.h"
#include "module_flamethrower.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "world.h"
#include "resource_manager.h"

mark::module::flamethrower::flamethrower(mark::resource::manager& manager) :
	mark::module::base({ 2, 2 }, manager.image("turret.png")),
	m_image_base(manager.image("turret.png")) { }

void mark::module::flamethrower::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	const auto pos = this->pos();

	{
		mark::sprite::info info;
		info.image = m_image_base;
		info.pos = pos;
		info.size = mark::module::size * 2.f;
		info.rotation = parent().rotation();
		info.color = this->heat_color();
		context.sprites[2].emplace_back(info);
	}

	if (m_shoot) {
		mark::tick_context::spray_info info;
		info.image = parent().world().resource_manager().image("explosion.png");
		info.pos = pos;
		info.lifespan(0.2f, 0.5f);
		info.diameter(16.f, 64.f);
		info.direction = parent().rotation();
		info.cone = 30.f;
		info.velocity(700.f, 1000.f);
		info.count = 4;
		context.render(info);

		std::unordered_set<mark::idamageable*> damaged;
		for (float i = -15; i < 15; i++) {
			const auto cur = pos + mark::rotate(mark::vector<double>(300, 0), i + parent().rotation());
			mark::world::damage_info info;
			info.context = &context;
			info.aoe_radius = 0.f;
			info.piercing = 1;
			info.segment = { pos, cur };
			info.damage.damaged = &damaged;
			info.damage.physical = 200.f * static_cast<float>(context.dt);
			info.damage.team = parent().team();
			const auto collision = parent().world().damage(info);
		}
	}

}

void mark::module::flamethrower::shoot(mark::vector<double> pos, bool release) {
	m_shoot = !release;
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

mark::module::flamethrower::flamethrower(mark::resource::manager& rm, const YAML::Node& node):
	mark::module::base(rm, node),
	m_image_base(rm.image("turret.png")) { }

void mark::module::flamethrower::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	this->serialize_base(out);
	out << EndMap;
}

auto mark::module::flamethrower::reserved() const noexcept -> reserved_type
{ return reserved_type::front; }

auto mark::module::flamethrower::passive() const noexcept -> bool
{ return false; }
