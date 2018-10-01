#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {
class grab_from_modular final : public action::base
{
public:
	explicit grab_from_modular(const vi32 pick_pos);
	void execute(const execute_info& info) override;

private:
	vi32 m_pick_pos;
};
}
}
}