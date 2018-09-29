#include "event.h"
#include <stdafx.h>

namespace mark {
namespace ui {

void mark::ui::callback_group::insert(
	std::function<handler_result(const event&)> callback)
{
	m_callbacks.push_back(move(callback));
}

handler_result mark::ui::callback_group::dispatch(const event& event) const
{
	for (let& callback : m_callbacks) {
		if (auto actions = callback(event)) {
			return actions;
		}
	}
	return { };
}

handler_result mark::ui::callback_group::operator()(const event& event) const
{
	return this->dispatch(event);
}

} // namespace ui
} // namespace mark
