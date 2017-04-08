#include "unit_minion.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "terrain_base.h"

mark::unit::minion::minion(mark::world& world, mark::vector<double> pos)
	:mark::unit::base(world, pos) {
	m_image = world.resource_manager().image("shield-generator.png");
}

auto mark::unit::minion::render() const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, m_pos.x, m_pos.y, 50.f) };
}

void mark::unit::minion::tick(double dt) {
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
		m_pos += mark::normalize((first - m_pos)) + mark::normalize(direction2) * 100.0 * dt;
	}
}