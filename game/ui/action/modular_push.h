#pragma once
#include "base.h"
#include <interface/item.h>
#include <unit/modular.h>

namespace mark {
namespace ui {
namespace action {

class modular_push : public base
{
private:
	std::unique_ptr<interface::item> m_item;

public:
	explicit modular_push(std::unique_ptr<interface::item> item)
		: m_item(move(item))
	{}
	void execute(const execute_info& info) override
	{
		let error_code = push(*info.modular, move(m_item));
		Ensures(success(error_code) || error_code == error::code::stacked);
	}
};

} // namespace action
} // namespace ui
} // namespace mark
