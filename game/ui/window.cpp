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
	node->m_parent = this;
	m_nodes.push_back(move(node));
}

void mark::ui::window::remove(node& to_remove)
{
	let it = find_if(m_nodes.begin(), m_nodes.end(), [&](let& node) {
		return node.get() == &to_remove;
	});
	drop(m_nodes, it)->m_parent = nullptr;
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

void mark::ui::window::insert(
	std::list<unique_ptr<node>>::const_iterator before,
	unique_ptr<node> node)
{
	node->m_parent = this;
	m_nodes.insert(before, move(node));
}

void mark::ui::window::erase(
	std::list<std::unique_ptr<mark::ui::node>>::const_iterator which)
{
	m_nodes.erase(which);
}

auto mark::ui::window::children() const -> const std::list<unique_ptr<node>>&
{
	return m_nodes;
}

void mark::ui::window::visibility(bool value) noexcept { m_visible = value; }

void mark::ui::window::clear() noexcept { m_nodes.clear(); }
