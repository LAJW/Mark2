#include "terrain_floor.h"
#include "world.h"
#include "resource_manager.h"
#include "sprite.h"

mark::terrain::floor::floor(mark::resource::manager& resource_manager, int variant):
	m_image(resource_manager.image("floor.png")),
	m_variant(variant),
	m_resource_manager(resource_manager) {
}

auto mark::terrain::floor::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	float rotation = 0.f;
	if (m_variant == 0 || m_variant == 9) {
		rotation = 0.f;
	} else if (m_variant == 1) {
		rotation = 90.f;
	} else if (m_variant == 2 || m_variant == 10) {
		rotation = 90.f;
	} else if (m_variant == 3) {
		rotation = 0.f;
	} else if (m_variant == 4) {
		
	} else if (m_variant == 5) {
		rotation = 180.f;
	} else if (m_variant == 6 || m_variant == 11) {
		rotation = -90.f;
	} else if (m_variant == 8 || m_variant == 12) {
		rotation = 180.f;
	} else if (m_variant == 7) {
		rotation = -90.f;
	}
	return { mark::sprite(m_image, map_pos.x * 32.f, map_pos.y * 32.f, 32.f, rotation) };
}

void mark::terrain::floor::variant(int variant) {
	m_variant = variant;
	if (m_variant == 0 || m_variant == 2 || m_variant == 6 || m_variant == 8) {
		m_image = m_resource_manager.image("wall-floor-corner.png");
	} else if (m_variant == 1 || m_variant == 3 || m_variant == 5 || m_variant == 7) {
		m_image = m_resource_manager.image("wall-floor.png");
	} else if (m_variant == 4) {
		m_image = m_resource_manager.image("floor.png");
	} else {
		m_image = m_resource_manager.image("wall-floor-edge.png");
	}
}