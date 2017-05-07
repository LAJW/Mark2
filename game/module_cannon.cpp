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
	bar.image = world.resource_manager().image("bar.png"),
	bar.pos = pos + mark::vector<double>(0, -mark::module::size * 2.0),
	bar.type = mark::tick_context::bar_type::health,
	bar.percentage = m_cur_health / m_max_health;
	context.render(bar);
	if (m_shoot) {
		std::unordered_set<mark::idamageable*> damaged;
		for (int i = 1; i < 200; i++) {
			const auto cur_len = mark::module::size * static_cast<double>(i);
			const auto cur_dir = mark::rotate(mark::vector<double>(1, 0), rotation);
			const auto prev = pos + cur_dir * (cur_len - mark::module::size);
			const auto cur = pos + cur_dir * (cur_len + 2.0);
			const auto collision = world.collide({ prev, cur });
			if (!std::isnan(collision.second.x)) {
				mark::idamageable::attributes attr;
				attr.pos = collision.second;
				attr.damaged = &damaged;
				attr.physical = 100.f * static_cast<float>(context.dt);
				attr.team = parent().team();
				if (collision.first) {
					if (collision.first->damage(attr)) {
						context.spray(
							m_im_ray,
							collision.second,
							std::make_pair(25.f, 50.f),
							1.f,
							8.f,
							4,
							0.0,
							rotation + 180.f,
							180.f,
							sf::Color::Red);
						break;
					}
				} else {
					context.spray(
						m_im_ray,
						collision.second,
						std::make_pair(25.f, 50.f),
						1.f,
						8.f,
						4,
						0.0,
						rotation + 180.f,
						180.f,
						sf::Color::Red);
					break;
				}
			}
			context.sprites[0].emplace_back(
				m_im_ray,
				cur,
				mark::module::size,
				rotation,
				0,
				sf::Color::Red);
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
