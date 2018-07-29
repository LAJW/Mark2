#pragma once
#include <stdafx.h>

namespace mark {
auto recycle(
	mark::resource::manager& rm,
	std::vector<std::unique_ptr<interface::item>> items)
	-> std::vector<std::unique_ptr<interface::item>>;
}
