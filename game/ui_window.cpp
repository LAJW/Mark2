#include "stdafx.h"
#include "ui_window.h"

void mark::ui::window::insert(std::unique_ptr<node> node)
{
	m_nodes.push_back(std::move(node));
}

void mark::ui::window::remove(std::unique_ptr<node> node)
{

}

bool mark::ui::window::click(const event& event)
{
	for (const auto& node : m_nodes) {
		const auto handled = node->click(event);
		if (handled) {
			return true;
		}
	}
	return false;
}

void mark::ui::window::tick(mark::tick_context& context)
{
	for (const auto& node : m_nodes) {
		node->tick(context);
	}
}

auto mark::ui::window::pos() const noexcept -> vector<int>
{
	if (m_parent) {
		return m_parent->pos() + m_pos;
	} else {
		return m_pos;
	}
}
