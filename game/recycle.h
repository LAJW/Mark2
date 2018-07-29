#pragma once
#include <stdafx.h>

namespace mark {
std::vector<std::unique_ptr<interface::item>> recycle(
	mark::resource::manager& rm,
	std::vector<std::unique_ptr<interface::item>> items);
}