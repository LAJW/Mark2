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
	constexpr auto particle_cone = 90.0;
	const auto beam_color = sf::Color::Red;
}

mark::module::cannon::cannon(mark::resource::manager& resource_manager) :
	mark::module::base({ 4, 2 }, resource_manager.image("cannon.png")),
	m_model(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")) {
}

void mark::module::cannon::tick(mark::tick_context& context) {
	m_model.tick(context.dt);
	auto pos = socket()->relative_pos();
	const auto rotation = socket()->rotation();
	const auto model_size = std::max(this->size().x, this->size().y) * mark::module::size;
	context.sprites[1].push_back(m_model.render(pos, model_size, rotation, sf::Color::White));
	if (m_shoot) {
		for (int i = 0; i < 200; i++) {
			const auto cur = pos + mark::rotate(mark::vector<double>(mark::module::size, 0.0), rotation) * static_cast<double>(i);
			auto enemy = socket()->world().find_one(cur, 320.f, [this, &cur](const mark::unit::base& unit) {
				return unit.team() != this->socket()->team()
					&& !unit.invincible()
					&& unit.collides(cur, 0);
			});
			if (!socket()->world().map().traversable(cur) || enemy) {
				if (enemy) {
					enemy->damage(1, cur);
				}
				for (int i = 0; i < particles_per_tick; i++) {
					const auto velocity = static_cast<float>(socket()->world().resource_manager().random_double(min_particle_velocity, max_particle_velocity));
					const auto direction = static_cast<float>(socket()->world().resource_manager().random_double(0, particle_cone)) + 180.0 - particle_cone / 2.0 + rotation;
					context.particles.push_back(mark::particle(m_im_ray, cur, velocity, direction, 1.f, beam_color));
				}
				break;
			} else {
				context.sprites[0].push_back(mark::sprite(m_im_ray, cur, mark::module::size, rotation, 0, beam_color));
			}
		}
		m_shoot = false;
	}
}

void mark::module::cannon::shoot(mark::vector<double> pos) {
	m_shoot = true;
}