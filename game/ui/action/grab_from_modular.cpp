#include "grab_from_modular.h"
#include <interface/item.h>
#include <slot.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

grab_from_modular::grab_from_modular(const vi32 pick_pos)
	: m_pick_pos(pick_pos)
{}

void grab_from_modular::execute(const execute_info& info)
{
	*info.grabbed = { *info.modular, m_pick_pos };
}

} // namespace action
} // namespace ui
} // namespace mark
