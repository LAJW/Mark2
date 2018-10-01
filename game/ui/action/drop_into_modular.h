#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

/// Attach the grabbed module to the modular
class drop_into_modular final : public base
{
public:
	drop_into_modular(vi32 drop_pos, std::vector<int8_t> bindings);
	void execute(const execute_info& info) override;

private:
	vi32 m_drop_pos;
	std::vector<int8_t> m_bindings;
};

}
}
}