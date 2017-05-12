#include <algorithm>
#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "particle.h"

mark::module::cannon::cannon(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {
}

void mark::module::cannon::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	m_model.tick(context.dt);
	auto pos = this->pos();
	const auto rotation = parent().rotation();
	const auto model_size = std::max(this->size().x, this->size().y)
		* mark::module::size;
	context.sprites[1].push_back(m_model.render(
		pos,
		model_size,
		rotation,
		sf::Color::White));
	auto& world = parent().world();
	mark::tick_context::bar_info bar;
	if (m_shoot) {
		std::unordered_set<mark::idamageable*> damaged;
		for (int i = 1; i < 200; i++) {
			const auto cur_len = mark::module::size * static_cast<double>(i);
			const auto cur_dir = mark::rotate(mark::vector<double>(1, 0), rotation);
			const auto prev = pos + cur_dir * (cur_len - mark::module::size);
			const auto cur = pos + cur_dir * (cur_len + 2.0);
			mark::world::damage_info info;
			info.context = &context;
			info.aoe_radius = 0.f;
			info.piercing = 1;
			info.segment = { prev, cur };
			info.damage.damaged = &damaged;
			info.damage.physical = 100.f * static_cast<float>(context.dt);
			info.damage.team = parent().team();
			const auto collision = world.damage(info);
			if (collision.second) {
				mark::tick_context::spray_info spray;
				spray.image = m_im_ray;
				spray.pos = collision.first;
				spray.velocity(25.f, 50.f);
				spray.lifespan(1.f);
				spray.diameter(8.f);
				spray.count = 4;
				spray.direction = rotation + 180.f;
				spray.cone = 180.f;
				spray.color = sf::Color::Red;
				context.render(spray);
				break;
			}
			{
				mark::sprite::info info;
				info.image = m_im_ray;
				info.pos = cur;
				info.size = mark::module::size;
				info.rotation = rotation;
				info.color = sf::Color::Red;
				context.sprites[0].emplace_back(info);
			}
		}
		m_shoot = false;
	}
}

void mark::module::cannon::shoot(mark::vector<double> pos) {
	m_shoot = true;
}

std::string mark::module::cannon::describe() const {
	return "Laser Cannon\n"
		"DPS: 60\n";
}
