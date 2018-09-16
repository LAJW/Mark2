#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
namespace impl {

/// Calculate a drop position for a module of a specified size
/// \param relative: Mouse position relative to the center of the ship.
/// Specified in the screen units.
/// \param item_size: Size of the item to be dropped into the modular specified
/// in grid units
/// \return position of the top-left module in coordinates suitable for the
/// attach() method
[[nodiscard]] vi32 drop_pos(vd relative, vu32 item_size);

/// Calculate a drag/pick position for a module at a specified relative screen
/// location
/// \param relative: Mouse position relative to the center of the ship.
/// Specified in the screen units.
/// \return Modular grid coordinate nearest to the specified position
[[nodiscard]] vi32 pick_pos(vd relative);

} // namespace impl
} // namespace ui
} // namespace mark
