#pragma once
#include "base.h"
namespace mark {
namespace ui {
namespace action {

class recycle : public base
{
public:
	recycle(interface::container& container, vi32 pos)
		: m_container(container)
		, m_pos(pos)
	{}
	void execute(const execute_info& info) override;

private:
	interface::container& m_container;
	const vi32 m_pos;
};

} // namespace action
} // namespace ui
} // namespace mark
