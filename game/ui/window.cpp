#include "window.h"
#include <algorithm.h>
#include <stdafx.h>

namespace mark {
namespace ui {

mark::ui::window::window(const info& info)
	: node(info)
{
	pos(info.pos);
}

void mark::ui::window::insert(unique_ptr<node> node)
{
	node->m_parent = this;
	if (m_first_child) {
		node->m_prev = m_last_child;
		auto& node_ref = *node;
		m_last_child->m_next = move(node);
		m_last_child = node_ref;
	} else {
		node->m_parent = this;
		m_first_child = move(node);
		m_last_child = *m_first_child;
	}
}

// Deprecated, combine erase and remove
void mark::ui::window::remove(const node& to_remove) { (void)erase(to_remove); }

// TODO: Implement as a non-member function
bool mark::ui::window::click(const event& event)
{
	if (!m_visible) {
		return false;
	}
	// Iterating over a copy to allow deletion in the middle
	// Using underlying list would be faster but unsafe
	return any_of(this->children(), [&](let& node) {
		return node.get().click(event);
	});
}

bool mark::ui::window::hover(const event& event)
{
	if (!m_visible) {
		return false;
	}
	return any_of(this->children(), [&](let& node) {
		return node.get().hover(event);
	});
}

void mark::ui::window::update(update_context& context)
{
	if (!m_visible) {
		return;
	}
	int top = 0;
	for (let child : this->children()) {
		auto& node = child.get();
		if (node.relative()) {
			node.pos({ 0, top });
			node.update(context);
			top += node.size().y;
		} else {
			node.update(context);
		}
	}
}

void mark::ui::window::insert(const node& before, unique_ptr<node>&& node)
{
	if (before.m_parent != this) {
		return; // TODO: Return an error here
	}
	if (&before == &*m_last_child) {
		// Calling "push_back" so that the last_child pointer is updated
		this->insert(move(node));
	} else {
		node->m_parent = this;
		// Casting is OK, because the node can be accessed using `this` pointer,
		// so it's mutable anyway. This is just faster
		auto& before_mutable = const_cast<ui::node&>(before);
		node->m_next = move(before_mutable.m_next);
		node->m_next->m_prev = *node;
		before_mutable.m_next = move(node);
	}
}

unique_ptr<node> window::erase(const node& const_which)
{
	// Casting away const-ness, because the node is accessible from this context
	auto& which = const_cast<node&>(const_which);
	if (&which == &*m_last_child) {
		m_last_child = which.m_prev;
		which.m_prev.reset();
		return move(m_last_child->m_next);
	} else {
		auto result = move(which.m_prev->m_next);
		which.m_next->m_prev = which.m_prev;
		which.m_prev->m_next = move(which.m_next);
		which.m_prev.reset();
		return result;
	}
}

template <typename T, typename U>
std::vector<ref<U>> window::children_impl(T& self)
{
	std::vector<ref<U>> children;
	for (auto cur = optional<U&>(*self.m_first_child); cur.has_value();
		 cur = *cur->m_next) {
		children.push_back(*cur);
	}
	return children;
}

auto window::children() const -> std::vector<ref<const node>>
{
	return children_impl(*this);
}

[[nodiscard]] std::vector<ref<node>> window::children()
{
	return children_impl(*this);
}

void window::visibility(bool value) noexcept { m_visible = value; }

void window::clear() noexcept
{
	m_first_child.reset();
	m_last_child.reset();
}

} // namespace ui
} // namespace mark
