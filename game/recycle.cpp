#include "recycle.h"
#include <algorithm.h>
#include <algorithm/range.h>
#include <item/chaos_orb.h>
#include <item/shard.h>

namespace mark {

auto recycle(
	mark::resource::manager& rm,
	std::vector<std::unique_ptr<interface::item>> items)
	-> std::vector<std::unique_ptr<interface::item>>
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
	if (chaos_orbs_to_make > 0) {
		// TODO: Create stacks of 20 and only then call this
		let stack_count = chaos_orbs_to_make / 20;
		let remaining_orbs = chaos_orbs_to_make % 20;
		for (let i : range(stack_count)) {
			(void)i;
			result.push_back(std::make_unique<mark::item::chaos_orb>(rm, [&] {
				auto _ = YAML::Node();
				_["quantity"] = 20;
				return _;
			}()));
		}
		result.push_back(std::make_unique<mark::item::chaos_orb>(rm, [&] {
			auto _ = YAML::Node();
			_["quantity"] = remaining_orbs;
			return _;
		}()));
	}
	if (remaining_shards > 0) {
		result.push_back(std::make_unique<mark::item::shard>(rm, [&] {
			auto _ = YAML::Node();
			_["quantity"] = remaining_shards;
			return _;
		}()));
	}
	return result;
}

} // namespace mark
