#pragma once
#include "base.h"
#include <ui/tooltip.h>
#include <variant>

namespace mark {
namespace ui {
namespace action {
class set_tooltip final : public base
{
private:
	std::variant<vd, vi32> m_pos;
	const void* m_id;
	std::string m_text;

public:
	set_tooltip(std::variant<vd, vi32> pos, const void* id, const std::string& text)
		: m_pos(pos)
		, m_text(text)
		, m_id(id)
	{}
	void execute(const execute_info& info) override
	{
		info.tooltip->set(m_pos, m_id, m_text);
	}
};
} // namespace action
} // namespace ui
} // namespace mark
