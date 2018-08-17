#include "window.h"
#include <algorithm.h>
#include <stdafx.h>
#include <exception.h>

namespace mark {
namespace ui {

mark::ui::window::window(const info& info)
	: node(info)
{
	pos(info.pos);
}

std::error_code mark::ui::window::append(std::unique_ptr<node>&& node) noexcept
{
	Expects(node);
	if (node.get() == this) {
		return error::code::ui_cycle;
	}
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
	return error::code::success;
}

[[nodiscard]] static bool
propagate(window& window, std::function<bool(node&)> propagator)
{
	if (!window.visibility()) {
		return false;
	}
	// Iterating over a copy to allow deletion in the middle
	// Using underlying list would be faster but unsafe
	return any_of(
		window.children(), [&](let& node) { return propagator(node.get()); });
}

bool mark::ui::window::click(const event& event)
{
	return propagate(*this, [&](auto& node) { return node.click(event); });
}

bool mark::ui::window::hover(const event& event)
{
	return propagate(*this, [&](auto& node) { return node.hover(event); });
}

static void update(window& window, update_context& context)
{
	if (!window.visibility()) {
		return;
	}
	int top = 0;
	for (let child : window.children()) {
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

void mark::ui::window::update(update_context& context)
{
	ui::update(*this, context);
}

std::error_code mark::ui::window::insert(const node& before, unique_ptr<node>&& node)
{
	Expects(node);
	if (before.m_parent != this) {
		return error::code::ui_bad_before;
	}
	if (node.get() == this) {
		return error::code::ui_cycle;
	}
	if (&before == m_first_child.get()) {
		m_first_child->m_prev = *node;
		node->m_next = move(m_first_child);
		m_first_child = move(node);
	} else {
		node->m_parent = this;
		// Casting is OK, because the node can be accessed using `this` pointer,
		// so it's mutable anyway. This is just faster
		auto& before_mutable = const_cast<ui::node&>(before);
		node->m_next = move(before_mutable.m_prev->m_next);
		node->m_prev = before_mutable.m_prev;
		auto& node_ref = *node;
		before_mutable.m_prev->m_next = move(node);
		before_mutable.m_prev = node_ref;
	}
	return error::code::success;
}

unique_ptr<node> window::remove(const node& const_which)
{
	// Casting away const-ness for speed, because mutable reference is
	// accessible from this context
	auto& which = const_cast<node&>(const_which);
	if (which.m_parent != this) {
		return nullptr;
	}
	if (&which == m_first_child.get()) {
		auto result = move(m_first_child);
		m_first_child = move(result->m_next);
		if (m_first_child) {
			m_first_child->m_prev.reset();
		} else {
			m_last_child.reset();
		}
		return result;
	} else if (&which == &*m_last_child) {
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
	for (auto cur = self.front(); cur; cur = *cur->m_next) {
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

void window::clear() noexcept
{
	m_first_child.reset();
	m_last_child.reset();
}

optional<node&> window::front() noexcept { return *m_first_child; }

optional<const node&> window::front() const noexcept { return *m_first_child; }

optional<node&> window::back() noexcept { return m_last_child; }

optional<const node&> window::back() const noexcept { return m_last_child; }

} // namespace ui
} // namespace mark
