#include "stdafx.h"
#include "ui_window.h"
#include "algorithm.h"

mark::ui::window::window(mark::vector<int> pos)
	: m_pos(pos) { }

void mark::ui::window::insert(std::unique_ptr<node> node)
{
	m_nodes.push_back(std::move(node));
}

void mark::ui::window::remove(node& to_remove)
{
	const auto it = std::find_if(
		m_nodes.begin(),
		m_nodes.end(),
		[&](const auto& node) {
		return node.get() == &to_remove;
	});
	mark::drop(m_nodes, it);
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

bool mark::ui::window::hover(const event& event)
{
	for (const auto& node : m_nodes) {
		const auto handled = node->hover(event);
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
