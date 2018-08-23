#include "base.h"

namespace mark {
namespace ui {
namespace action {
class pop_ui_state final : public base
{
public:
	void execute(const execute_info& info) override { info.mode_stack->pop(); }

private:
	mode m_mode;
};
} // namespace action
} // namespace ui
} // namespace mark
