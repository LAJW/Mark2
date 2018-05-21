#pragma once
#include <stdafx.h>

namespace mark {
namespace item {
auto deserialize(resource::manager& rm, const YAML::Node& node)
	-> interface::item_ptr;
}
} // namespace mark
