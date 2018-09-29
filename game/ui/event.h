#pragma once
#include <stdafx.h>
#include <ui/action/base.h>

namespace mark {
namespace ui {

struct event final
{
	// Mouse cursor position relative to the screen
	vi32 absolute_cursor;
	// Mouse cursor relative to the element top left corner
	vi32 cursor;
	// Mouse cursor relative to the world
	vd world_cursor;
	// Was shift key pressed
	bool shift;
};

using handler_result = optional<std::vector<std::unique_ptr<action::base>>>;
template <typename T, typename... Ts>
[[nodiscard]] handler_result make_handler_result(Ts&&... action_args)
{
	std::vector<std::unique_ptr<action::base>> actions;
	actions.push_back(std::make_unique<T>(std::forward<Ts>(action_args)...));
	return handler_result(move(actions));
}

/// Create a "handled", but action-less handler result
inline [[nodiscard]] handler_result handled()
{
	return handler_result(std::vector<std::unique_ptr<action::base>>());
}

class callback_group final
{
public:
	void insert(std::function<handler_result(const event&)> callback);
	[[nodiscard]] handler_result dispatch(const event&) const;
	[[nodiscard]] handler_result operator()(const event&) const;

private:
	std::vector<std::function<handler_result(const event&)>> m_callbacks;
};

} // namespace ui
} // namespace mark
