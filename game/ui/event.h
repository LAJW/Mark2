﻿#pragma once
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

struct handler_result final
{
	bool handled;
	std::vector<unique_ptr<action::base>> actions;

	/// Create a handled result with a single action
	[[nodiscard]] static handler_result
	make(std::unique_ptr<action::base> action)
	{
		handler_result result;
		result.handled = true;
		result.actions.push_back(move(action));
		return result;
	};
};

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
