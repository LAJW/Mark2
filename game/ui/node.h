#pragma once
#include <add_const_if.h>
#include <stdafx.h>
#include <ui/action/legacy.h>
#include <ui/event.h>

namespace mark {
namespace ui {
struct event;
class window;
class node;

class node_ref
{
private:
	template <typename T, typename U = add_const_if_t<node, std::is_const_v<T>>>
	[[nodiscard]] static optional<U&> next_impl(T& self)
	{
		if (self.m_next) {
			return *self.m_next;
		}
		return {};
	}

public:
	[[nodiscard]] optional<node&> next() { return next_impl(*this); }
	[[nodiscard]] optional<const node&> next() const
	{
		return next_impl(*this);
	}
	[[nodiscard]] optional<node&> prev() { return m_prev; }
	[[nodiscard]] optional<const node&> prev() const { return m_prev; };

protected:
	friend window;
	node_ref() = default;
	~node_ref() = default;
	[[nodiscard]] optional<const window&> parent() const
	{
		if (m_parent) {
			return *m_parent;
		}
		return {};
	}
	[[nodiscard]] optional<const window&> parent()
	{
		if (m_parent) {
			return *m_parent;
		}
		return {};
	}

private:
	std::unique_ptr<node> m_next;
	optional<node&> m_prev;
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

	[[nodiscard]] virtual handler_result click(const event&) = 0;
	[[nodiscard]] virtual handler_result hover(const event&) = 0;

	// get absolute pos
	[[nodiscard]] virtual auto pos() const noexcept -> vi32 { return m_pos; }
	void pos(vi32 pos) { m_pos = pos; }
	[[nodiscard]] virtual auto size() const -> vi32 { return { 0, 0 }; }
	[[nodiscard]] auto relative() const noexcept -> bool { return m_relative; }
	/// Called by the parent component when resizing.
	/// Override to handle the resize action.
	virtual void resize(vi32 outer_size) { (void)outer_size; }

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
