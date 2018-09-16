#include "ui.h"
#include <module/base.h>

namespace mark {
namespace ui {
namespace impl {

vi32 drop_pos(const vd relative, const vu32 item_size)
{
	Expects(item_size.x > 0);
	Expects(item_size.y > 0);
	let constexpr field_size = static_cast<double>(mark::module::size);
	const auto proc_coord = [&](const double relative, const unsigned uSize) {
		let size = static_cast<double>(uSize) * field_size;
		return gsl::narrow<int>(
			uSize % 2 == 0
				? std::round((relative - size / 2.) / field_size)
				: std::floor(
					  (relative + field_size / 2. - size / 2.) / field_size));
	};
	return { proc_coord(relative.x, item_size.x),
			 proc_coord(relative.y, item_size.y) };
}

vi32 pick_pos(const vd relative)
{
	return floor(relative / static_cast<double>(module::size));
}

} // namespace impl
} // namespace ui
} // namespace mark
