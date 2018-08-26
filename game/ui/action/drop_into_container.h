#pragma once
#include "base.h"

namespace mark {
namespace ui {
namespace action {

class drop_into_container final : public base
{
public:
	drop_into_container(const interface::container& container, vi32 pos)
		: m_container(container)
		, m_pos(pos)
	{}
	void execute(const execute_info& info) override;
	[[nodiscard]] const interface::container& container() const noexcept
	{
		return m_container;
	}
	[[nodiscard]] const vi32 pos() const noexcept { return m_pos; }

private:
	const interface::container& m_container;
	const vi32 m_pos;
};

} // namespace action
} // namespace ui
} // namespace mark
