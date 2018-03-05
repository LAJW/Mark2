#include <stdafx.h>
#include "event.h"

void mark::ui::callback_group::insert(std::function<bool(const event&)> callback)
{
	m_callbacks.push_back(callback);
}

bool mark::ui::callback_group::dispatch(const event& event) const
{
	for (let& callback : m_callbacks) {
		if (let stop_propagation = callback(event)) {
			return true;
		}
	}
	return false;
}

bool mark::ui::callback_group::operator()(const event& event) const
{ return this->dispatch(event); }
