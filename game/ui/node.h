#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
struct event;
class node
{
public:
	virtual ~node() = default;
	virtual void tick(tick_context& ctx) = 0;
	virtual bool click(const event&) = 0;
	virtual bool hover(const event&) = 0;
	// get absolute pos
	virtual auto pos() const noexcept -> vector<int> { return m_pos; }
	void pos(const vector<int>& pos) { m_pos = pos; }
	virtual vector<int> size() const { return { 0, 0 }; }
	bool m_relative = false;
	auto relative() const noexcept -> bool { return m_relative; }

protected:
	vector<int> m_pos;
};
} // namespace ui
} // namespace mark