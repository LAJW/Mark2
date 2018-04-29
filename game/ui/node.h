#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
struct event;
class window;

class node_ref
{
protected:
	friend window;
	node_ref() = default;
	~node_ref() = default;
	const window& parent() const
	{
		Expects(m_parent);
		return *m_parent;
	}
	window& parent()
	{
		Expects(m_parent);
		return *m_parent;
	}
private:
	window* m_parent = nullptr;
};

class node : public node_ref
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
	virtual auto pos() const noexcept -> vi32 { return m_pos; }
	void pos(vi32 pos) { m_pos = pos; }
	virtual auto size() const -> vi32 { return { 0, 0 }; }
	auto relative() const noexcept -> bool { return m_relative; }

protected:
	node(const info& info)
		: m_relative(info.relative)
	{}

	vi32 m_pos;

private:
	bool m_relative = false;
};
} // namespace ui
} // namespace mark
