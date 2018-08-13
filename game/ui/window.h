#pragma once
#include "node.h"
#include <stdafx.h>
#include <add_const_if.h>
#include <property.h>

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
	void remove(const node& node);
	bool click(const event&) override;
	bool hover(const event&) override;
	void update(update_context&) override;
	[[nodiscard]] std::vector<ref<const node>> children() const;
	[[nodiscard]] std::vector<ref<node>> children();
	void insert(const node& before, unique_ptr<node>&& node);
	[[nodiscard]] unique_ptr<node> erase(const node& which);
	void clear() noexcept;
	Property<bool> visibility = true;

private:
	template <typename T, typename U = add_const_if_t<node, std::is_const_v<T>>>
	[[nodiscard]] static std::vector<ref<U>> children_impl(T& self);
	unique_ptr<node> m_first_child;
	optional<node&> m_last_child;
	window* m_parent = nullptr;
};
} // namespace ui
} // namespace mark
