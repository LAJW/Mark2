#include "quick_detach.h"
#include <interface/item.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

quick_detach::quick_detach(const vi32 pick_pos)
	: m_pick_pos(pick_pos)
{}

void quick_detach::execute(const execute_info& info)
{
	auto& modular = *info.modular;
	let pos = modular.pos_at(m_pick_pos);
	if (auto detached = modular.detach(m_pick_pos)) {
		if (failure(push(modular, move(detached)))) {
			// It should be possible to reattach a module, if it was
			// already attached
			Expects(success(modular.attach(*pos, move(detached))));
		}
	}
}

} // namespace action
} // namespace ui
} // namespace mark
