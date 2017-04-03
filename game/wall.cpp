#include "wall.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"

mark::unit::wall::wall(mark::world& world, mark::vector<double> pos)
	:mark::unit::base(world, pos) {
	m_image = world.resource_manager().image("image.png");
}

auto mark::unit::wall::render() const -> std::vector<mark::sprite> {
	if (m_pos.x > 0 && m_pos.x < 1000 && m_pos.y > 0 && m_pos.y < 1000) {
		return { mark::sprite(m_image, m_pos.x, m_pos.y, 32.f) };
	} else {
		return { };
	}
}