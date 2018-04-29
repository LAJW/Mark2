#pragma once
#include <stdafx.h>

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

class callback_group
{
public:
	void insert(std::function<bool(const event&)> callback);
	// Returns true if event was handled
	bool dispatch(const event&) const;
	bool operator()(const event&) const;

private:
	std::vector<std::function<bool(const event&)>> m_callbacks;
};
} // namespace ui
} // namespace mark