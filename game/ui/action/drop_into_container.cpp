#include "drop_into_container.h"
#include <algorithm/find_if.h>
#include <module/cargo.h>
#include <slot.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

void drop_into_container::execute(const execute_info& info)
{
	let container = find_if(info.modular->containers(), [&](let& cur) {
		return &cur.get() == &m_container;
	});
	let result = container->get().attach(m_pos, mark::detach(*info.grabbed));
	Expects(success(result) || result == error::code::stacked);
}

} // namespace action
} // namespace ui
} // namespace mark
