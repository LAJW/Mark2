#include <stdafx.h>

#include "cargo.h"
#include <algorithm.h>
#include <module/base.h>
#include <unit/modular.h>

namespace mark {
bool in_range(const module::base& a, const module::base& b, double range)
{
	return length(a.pos() - b.pos()) < range;
}

optional<const module::base&>
neighbor_at_pos_in_range(const module::base& root, vi32 pos, double range)
{
	let module = root.parent().module_at(pos);
	if (module && !module->equals(root) && in_range(*module, root, range))
		return module;
	return {};
}

std::unordered_set<not_null<const module::base*>>
neighbors_in_radius(const module::base& root, double radius)
{
	std::unordered_set<not_null<const module::base*>> neighbors;
	let center = root.grid_pos() + vi32(root.size()) / 2;
	let bound = static_cast<int>(std::ceil(radius / 16.));
	for (let offset :
		 range<vi32>({ -bound, -bound }, { bound + 1, bound + 1 })) {
		let module = neighbor_at_pos_in_range(root, center + offset, radius);
		if (module) {
			neighbors.insert(&*module);
		}
	}
	return neighbors;
}

// TODO: Replace with an optional reference
const mark::module::base*
most_damaged_neighbor_in_range(const module::base& root, double range)
{
	let neighbors = neighbors_in_radius(root, range);
	let min_health_neighbour = min_element_v(neighbors, [](let a, let b) {
		return a->cur_health() / a->max_health()
			< b->cur_health() / b->max_health();
	});
	return min_health_neighbour && (*min_health_neighbour)->needs_healing()
		? min_health_neighbour->get()
		: nullptr;
}
} // namespace mark
