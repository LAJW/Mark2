#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"
#include "unit_projectile.h"

mark::unit::minion::minion(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_bareer_reaction(0.01f, 255.f, 0.1f, 1.f),
	m_im_shield(world.resource_manager().image("shield-reaction.png")) {
	m_image = world.resource_manager().image("mark1.png");
}

void mark::unit::minion::tick(mark::tick_context& context) {
	m_bareer_reaction.tick(context.dt);
	double dt = context.dt;
	auto neighbors = m_world.find(m_pos, 50.0);
	const auto distance = m_world.camera() - m_pos;
	const auto length = mark::length(distance);
	auto direction2 = mark::vector<double>(0, 0);
	for (const auto& neighbor : neighbors) {
		auto dist = (m_pos - neighbor->pos());
		auto len = mark::length(dist);
		if (len) {
			dist = dist / len;
			direction2 += dist;
		}
	}
	const auto path = m_world.map().find_path(m_pos, m_world.camera());
	if (path.size() > 3) {
		const auto first = mark::vector<double>(path[path.size() - 3]);
		const auto direction = mark::normalize((first - m_pos)) + mark::normalize(direction2);
		const auto turn_direction = mark::sgn(mark::atan(mark::rotate(direction, -mark::atan(m_direction))));
		m_direction = rotate(m_direction, turn_direction  * 180.f * dt);
		m_pos += direction * 100.0 * dt;
	}
	m_clock += context.dt;
	if (m_clock > 0.2f) {
		m_frame = (m_frame + 1) % static_cast<int>(m_image->getSize().x / m_image->getSize().y);
		m_clock = 0.f;

		auto projectile = std::make_shared<mark::unit::projectile>(m_world, m_pos, mark::atan(m_direction));
		projectile->team(this->team());
		context.units.emplace_back(std::move(projectile));
	}

	const auto rotation = mark::atan(m_direction);
	context.sprites[1].push_back(mark::sprite(m_image, m_pos, 116.f, rotation, m_frame));
	context.sprites[2].push_back(mark::sprite(m_im_shield, m_pos, 116.f, m_bareer_direction, 0, sf::Color(155, 255, 255, static_cast<uint8_t>(m_bareer_reaction.get() + 1.f))));
	m_clock += context.dt;

}

auto mark::unit::minion::dead() const -> bool {
	return m_health <= 0;
}

void mark::unit::minion::damage(unsigned amount, mark::vector<double> pos) {
	m_bareer_reaction.trigger();
	this->m_health -= amount;
	m_bareer_direction = mark::atan(pos - m_pos);
}
