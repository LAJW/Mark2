#include "use_grabbed_item.h"
#include <unit/modular.h>
#include <interface/item.h>
#include <slot.h>

namespace mark {
namespace ui {
namespace action {

use_grabbed_item::use_grabbed_item(
	const vi32 drop_pos,
	const std::unordered_map<std::string, YAML::Node>& blueprints)
	: m_drop_pos(drop_pos)
	, m_blueprints(blueprints)
{}
void use_grabbed_item::execute(const execute_info& info)
{
	let module = info.modular->module_at(m_drop_pos);
	let[error, consumed] =
		item_of(*info.grabbed).use_on(*info.random, m_blueprints, *module);
	if (success(error) && consumed) {
		detach(*info.grabbed);
	}
}

} // namespace action
} // namespace ui
} // namespace mark
