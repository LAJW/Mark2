#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
namespace impl {

[[nodiscard]] vi32
drop_pos(const vd relative, const mark::interface::item& item);

[[nodiscard]] vi32 drop_pos(const vd relative, const vu32 item_size);

[[nodiscard]] vi32 pick_pos(const vd relative);

} // namespace impl
} // namespace ui
} // namespace mark
