#include "stack_into_container.h"
#include <algorithm/find_if.h>
#include <module/cargo.h>
#include <slot.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

void stack_into_container::execute(const execute_info& info)
{
	let container = find_if(info.modular->containers(), [&](let& cur) {
		return &cur.get() == &m_container;
	});
	// TODO: If not fully stacked, put the item back where
	// it was
	(void)container->get().attach(m_pos, detach(*info.grabbed));
}

} // namespace action
} // namespace ui
} // namespace mark
