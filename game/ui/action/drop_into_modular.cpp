#include "drop_into_modular.h"
#include <unit/modular.h>
#include <slot.h>
#include <interface/item.h>

namespace mark {
namespace ui {
namespace action {

drop_into_modular::drop_into_modular(
	const vi32 drop_pos,
	std::vector<int8_t> bindings)
	: m_drop_pos(drop_pos)
	, m_bindings(move(bindings))
{}

void drop_into_modular::execute(const execute_info& info)
{
	Expects(info.grabbed);
	Expects(!info.modular->attach(m_drop_pos, detach(*info.grabbed)));
	for (let& bind : m_bindings) {
		info.modular->toggle_bind(bind, m_drop_pos);
	}
}

} // namespace action
} // namespace ui
} // namespace mark
