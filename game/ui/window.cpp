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
	// Creating a copy of the node list, so that nodes can be deleted in the
	// middle of iteration without invalidating invariants
	std::vector<ref<ui::node>> nodes;
	for (let& node : m_nodes) {
		nodes.push_back(*node);
	}
	return any_of(nodes, [&](let& node) { return node.get().click(event); });
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
			top += node->size().y;
		} else {
			node->update(context);
		}
	}
}

void mark::ui::window::insert(const node& before, unique_ptr<node>&& node)
{
	node->m_parent = this;
	let before_it = find_if(m_nodes.begin(), m_nodes.end(), [&](let& node) {
		return node.get() == &before;
	});
	m_nodes.insert(before_it, move(node));
}


namespace mark {
namespace ui {

unique_ptr<node> window::erase(const node& which)
{
	let node_it = find_if(m_nodes.begin(), m_nodes.end(), [&](let& node) {
		return node.get() == &which;
	});
	auto result = move(*node_it);
	m_nodes.erase(node_it);
	return result;
}

auto window::children() const -> const std::list<unique_ptr<node>>&
{
	return m_nodes;
}

void window::visibility(bool value) noexcept { m_visible = value; }

void window::clear() noexcept { m_nodes.clear(); }

[[nodiscard]] std::vector<ref<node>> window::children_mutable()
{
	std::vector<ref<node>> result;
	for (auto& child : m_nodes) {
		result.push_back(*child);
	}
	return result;
}

} // namespace ui
} // namespace mark
