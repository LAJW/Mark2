#include "recycle.h"
#include <algorithm.h>
#include <algorithm/find_if.h>
#include <algorithm/has_one.h>
#include <interface/item.h>
#include <module/cargo.h>
#include <slot.h>
#include <unit/modular.h>

namespace mark {
namespace ui {

using queue_type = array2d<mark::slot, 16, 32>;
[[nodiscard]] static auto reserved(const queue_type& queue)
{
	array2d<bool, 16, 32> reserved;
	reserved.fill(false);
	for (let & [ i, slot ] : enumerate(queue)) {
		if (slot.empty()) {
			continue;
		}
		for (let j : range(i, i + vector<size_t>(item_of(slot).size()))) {
			reserved[j] = true;
		}
	}
	return reserved;
}

[[nodiscard]] static optional<vector<size_t>>
find_empty_pos_for(const queue_type& queue, const interface::item& item)
{
	let reserved = mark::ui::reserved(queue);
	for (auto pair : enumerate(queue)) {
		auto& [i, slot] = pair;
		let item_size = item.size();
		if (all_of(range(i, i + vector<size_t>(item_size)), [&](let pos) {
				return pos.x < queue.size().x && pos.y < queue.size().y
					&& !reserved[pos];
			})) {
			return i;
		}
	}
	return {};
}

namespace action {

void recycle::execute(const execute_info& info)
{
	let containers = info.modular->containers();
	let container = find_if(
		containers, [&](let& cur) { return &cur.get() == &m_container; });
	Expects(container != containers.end());
	auto& queue = *info.queue;
	auto slot = mark::slot(container->get(), m_pos);
	if (!has_one(queue.data(), slot)) {
		auto& item = item_of(slot);
		if (let queue_pos = find_empty_pos_for(queue, item)) {
			(*info.queue)[*queue_pos] = slot;
		}
	}
}

} // namespace action
} // namespace ui
} // namespace mark
