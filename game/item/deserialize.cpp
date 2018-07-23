#include "deserialize.h"
#include "chaos_orb.h"
#include "shard.h"
#include <stdafx.h>

auto mark::item::deserialize(
	resource::manager& rm,
	mark::random& random,
	const YAML::Node& node)
	-> interface::item_ptr
{
	(void)random; // Ignoring parameter for now. Will be useful in the future
	let type_name = node["type"].as<std::string>();
	if (type_name == item::chaos_orb::type_name) {
		return std::make_unique<item::chaos_orb>(rm, node);
	}
	if (type_name == item::shard::type_name) {
		return std::make_unique<item::shard>(rm, node);
	}
	return nullptr;
}
