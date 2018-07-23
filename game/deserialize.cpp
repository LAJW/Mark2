#include "deserialize.h"
#include <module/base.h>
#include <item/deserialize.h>
#include <stdafx.h>

auto mark::item::deserialize(
	resource::manager& rm,
	mark::random& random,
	const YAML::Node& node)
	-> interface::item_ptr
{
	if (auto module = module::deserialize(rm, random, node)) {
		return module;
	}
	if (auto item = item::deserialize(rm, random, node)) {
		return item;
	}
	return nullptr;
}
