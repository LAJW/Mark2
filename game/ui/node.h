#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
struct event;
class node
{
public:
	struct info
	{
		bool relative = false;
	};

	virtual void update(update_context& ctx) = 0;
	virtual bool click(const event&) = 0;
	virtual bool hover(const event&) = 0;
	// get absolute pos
	virtual auto pos() const noexcept -> vector<int> { return m_pos; }
	void pos(const vector<int>& pos) { m_pos = pos; }
	virtual auto size() const -> vector<int> { return { 0, 0 }; }
	auto relative() const noexcept -> bool { return m_relative; }

protected:
	node(const info& info)
		: m_relative(info.relative)
	{}
	vector<int> m_pos;

private:
	bool m_relative = false;
};
} // namespace ui
} // namespace mark
