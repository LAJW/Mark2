#include "base.h"

namespace mark {
namespace ui {
namespace action {

/// Detach a module at a specified position from the modular and push it into
/// cargo (if available)
class quick_detach final : public action::base
{
public:
	explicit quick_detach(const vi32 pick_pos);
	void execute(const execute_info& info) override;

private:
	vi32 m_pick_pos;
};

} // namespace action
} // namespace ui
} // namespace mark
