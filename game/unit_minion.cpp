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
	if (mark::length(direction2)) {
		const auto step = direction2 * 100.0 * dt;
		const auto new_pos = step + m_pos;
		const auto map_pos = mark::vector<int>(std::floor(new_pos.x / 32.0), std::floor(new_pos.y / 32.0)) + mark::vector<int>(500, 500);
		const auto& map = m_world.map();
		if (map[map_pos.x][map_pos.y] != nullptr && map[map_pos.x][map_pos.y]->traversable()) {
			m_pos = new_pos;
		}
	} else if (length != 0) {
		const auto direction = distance / length;
		const auto step = (direction + direction2 * 0.7) * 100.0 * dt;
		const auto new_pos = step + m_pos;
		const auto map_pos = mark::vector<int>(std::floor(new_pos.x / 32.0), std::floor(new_pos.y / 32.0)) + mark::vector<int>(500, 500);
		const auto& map = m_world.map();
		if (map[map_pos.x][map_pos.y] != nullptr && map[map_pos.x][map_pos.y]->traversable()) {
			m_pos = new_pos;
		}
	}
}