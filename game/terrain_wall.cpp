#include <array>
#include "terrain_wall.h"
#include "sprite.h"
#include "resource_manager.h"

mark::terrain::wall::wall(mark::resource::manager& resource_manager)
	:m_image(resource_manager.image("snow.png")) {
}

auto mark::terrain::wall::render(mark::vector<int> map_pos) const -> std::vector<mark::sprite> {
	return { mark::sprite(m_image, mark::vector<double>(map_pos) * mark::terrain::grid_size, static_cast<float>(mark::terrain::grid_size)) };
}

auto mark::terrain::wall::collide(
	const mark::vector<double> pos,
	const mark::segment_t other) const -> mark::vector<double> {
	const auto a = mark::terrain::grid_size / 2.0;
	const std::array<segment_t, 4> segments {
		mark::segment_t{ { pos.x - a, pos.y - a }, { pos.x - a, pos.y + a } }, // left
		mark::segment_t{ { pos.x - a, pos.y + a }, { pos.x + a, pos.y + a } }, // bottom
		mark::segment_t{ { pos.x + a, pos.y + a }, { pos.x + a, pos.y - a } }, // right
		mark::segment_t{ { pos.x + a, pos.y - a }, { pos.x - a, pos.y - a } }  // side
	};
	auto min_length = INFINITY;
	auto min = mark::vector<double>(NAN, NAN);
	for (const auto& segment : segments) {
		const auto intersection = mark::intersect(segment, other);
		if (!std::isnan(intersection.x)) {
			const auto length = mark::length(other.first - intersection);
			if (length < min_length) {
				min_length = length;
				min = intersection;
			}
		}
	}
	return min;
}
