#include "grab_from_container.h"
#include <slot.h>
#include <unit/modular.h>
#include <module/cargo.h>
#include <algorithm/find_if.h>

namespace mark {
namespace ui {
namespace action {

void grab_from_container::execute(const execute_info& info)
{
	let container = find_if(info.modular->containers(), [&](let& cur) {
		return &cur.get() == &m_container;
	});
	(*info.grabbed) = { container->get(), m_pos };
}

} // namespace action
} // namespace ui
} // namespace mark
