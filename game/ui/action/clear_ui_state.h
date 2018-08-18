#include "base.h"

namespace mark {
namespace ui {
namespace action {
class clear_ui_state final : public base {
private:
	mode m_mode;

public:
	void execute(const execute_info& info) override
	{
		info.mode_stack->clear();
	}
};

}
}
}
