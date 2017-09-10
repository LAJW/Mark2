#include "stdafx.h"
#include "ui_event.h"

void mark::ui::callback_group::insert(std::function<bool(const event&)> callback)
{
	m_callbacks.push_back(callback);
}

bool mark::ui::callback_group::dispatch(const event& event) const
{
	for (const auto& callback : m_callbacks) {
		if (const auto stop_propagation = callback(event)) {
			return true;
		}
	}
	return false;
}

bool mark::ui::callback_group::operator()(const event& event) const
{ this->dispatch(event); }
