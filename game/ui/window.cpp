#include <stdafx.h>
#include "window.h"
#include <algorithm.h>

mark::ui::window::window(const vector<int>& pos)
{
	m_pos = pos;
}

void mark::ui::window::insert(std::unique_ptr<node> node)
{
	m_nodes.push_back(move(node));
}

void mark::ui::window::remove(node& to_remove)
{
	let it = find_if(
		m_nodes.begin(),
		m_nodes.end(),
		[&](let& node) {
		return node.get() == &to_remove;
	});
	drop(m_nodes, it);
}

bool mark::ui::window::click(const event& event)
{
	if (!m_visible) {
		return false;
	}
	for (let& node : m_nodes) {
		let handled = node->click(event);
		if (handled) {
			return true;
		}
	}
	return false;
}

bool mark::ui::window::hover(const event& event)
{
	if (!m_visible) {
		return false;
	}
	for (let& node : m_nodes) {
		let handled = node->hover(event);
		if (handled) {
			return true;
		}
	}
	return false;
}

void mark::ui::window::tick(tick_context& context)
{
	if (!m_visible) {
		return;
	}
	int top = 0;
	for (let& node : m_nodes) {
		if (node->relative()) {
			node->pos({ 0, top });
			node->tick(context);
			top += node->size().y + 10;
		} else {
			node->tick(context);
		}
	}
}

auto mark::ui::window::children() -> std::list<std::unique_ptr<node>>&
{ return m_nodes; }

void mark::ui::window::visibility(bool value) noexcept
{ m_visible = value; }
