#include "terrain_floor.h"
#include "world.h"
#include "resource_manager.h"
#include "sprite.h"

mark::terrain::floor::floor(mark::world& world)
	:m_image(world.resource_manager().image("floor.png")) {
}

auto mark::terrain::floor::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, map_pos.x * 32.f, map_pos.y * 32.f, 32.f) };
}