#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

class grab_from_container final : public base
{
public:
	grab_from_container(const interface::container& container, vi32 pos)
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
