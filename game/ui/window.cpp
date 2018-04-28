#include "window.h"
#include <algorithm.h>
#include <stdafx.h>

mark::ui::window::window(const info& info)
	: node(info)
{
	pos(info.pos);
}

void mark::ui::window::insert(unique_ptr<node> node)
{
	m_nodes.push_back(move(node));
}

void mark::ui::window::remove(node& to_remove)
{
	let it = find_if(m_nodes.begin(), m_nodes.end(), [&](let& node) {
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

void mark::ui::window::update(update_context& context)
{
	if (!m_visible) {
		return;
	}
	int top = 0;
	for (let& node : m_nodes) {
		if (node->relative()) {
			node->pos({ 0, top });
			node->update(context);
			top += node->size().y + 10;
		} else {
			node->update(context);
		}
	}
}

auto mark::ui::window::children() -> std::list<unique_ptr<node>>&
{
	return m_nodes;
}

auto mark::ui::window::children() const
	-> const std::list<unique_ptr<node>>&
{
	return m_nodes;
}

void mark::ui::window::visibility(bool value) noexcept { m_visible = value; }
