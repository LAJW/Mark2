#include "base.h"
#include <mode_stack.h>

namespace mark {
namespace ui {
namespace action {
class push_ui_state final : public base
{
public:
	explicit push_ui_state(mode mode)
		: m_mode(mode)
	{}
	void execute(const execute_info& info) override
	{
		info.mode_stack->push(m_mode);
	}
	[[nodiscard]] mode mode() const noexcept { return m_mode; }

private:
	mark::mode m_mode;
};

} // namespace action
} // namespace ui
} // namespace mark
