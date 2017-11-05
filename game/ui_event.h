#pragma once
#include "stdafx.h"

namespace mark {
namespace ui {

struct event final {
	// Mouse cursor position relative to the screen
	vector<int> absolute_cursor;
	// Mouse cursor relative to the element top left corner
	vector<int> cursor;
	// Mouse cursor relative to the world
	vector<double> world_cursor;
};

class callback_group {
public:
	void insert(std::function<bool(const event&)> callback);
	// Returns true if event was handled
	bool dispatch(const event&) const;
	bool operator()(const event&) const;
private:
	std::vector<std::function<bool(const event&)>> m_callbacks;
};

} }