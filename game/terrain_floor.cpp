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
	return { mark::sprite(m_image, map_pos.x * 32.f, map_pos.y * 32.f, 32.f, 0.f, m_variant) };
}

void mark::terrain::floor::variant(int variant) {
	m_variant = variant;
}