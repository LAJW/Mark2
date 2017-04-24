#include "terrain_wall.h"
#include "sprite.h"
#include "resource_manager.h"

mark::terrain::wall::wall(mark::resource::manager& resource_manager)
	:m_image(resource_manager.image("snow.png")) {
}

auto mark::terrain::wall::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, mark::vector<double>(map_pos) * mark::terrain::grid_size, mark::terrain::grid_size) };
}