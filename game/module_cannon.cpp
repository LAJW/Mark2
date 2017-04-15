#include "module_cannon.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include "particle.h"

mark::module::cannon::cannon(mark::resource::manager& resource_manager) :
	mark::module::base({ 4, 2 }),
	m_im_body(resource_manager.image("cannon.png")),
	m_im_ray(resource_manager.image("ray.png")),
	m_frame_cooldown(0.2f) {
}

void mark::module::cannon::tick(mark::tick_context& context) {
	m_frame_cooldown.tick(context.dt);
	auto pos = socket()->relative_pos();
	if (m_frame_cooldown.trigger()) {
		m_frame = (m_frame + 1) % static_cast<unsigned>(m_im_body->getSize().x / m_im_body->getSize().y);
	}
	context.sprites[1].push_back(mark::sprite(m_im_body, pos, 64.f, socket()->rotation(), m_frame));
	const auto rotation = socket()->rotation();
	for (int i = 0; i < 200; i++) {
		const auto cur = pos + mark::rotate(mark::vector<double>(16.0, 0.0), rotation) * static_cast<double>(i);
		auto nearby = socket()->world().find(cur, 50.f);
		auto enemy_it = std::find_if(nearby.begin(), nearby.end(), [this](std::shared_ptr<mark::unit::base>& unit) {
			return unit->team() != this->socket()->team() && !unit->invincible();
		});
		if (!socket()->world().map().traversable(cur)
			|| enemy_it != nearby.end()) {
			if (enemy_it != nearby.end()) {
				(*enemy_it)->damage(1, cur);
			}
			for (int i = 0; i < 4; i++) {
				const auto velocity = socket()->world().resource_manager().random_double(25, 50);
				const auto direction = static_cast<float>(socket()->world().resource_manager().random_double(90, 270)) + rotation;
				context.particles.push_back(mark::particle(m_im_ray, cur, velocity, direction, 1.f, { 255, 0, 0 }));
			}
			break;
		} else {
			context.sprites[0].push_back(mark::sprite(m_im_ray, cur, 16.f, socket()->rotation(), 0, { 255, 0, 0 }));
		}
	}
}