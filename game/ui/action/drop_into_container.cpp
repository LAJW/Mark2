#include "drop_into_container.h"
#include <algorithm/find_one_if.h>
#include <module/cargo.h>
#include <slot.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

void drop_into_container::execute(const execute_info& info)
{
	let containers = info.modular->containers();
	let container = find_one_if(containers, [&](let& cur) {
		return &cur.get() == &m_container;
	});
	Expects(container);
	let result = container->get().attach(m_pos, mark::detach(*info.grabbed));
	Expects(success(result) || result == error::code::stacked);
}

} // namespace action
} // namespace ui
} // namespace mark
