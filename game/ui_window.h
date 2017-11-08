#pragma once
#include "stdafx.h"
#include "ui_node.h"

namespace mark {
namespace ui {
struct event;
class window : public node {
public:
	window(const vector<int>& pos = { 0, 0 });
	void insert(std::unique_ptr<node> node);
	void remove(node& node);
	auto pos() const noexcept -> vector<int> override;
	bool click(const event&) override;
	bool hover(const event&) override;
	void tick(tick_context&) override;
	auto children() -> std::list<std::unique_ptr<node>>&;
private:
	std::list<std::unique_ptr<node>> m_nodes;
	window* m_parent = nullptr;
};

} }