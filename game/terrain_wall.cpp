#include "terrain_wall.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"

mark::terrain::wall::wall(mark::resource::manager& resource_manager)
	:m_image(resource_manager.image("wall.png")) {
}

auto mark::terrain::wall::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, map_pos.x * 32.f, map_pos.y * 32.f, 32.f) };
}