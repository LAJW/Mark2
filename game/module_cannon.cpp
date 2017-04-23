#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "particle.h"

mark::module::cannon::cannon(mark::resource::manager& resource_manager) :
	mark::module::base({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {
}

void mark::module::cannon::tick(mark::tick_context& context) {
	m_model.tick(context.dt);
	auto pos = socket()->relative_pos();
	context.sprites[1].push_back(m_model.render(pos, 64.f, socket()->rotation(), sf::Color::White));
	const auto rotation = socket()->rotation();
	if (m_shoot) {
		for (int i = 0; i < 200; i++) {
			const auto cur = pos + mark::rotate(mark::vector<double>(16.0, 0.0), rotation) * static_cast<double>(i);
			auto enemy = socket()->world().find_one(cur, 200.f, [this, &cur](const mark::unit::base& unit) {
				return unit.team() != this->socket()->team() && !unit.invincible() && unit.collides(cur, 0);
			});
			if (!socket()->world().map().traversable(cur)
				|| enemy) {
				if (enemy) {
					enemy->damage(1, cur);
				}
				for (int i = 0; i < 4; i++) {
					const auto velocity = static_cast<float>(socket()->world().resource_manager().random_double(25, 50));
					const auto direction = static_cast<float>(socket()->world().resource_manager().random_double(90, 270)) + rotation;
					context.particles.push_back(mark::particle(m_im_ray, cur, velocity, direction, 1.f, { 255, 0, 0 }));
				}
				break;
			} else {
				context.sprites[0].push_back(mark::sprite(m_im_ray, cur, 16.f, socket()->rotation(), 0, { 255, 0, 0 }));
			}
		}
		m_shoot = false;
	}
}

void mark::module::cannon::shoot(mark::vector<double> pos) {
	m_shoot = true;
}