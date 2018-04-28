#pragma once
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
	void insert(std::unique_ptr<node> node);
	void remove(node& node);
	bool click(const event&) override;
	bool hover(const event&) override;
	void update(update_context&) override;
	auto children() -> std::list<std::unique_ptr<node>>&;
	auto children() const -> const std::list<std::unique_ptr<node>>&;
	void visibility(bool) noexcept;

private:
	std::list<std::unique_ptr<node>> m_nodes;
	window* m_parent = nullptr;
	bool m_visible = true;
};
} // namespace ui
} // namespace mark
