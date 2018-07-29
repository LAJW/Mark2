#include "recycle.h"
#include <item/chaos_orb.h>
#include <item/shard.h>
#include <algorithm.h>
#include <algorithm/range.h>

namespace mark {

std::vector<std::unique_ptr<interface::item>> mark::recycle(
	mark::resource::manager& rm,
	std::vector<std::unique_ptr<interface::item>> items)
{
	std::vector<std::unique_ptr<interface::item>> result;
	let shard_count = accumulate(
		items,
		size_t(0),
		[&](size_t total, const std::unique_ptr<interface::item>& item) {
		if (item == nullptr) {
			return total;
		} else if (
			let shard = dynamic_cast<const item::shard*>(item.get())) {
			return total + shard->quantity();
		} else {
			return total + 1;
		}
	});
	let chaos_orbs_to_make = shard_count / 20;
	let remaining_shards = shard_count % 20;
	for (let i : range(chaos_orbs_to_make)) {
		(void)i;
		result.push_back(
			std::make_unique<mark::item::chaos_orb>(rm, YAML::Node()));
	}
	for (let i : range(remaining_shards)) {
		(void)i;
		result.push_back(std::make_unique<mark::item::shard>(rm, YAML::Node()));
	}
	return result;
}

} // namespace mark