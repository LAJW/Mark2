#include "terrain_floor.h"
#include "world.h"
#include "resource_manager.h"
#include "sprite.h"

mark::terrain::floor::floor(mark::resource::manager& resource_manager, int variant):
	m_image(resource_manager.image("ice.png")),
	m_variant(variant),
	m_resource_manager(resource_manager) {
}

auto mark::terrain::floor::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, mark::vector<double>(map_pos) * mark::terrain::grid_size, static_cast<float>(mark::terrain::grid_size), 0.f, m_variant) };
}

void mark::terrain::floor::variant(int variant) {
	m_variant = variant;
}

auto mark::terrain::floor::collide(mark::vector<double> world_pos, mark::segment_t line) const -> mark::vector<double> {
	return { NAN, NAN };
}
