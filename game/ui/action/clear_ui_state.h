#include "base.h"
#include <mode_stack.h>

namespace mark {
namespace ui {
namespace action {
class clear_ui_state final : public base
{
public:
	void execute(const execute_info& info) override
	{
		info.mode_stack->clear();
	}

private:
	mode m_mode;
};

} // namespace action
} // namespace ui
} // namespace mark
