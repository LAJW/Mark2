#pragma once
#include "stdafx.h"
#include "ui_node.h"

namespace mark {
namespace ui {

struct event;
class window final : public node {
public:
	window()=default;
	void insert(std::unique_ptr<node> node);
	void remove(node& node);
	auto pos() const noexcept -> vector<int> override;
	bool click(const event&) override;
	bool hover(const event&) override;
	void tick(mark::tick_context&) override;
private:
	std::vector<std::unique_ptr<node>> m_nodes;
	window* m_parent = nullptr;
	mark::vector<int> m_pos;
};

} }