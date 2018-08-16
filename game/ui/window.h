#pragma once
#include "node.h"
#include <add_const_if.h>
#include <property.h>
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
	[[nodiscard]] std::error_code append(unique_ptr<node>&& node) noexcept; 
	[[nodiscard]] handler_result click(const event&) override;
	[[nodiscard]] handler_result hover(const event&) override;
	void update(update_context&) override;
	[[nodiscard]] std::vector<ref<const node>> children() const;
	[[nodiscard]] std::vector<ref<node>> children();
	[[nodiscard]] std::error_code
	insert(const node& before, unique_ptr<node>&& node);
	[[nodiscard]] unique_ptr<node> remove(const node& which);
	void clear() noexcept;
	[[nodiscard]] optional<node&> front() noexcept;
	[[nodiscard]] optional<const node&> front() const noexcept;
	[[nodiscard]] optional<node&> back() noexcept;
	[[nodiscard]] optional<const node&> back() const noexcept;

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
