#include <algorithm>
#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "particle.h"

namespace {
	constexpr auto particles_per_tick = 4;
	constexpr auto min_particle_velocity = 25.0;
	constexpr auto max_particle_velocity = 50.0;
	constexpr auto particle_cone = 90.f;
	const auto beam_color = sf::Color::Red;
}

mark::module::cannon::cannon(mark::resource::manager& resource_manager) :
	mark::module::base({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {
}

void mark::module::cannon::tick(mark::tick_context& context) {
	m_model.tick(context.dt);
	auto pos = this->pos();
	const auto rotation = parent().rotation();
	const auto model_size = std::max(this->size().x, this->size().y) * mark::module::size;
	context.sprites[1].push_back(m_model.render(pos, model_size, rotation, sf::Color::White));
	context.render_bar(parent().world().resource_manager().image("bar.png"), pos + mark::vector<double>(0, -mark::module::size * 2.0), mark::tick_context::bar_type::health, m_cur_health / m_max_health);
	if (m_shoot) {
		std::unordered_set<mark::idamageable*> damaged;
		for (int i = 0; i < 200; i++) {
			const auto cur = pos + mark::rotate(mark::vector<double>(mark::module::size, 0.0), rotation) * static_cast<double>(i);
			mark::idamageable::attributes attr;
			attr.pos = cur;
			attr.damaged = &damaged;
			attr.physical = 1.f * static_cast<float>(context.dt);
			attr.team = parent().team();
			if (parent().world().damage(attr) || !parent().world().map().traversable(cur)) {
				context.spray(m_im_ray, cur, std::make_pair(min_particle_velocity, max_particle_velocity), 1.f, 8.f, 4, 0.0, rotation + 180.f, 180.f, sf::Color::Red);
				break;
			} else {
				context.sprites[0].emplace_back(m_im_ray, cur, mark::module::size, rotation, 0, beam_color);
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
