#include <array>
#include "terrain_floor.h"
#include "world.h"
#include "resource_manager.h"
#include "sprite.h"

mark::terrain::floor::floor(
	mark::resource::manager& resource_manager,
	int variant):
	m_image(resource_manager.image("ice.png")),
	m_variant(variant),
	m_resource_manager(resource_manager) { }

auto mark::terrain::floor::render(mark::vector<int> map_pos) const ->
	std::vector<mark::sprite> {
	mark::sprite::arguments info;
	info.image = m_image;
	info.pos = mark::vector<double>(map_pos) * mark::terrain::grid_size;
	info.size = static_cast<float>(mark::terrain::grid_size);
	info.frame = m_variant;
	return { info };
}

void mark::terrain::floor::variant(int variant) {
	m_variant = variant;
}

auto mark::terrain::floor::collide(
	mark::vector<double> pos,
	mark::segment_t other) const -> mark::vector<double> {
	const auto a = mark::terrain::grid_size / 2.0;
	const auto tile_count = 13;
	const auto tile_type = m_variant % tile_count;
	if (tile_type == 4) {
		return { NAN, NAN };
	} else if (tile_type == 1 || tile_type == 7) {
		const auto segment = segment_t(
			{ pos.x - a, pos.y },
			{ pos.x + a, pos.y }
		);
		return mark::intersect(segment, other);
	} else if (tile_type == 3 || tile_type == 5) {
		const auto segment = segment_t(
			{ pos.x, pos.y - a },
			{ pos.x, pos.y + a}
		);
		return mark::intersect(segment, other);
	} else {
		std::array<segment_t, 2> segments;
		if (tile_type == 0 || tile_type == 9) {
			segments = {
				mark::segment_t({ pos.x, pos.y }, { pos.x, pos.y + a }),
				mark::segment_t({ pos.x, pos.y }, { pos.x + a, pos.y })
			};
		} else if (tile_type == 2 || tile_type == 10) {
			segments = {
				mark::segment_t({ pos.x, pos.y }, { pos.x - a, pos.y }),
				mark::segment_t({ pos.x, pos.y }, { pos.x, pos.y + a })
			};
		} else if (tile_type == 6 || tile_type == 11) {
			segments = {
				mark::segment_t({ pos.x, pos.y }, { pos.x + a, pos.y }),
				mark::segment_t({ pos.x, pos.y }, { pos.x, pos.y - a })
			};
		} else if (tile_type == 8 || tile_type == 12) {
			segments = {
				mark::segment_t({ pos.x, pos.y }, { pos.x - a, pos.y }),
				mark::segment_t({ pos.x, pos.y }, { pos.x, pos.y - a })
			};
		}
		auto min_length = static_cast<double>(INFINITY);
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
}
