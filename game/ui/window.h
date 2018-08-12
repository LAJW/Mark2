﻿#pragma once
#include "node.h"
#include <stdafx.h>

namespace mark {
namespace ui {
struct event;
class window : public node
{
public:
	struct info : node::info
	{
		vi32 pos;
	};
	window(const info& info);
	void insert(unique_ptr<node> node);
	void remove(node& node);
	bool click(const event&) override;
	bool hover(const event&) override;
	void update(update_context&) override;
	auto children() const -> const std::list<unique_ptr<node>>&;
	[[nodiscard]] std::vector<ref<node>> children_mutable();
	void insert(
		std::list<std::unique_ptr<node>>::const_iterator before,
		unique_ptr<node> node);
	void erase(std::list<std::unique_ptr<node>>::const_iterator which);
	void clear() noexcept;
	void visibility(bool) noexcept;

private:
	std::list<unique_ptr<node>> m_nodes;
	window* m_parent = nullptr;
	bool m_visible = true;
};
} // namespace ui
} // namespace mark
