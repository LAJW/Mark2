#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"
#include "tick_context.h"

mark::unit::minion::minion(mark::world& world, mark::vector<double> pos)
	:mark::unit::base(world, pos) {
	m_image = world.resource_manager().image("mark1.png");
}

auto mark::unit::minion::render() const -> std::vector<mark::sprite> {
	const auto rotation = mark::atan(m_direction) + 90.f;
	return { mark::sprite(m_image, m_pos.x, m_pos.y, 50.f, rotation) };
}

void mark::unit::minion::tick(mark::tick_context& context) {
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
}

auto mark::unit::minion::dead() const -> bool {
	return m_health <= 0;
}