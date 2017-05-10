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
		context.sprites[0].emplace_back(info);
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
			const auto collision = parent().world().collide({ pos, cur });
			if (!std::isnan(collision.second.x)) {
				mark::idamageable::info attr;
				attr.pos = collision.second;
				attr.damaged = &damaged;
				attr.physical = 200.f * static_cast<float>(context.dt);
				attr.team = parent().team();
				if (collision.first) {
					collision.first->damage(attr);
				}
			}
		}
		m_shoot = false;
	}

}

void mark::module::flamethrower::shoot(mark::vector<double> pos) {
	m_shoot = true;
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
