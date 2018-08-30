#pragma once
#include "base.h"
#include <slot.h>

namespace mark {
namespace ui {
namespace action {

class release_grabbed final : public base
{
public:
	release_grabbed() = default;
	void execute(const execute_info& info) override { *info.grabbed = {}; }
};

} // namespace action
} // namespace ui
} // namespace mark
