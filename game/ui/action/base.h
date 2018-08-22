#pragma once
#include <stdafx.h>
#include <array2d.h>

namespace mark {
class mode_stack;
class slot;
namespace ui {
class tooltip;
namespace action {

class base
{
public:
	virtual ~base() = default;
	struct execute_info final
	{
		using queue_type = array2d<mark::slot, 16, 32>;

		optional<unit::modular&> modular;
		optional<mode_stack&> mode_stack;
		optional<tooltip&> tooltip;
		optional<queue_type&> queue;
		optional<slot&> grabbed;
	};
	virtual void execute(const execute_info& info) = 0;

protected:
	base() = default;
};

} // namespace action
} // namespace ui
} // namespace mark
