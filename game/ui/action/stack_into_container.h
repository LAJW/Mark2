#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

class stack_into_container final : public base
{
public:
	stack_into_container(const interface::container& container, vi32 pos)
		: m_container(container)
		, m_pos(pos)
	{}
	void execute(const execute_info& info) override;

private:
	const interface::container& m_container;
	const vi32 m_pos;
};

} // namespace action
} // namespace ui
} // namespace mark
