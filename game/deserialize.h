#pragma once
#include <stdafx.h>

namespace mark {
auto deserialize(resource::manager& rm, const YAML::Node& node)
	-> interface::item_ptr;
} // namespace mark
