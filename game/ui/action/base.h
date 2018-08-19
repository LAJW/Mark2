#pragma once
#include <stdafx.h>

namespace mark {
class mode_stack;
namespace ui {
class tooltip;
namespace action {

class base
{
public:
	virtual ~base() = default;
	struct execute_info final
	{
		optional<unit::modular&> modular;
		optional<mode_stack&> mode_stack;
		optional<tooltip&> tooltip;
	};
	virtual void execute(const execute_info& info) = 0;

protected:
	base() = default;
};

} // namespace action
} // namespace ui
} // namespace mark
